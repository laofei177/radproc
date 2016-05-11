#include "nonThermalLuminosity.h"

#include "targetFields.h"
#include "modelParameters.h"
#include "cilindricIntegral.h"

#include <flosses\nonThermalLosses.h>
//#include <fmath\RungeKutta.h>
#include <fparameters\parameters.h>

#include <fmath\physics.h>



double dLnt(double z)  //esta es la funci�n que depende del n�mero de estrellas a tiempo t
{
	
	//double Tstar = 3.0e3; 
	double E = P2(electronMass*cLight2) / (boltzmann*starT);

	double Sj = pi*P2(jetRadius(z, openingAngle));
	double stagPoint = stagnationPoint(z);
	double So = 100.0*pi*P2(stagPoint);

	double vel_lat = cLight*openingAngle;

	double tad = adiabaticLosses(E, z, vel_lat) / E; //ad esta en erg/s

	double wph = Lj / (cLight*4.0*Sj);
	double trad = 4.0*cLight*thomson*wph*E / P2(electronMass*cLight2);

	double frad = 1.0 / (1.0 + tad / trad);

	double Lnt = accEfficiency*Lj*(So / Sj)*frad*pow(Dlorentz, 4) / P2(Gamma);

	double f = starDensity(z)*Lnt; //saque el Sj

	return f;   

}

double nonThermalLuminosity(double intRmin, double intRmax)
{
	double integral = intCilindric(intRmin, intRmax, dLnt); // (double z){ return dLnt(z, dummie) });
	return integral;
	//return 1.0e38;
}