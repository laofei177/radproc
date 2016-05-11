#include "modelParameters.h"

#include "State.h"

#include <fmath\interpolation.h>
#include <fparameters\parameters.h>
#include <fmath\physics.h>

//#include <string>
#include <iostream>
//#include <fstream>

double fmagneticField(double z, double B_o)
{	
	return B_o/z;
}

double jetRadius(double z, double openingAngle)
{
	return z*openingAngle;
}

double eEmax(double z, double B)
{
	double Reff = 10.0*stagnationPoint(z);
	double vel_lat = cLight*openingAngle;

	double Emax_ad = accEfficiency*3.0*jetRadius(z,openingAngle)*cLight*electronCharge*B / (vel_lat*Gamma);
	double Emax_syn = electronMass*cLight2*sqrt(accEfficiency*6.0*pi*electronCharge / (thomson*B));
	double Emax_hillas = electronCharge*B*Reff;
	double min1 = std::min(Emax_syn, Emax_syn);

	//std::ofstream file;
	//file.open("Emax.txt", std::ios::out);

	//std::cout << z << '\t' << Emax_ad << '\t' << Emax_syn << '\t' << Emax_hillas << '\t' << std::endl;

	return std::min(min1, Emax_hillas);
		
}

double stagnationPoint(double z)
{
	double Mdot_wind = 1.0e-8*solarMass / yr;
	double v_wind = 2.0e7;

	double stagPoint = sqrt(Mdot_wind*v_wind*cLight / (4.0*Lj))*jetRadius(z,openingAngle);

	return stagPoint;

}


void derive_parameters_r(double E, double z, double t)
{
	radius = jetRadius(z, openingAngle);
	magneticField = fmagneticField(z, B0); 
	//electronLogEmax = log10(eEmax(z, magneticField));

	//Rsp = stagnationPoint(z);
}


void setParameters(void )
{
	
	double mBH = 1.0e7*solarMass;  //black hole mass
	double rg = mBH*gravitationalConstant / cLight2;

	double z0 = 50.0*rg; //50 * Rschw  //OJO! si cambian, cambiar tmb nonthermalLuminosity!!

	starT = 3.0e3;

	Lj = 1.0e43;
	openingAngle = 0.1;  //jet opening angle

	B0 = sqrt(8.0*Lj / cLight) / openingAngle;  //ojo que esto es Bo*z0

//	Rsp = 1.0e14; //distance to stagnation point

	double inc = 10.0*pi / 180; //ang obs del jet
	Gamma = 10;
	double beta = 1.0 - 1.0 / P2(Gamma);
	Dlorentz = 1.0 / (Gamma*(1.0 - cos(inc)*beta));

	accEfficiency = 0.1; 

	//magneticField = fmagneticField(z0,B0);  //el primero lo calculo en r = z0
	//density = nWindDensity(Rc, starR); //el primero lo calculo en r = Rc

// Data of electrons and protons
	primaryIndex  = 2.0;
	//factor_qrel   = 3.0; 

	electronLogEmin = 6.0;
	electronLogEmax = 15.0;  


//Data of photons

	photonLogEmin = -6.0;
	photonLogEmax = 12.0;

	targetPhotonEmin = pow(10.0,photonLogEmin)*1.6e-12;  //0.15e3*1.6e-12;  //photonEmin = 0.15 KeV 
	targetPhotonEmax = pow(10.0,photonLogEmax)*1.6e-12;  //150.0e3*1.6e-12;   //cutEnergy  = 150 KeV

	rmin = 1.0*pc;
	rmax = 1.0e3*pc;
	nR = 10;

	//los parametros de t los comento porque el vector t(i) lo construyo como los crossing times de las celdas xi
//	timeMin = 1.0e-2; 
//	timeMax = (rmax / cLight); // 1.0e11; // rmax / cLight;
//	nTimes = 50;

	nEnergies = 20;        //massive particles
	nPhotonEnergies = 20;  //
}

void initializeRPoints(Vector& v, double Rmin, double Rmax)
{

	double R_int = pow((Rmax / Rmin), (1.0 / (v.size() - 1)));

	v[0] = Rmin;

	for (size_t i = 1; i < v.size(); ++i){
		v[i] = v[i - 1] * R_int;
	}

}

void initializeCrossingTimePoints(Vector& time, double rMin, double rMax)
{
	double R_int = pow((rMax / rMin), (1.0 / nR));

	Vector v(time.size()+1, 0.0);

	v[0] = rMin;

	for (size_t i = 0; i < time.size(); ++i){
		v[i+1] = v[i] * R_int;
		double delta_x = v[i+1]-v[i];
		time[i] = delta_x / cLight;  //construyo los t_i como los crossing time de las celdas i
	}

}




/*

double vWind(double r, double starR) //x es la energia
{
	double vInfty = 2.0e8;
	return vInfty*(1.0 - starR / r);
}

double nWindDensity(double r, double starR)
{
	double Mdot = 3.0e-6*solarMass;
	return Mdot / (4.0*pi*P2(r)*vWind(r, starR)*protonMass);
}*/