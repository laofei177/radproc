#include "lossesAnisotropicIC.h"

//#include "crossSectionInel.h"
//#include "dataLosses.h"

#include "targetFields.h"
#include <fmath\physics.h>
#include <fparameters/parameters.h>

#include <boost/property_tree/ptree.hpp>

double b_theta(double theta, double w0, double E)
{
	static const double Gamma = GlobalConfig.get<double>("Gamma", 10);
	static const double Dlorentz = GlobalConfig.get<double>("Dlorentz");

	//corregido es 1.0 - cos(theta') escrito en funcion de theta sin primar
	double Dstar = 1.0 / (Gamma - 1.0);
	double corregido = (1.0 - cos(theta))*Dlorentz*Dstar;
	double b = 2.0 * (corregido)*w0*E / P2(electronMass*cLight2);
	return b;
}


double cAniIC(double w0, double theta, double E)   //limite inferior
{
	return w0;
}

double dAniIC(double w0, double theta, double E)   //limite superior     
{

	double s = b_theta(theta, w0, E);  //reemplazo el Gamma = 4*u*E / P2(mass*cLight2), para incuir la dep theta
	return s*E / (1 + s);
}



double difN(double theta, double w, double w0, double E, double r)   //funcion a integrar
{
	// eps -> variable de afuera
	// eps_0 -> variable integracion

	double b = b_theta(theta, w0, E); 
	double z = w / E;

	//defino F(z)
	double F = 1.0 + P2(z) / (2.0*(1.0 - z)) - 2.0*z / (b*(1.0 - z)) + 2.0*P2(z) / P2(b*(1.0 - z));

	double nph = starBlackBody(w,r);
	double invariant = nph / w;

	double result = (3.0*thomson / (16.0*pi)) * P2(electronMass*cLight2 / E) * invariant * F;

	return result;


}

double fLosses(double x, double t, double y, double E, double r)
{
	return (y - x)*difN(t, x, y, E, r)*(0.5*sin(t));
}


double lossesAnisotropicIC(double E, Particle& particle, double r)
{
	static const double starT = GlobalConfig.get<double>("starT",3.0e3);

	double mass = particle.mass;

	double a = 1.0e-3*boltzmann*starT;      //energia minima de los fotones en erg
	double b = 1.0e3*boltzmann*starT;    //energia maxima de los fotones en erg


	double integral = 
		intTriple(
			E, a, b, r, 
			[E](double w0, double theta){
				return cAniIC(w0, theta, E);
			},
			[E](double w0, double theta){
				return dAniIC(w0, theta, E);
			},
			[E, r](double x, double t, double y){
				return fLosses(x, t, y, E, r);
			}
		);


	return integral*cLight; //en erg/s

}




//esta integral no es un runge Kutta; directamente hice la suma de todas las contribuciones

IntLossesOpt DefOpt_IntLosses{ 20, 20, 20 };
double intTriple(double E, double eps_min, double eps_max, double r, fun2 c, fun2 d, fun3 f, const IntLossesOpt& opt)
{
	const int Xs{ opt.samples_x };
	const int Ts{ opt.samples_t };
	const int Ys{ opt.samples_y };
	const double x_int = pow((eps_max / eps_min), (1.0 / Xs));
	const double t_min = pi / 2; //   //t_min=0.0
	const double t_max = pi;
	const double t_int = pow((t_max / t_min), (1.0 / Ts));

//	double X1(0.0), X2(0.0), X3(0.0);
	double L3 = 0.0;

	if (eps_min < eps_max)
	{
		// [hook]
		// { the only sincronization point is the accumulation in L3 }
		// { we use the reduction directive to specify this: }
		#pragma omp parallel for reduction(+:L3)
		for (int i_x = 0; i_x < Xs; ++i_x)     //le saco el n para que se multiplique n veces y no n+1
		{
			const double x{ eps_min*pow(x_int, i_x) };
			const double dx{ x*(x_int - 1) };

			//	if (t_min < t_max)
			//	{

//			double T1(0.0), T2(0.0), T3(0.0);
			double L2{ 0.0 };

			double t{ t_min };

			for (int i_t = 0; i_t < Ts; ++i_t)
			{
				const double dt{ t*(t_int - 1) };

				const double inf{ c(x, t) }; //limite inferior de eps_1
				const double sup{ d(x, t) }; //limite superior (double u, double E, double theta) 

				if (inf < sup)
				{

					const double y_int{ pow((sup / inf), (1.0 / Ys)) };

//					double Y1(0.0), Y2(0.0), Y3(0.0);

					double L1{ 0.0 };

					double y{ inf };

					for (int i_y = 0; i_y < Ys; ++i_y)
					{
						const double dy{ y*(y_int - 1) };

						////////////// calculo la funcion a integrar

						// eps -> variable de afuera = x
						// eps_0 -> variable integracion =y

						const double Q{ f(x, t, y) };// f(x, t, y, E, r);
						//double Q = 1.0;

						////////////////////////////

						L1 += Q*dy;

						//if (L1 > 0.0) { 
						//incremento(i_y, 0, n - 1, Y1, Y2, Y3, L1);// }

						y *= y_int;
					}

					//double L2 = (Y1 + 2.0 * Y2 + 4.0 * Y3) *dt / 3.0;
					L2 += L1*dt;

					//if (L2 > 0.0) { 
					//incremento(i_t, 0, n - 1, T1, T2, T3, L2); //}			
				}


				t *= t_int;

			}

			//			//}
			//
			//double L3 = (T1 + 2.0 * T2 + 4.0 * T3)*dx / 3.0;
			L3 += L2*dx;

			//if (L3 > 0.0) { 
			//incremento(i_x, 0, n - 1, X1, X2, X3, L3); //}

			// [hook] we can't do this in parallel version since it would introduce
			// a dependency between iterations:
			//x = x*x_int;
		}

		//double L4 = (X1 + 2.0 * X2 + 4.0 * X3) / 3.0;
		//return L4;
		return L3;

	}//cierra el primer if
	else
	{
		return 0.0;
	}

}


void incremento(int entero, int left, int right, double &A1, double &A2, double &A3, double R)
{
	double floaT = entero;
	double aux = fmod(floaT, 2.0);

	if (entero == left || entero == right) //si el entero coincide con los extremos
	{
		A1 += R;
	}
	else if (aux == 0)				//si el entero es par
	{
		A2 += R;
	}
	else
	{
		A3 += R;
	}    //si el entero es impar

}