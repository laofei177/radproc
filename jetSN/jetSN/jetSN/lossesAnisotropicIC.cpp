#include "lossesAnisotropicIC.h"

#include "targetFields.h"
#include "modelParameters.h"
#include <fmath\physics.h>
#include <fparameters/parameters.h>

#include <boost/property_tree/ptree.hpp>


double lossesAnisotropicIC(double E, Particle& particle, double z,
	double gamma, fun3 tpf, double starT)
{
	static const double R_d = GlobalConfig.get<double>("R_d") *pc;

	double Dlorentz = computeDlorentz(gamma);
	double bG = beta(gamma);

	double mass = particle.mass;
	double Erest = (mass*cLight2);

	double eGamma = E / Erest;
	double eBeta = beta(eGamma); 

	//double cte = (3.0*thomson / (8.0)) * cLight* P2(gamma/z)*(Erest);  //Erest porque es dG/dt

	double theta = boltzmann*starT / Erest;

	double phi_min = 0.0;
	double phi_max = 2.0*pi;
	int nPhi = 20;
	double dphi = (phi_max - phi_min) / nPhi;

	double k_min = -1.0;
	double k_max = 1.0;
	int nK = 20;
	double dk = (k_max - k_min) / nK;

	double r_min = 1.0e15; //generalizar
	double r_max = R_d;
	int nR = 20;
	double r_int = pow((r_max / r_min), (1.0 / nR));
	
	double suma = 0.0;

	double r = r_min;
	// [hook]
	// { the only sincronization point is the accumulation in suma }
	// { we use the reduction directive to specify this: }
	#pragma omp parallel for reduction(+:suma)
		
	for (int i_r = 0; i_r < nR; ++i_r)
	{
		double dr = r*(r_int - 1.0);

		double x = sqrt(P2(r) + P2(z));
		double eta_ph = (z-bG*x)/(x-bG*z);
		double u = 2.7*theta / (gamma*(1.0 + bG*eta_ph));  //<eps>

		double f_r = z*r*pow(theta, 4.0)*P2(x - bG*z) / pow(x, 5.0);

		double phi = phi_min;
		for (int i_phi = 0; i_phi < nPhi; ++i_phi)
		{
			double mu = sqrt(1.0 - P2(eta_ph))*cos(phi);
			double f_phi = P2(1.0 - eBeta*mu);

			double k = k_min;  //este k es en realidad k_prim
			for (int i_k = 0; i_k < nK; ++i_k)
			{
				double a = eBeta*sqrt(1.0 - P2(eta_ph));
				double F = 1.0 + u*eGamma*(1.0 - a*cos(phi))*(1 - k);

				double f1 = 1 - k;
				double f2 = 1.0 + F*(F - 1.0) + F*P2(k);
				double f3 = pow(F, 4.0);

				double integral = f_r*f_phi*f1*(f2 / f3);

				double nph_norm = dr*tpf(u*Erest, z, r); // (u*Erest); 
				// [nph_norm]= cm^-3, en el sist del disco
						
				suma += integral*nph_norm*dk*dphi*dr;

				k = k +dk;
			}  //cierro k

			phi = phi + dphi;
		} //cierro phi

		r = r*r_int;
	} //cierro r

	double cte = pow(pi, 4.0)*thomson*P2(gamma*eGamma)*cLight / 40.0;
	
	return suma*cte*Erest;  //Erest porque paso de dG/dt a dE/dt
}



/*	double eps_min = 1.0e-5*boltzmann*starT / Erest;      //energia minima de los fotones en erg
	double eps_max = 1.0e5*boltzmann*starT / Erest;    //energia maxima de los fotones en erg
	int nU = 50;
	double u_int = pow((eps_max / eps_min), (1.0 / nU));*/