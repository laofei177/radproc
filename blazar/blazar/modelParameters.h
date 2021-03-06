#pragma once

#include <fmath\mathematics.h>
#include <fparticle\Particle.h>
#include <boost/property_tree/ptree_fwd.hpp>

//
//class Electron : public ParticleCfg<Electron> {};
//class Photon : public ParticleCfg<Photon> {};

const DimensionCoord
DIM_E = 0,
	DIM_R = 1,
	DIM_T = 2;

/* define the inital values of the global parameters*/
void prepareGlobalCfg();



void initializeEnergyPoints(Vector& v, double logEmin, double logEmax);

/* change_parameters changes the values of some parameters for the iteration on variable r*/
//void derive_parameters_r(double E, double R, double T);

double stagnationPoint(double z);

double computeMagField(double z);

double jetRadius(double z, double openingAngle);

double eEmax(double z, double B);


//double vWind(double r, double starR);

