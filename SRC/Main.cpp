#include <iostream>
#include <math.h>

#include "Grille.h"
#include "Champ.h"
#include "Liquide.h"
#include "Solveur.h"

using namespace std;

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "../DEP/glew.h"
#include "../DEP/glfw3.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int GRID_SIZE = 128;

int main() {
    // ----------------------
    // Paramètres de la grille / fluide
    int nx = 100, ny = 100;
    double lx = 1.0, ly = 15.0;
    double T=0.,Tmax=10;
    double dt;

    double nu = 1.0, rho = 1.0;

    double U = 0.5, p0 = 1e5;

    double cx = lx/2., radius = 0.4; 

    double eps = 1e-1;
    double omega=1.7;
    // Création du fluide
    Liquide fluide(nx, ny, lx, ly, nu, rho, cx, radius, U, p0);

    while(T<Tmax){
        dt=fluide.CFL();
        cout<<dt<<endl;
        fluide.calc_tot_U_star(dt);
        cout<<"u_star_check"<<endl;
        fluide.SolveurPression(eps,dt,omega,1e3);
        cout<<"solv_press"<<endl;
        fluide.Contrib(dt);
        cout<<"contrib_check"<<endl;
        fluide.condi_lim(U);
        cout<<"condi lim check"<<endl;
        T+=dt;
        cout<<T<<endl;
    }

    return 0;
}
