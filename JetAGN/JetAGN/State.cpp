#include "State.h"

#include <fparameters\Dimension.h>
#include <fparameters\SpaceIterator.h>

//ParticleConfig ParticleCfg<Electron>::config{ PT_electron, electronMass, 0, 0, 0 };
//
//ParticleConfig ParticleCfg<Photon>::config{ PT_photon, 0, 0, 0, 0 };

State::State(boost::property_tree::ptree& cfg) :
 nph(photon.ps, false),
 electron{ "electron" },
 photon{ "photon" }
{
	particles.push_back(&electron);
	//particles.push_back(&proton);
	//particles.push_back(&pion);
	//particles.push_back(&muon);
	particles.push_back(&photon);
	//particles.push_back(&secondaryElectron);
	//particles.push_back(&positron);

	for (auto p : particles) {
		initializeParticle(*p,cfg);
	}

	nph.initialize();
	//tpf = nph.dimInterpolator(0);
}


Dimension* createDimension(Particle& p, std::string dimid, std::function<void(Vector&,double,double)> initializer, boost::property_tree::ptree& cfg) {
	int samples = p.getpar<int>(cfg,"dim." + dimid + ".samples");
	int min = p.getpar<int>(cfg, "dim." + dimid + ".min");
	int max = p.getpar<int>(cfg, "dim." + dimid + ".max");
	return new Dimension(samples, bind(initializer, std::placeholders::_1, min, max));
}

void State::initializeParticle(Particle& p, boost::property_tree::ptree& cfg)
{
	using std::bind;

	p.configure(cfg.get_child("particle.default"));
	p.configure(cfg.get_child("particle."+p.id));

	p.ps.add(createDimension(p,"energy",initializeEnergyPoints,cfg));

	// we can't use createDimension because we're multiplying by pc before creating them
	// add dimension for R
	double rmin = p.getpar(cfg,"dim.radius.min", 1.0)*pc;
	double rmax = p.getpar(cfg,"dim.radius.max", 1.0e3)*pc;
	int nR = p.getpar(cfg,"dim.radius.samples", 5); // solo por ahora; y no deberia ser usado directamente desde otro lado
	p.ps.add(new Dimension(nR, bind(initializeRPoints, std::placeholders::_1, rmin, rmax)));

	// add dimension for T
	double tmin = p.getpar(cfg, "dim.time.min", 1.0)*pc;
	double tmax = p.getpar(cfg, "dim.time.max", 1.0e3)*pc;
	int tR = p.getpar(cfg, "dim.time.samples", 5); // solo por ahora; y no deberia ser usado directamente desde otro lado
	p.ps.add(new Dimension(tR, bind(initializeCrossingTimePoints, std::placeholders::_1, tmin, tmax)));

	p.ps.addDerivation([](const SpaceIterator& i){
		derive_parameters_r(i.val(DIM_E), i.val(DIM_R), i.val(DIM_T));
	});

	p.initialize();
}




