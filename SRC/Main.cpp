#include <iostream>
#include <math.h>

#include "Grille.h"
#include "Champ.h"
#include "Liquide.h"
#include "Solveur.h"
#include "Simulation.h"

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {

	// ----------------------
	// Paramètres de la grille / fluide
	int nx = 1024, ny = 1024;
	double lx = 1.0, ly = 15.0;
	double T = 0., Tmax=10;
	double dt;

	double nu = 1e-6, rho = 1.0;

	double U = 0.5, p0 = 1e5;

	double cx = lx/2., radius = 0.1; 

	double eps = 1e-1;
	double omega=1/(2+sin(M_PI/ny));
	// Création du fluide
	Liquide fluide(nx, ny, lx, ly, nu, rho, cx, radius, U, p0);
	
	// Creation of the simulation
	Simulation sim(nx, ny, "Simulation cylindre", fluide);

//	while(T<Tmax){
//		dt=fluide.CFL();
//		cout<<dt<<endl;
//		fluide.calc_tot_U_star(dt);
//		cout<<"u_star_check"<<endl;
//		fluide.SolveurPression(eps,dt,omega,1e3);
//		cout<<"solv_press"<<endl;
//		fluide.Contrib(dt);
//		cout<<"contrib_check"<<endl;
//		fluide.condi_lim(U);
//		cout<<"condi lim check"<<endl;
//		T+=dt;
//		cout<<T<<endl;
//	}

	sim.run();
	return 0;
}
