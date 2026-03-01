#pragma once

#include <vector>
#include "Grille.h"
#include "Champ.h"

/*
 * Classe Liquide
 * Contient :
 *  - ux, uy : vitesses
 *  - p : pression
 *  - ux_star, uy_star : champs intermédiaires
 * Fournit des fonctions :
 *  - div_u, div_u_star : divergence
 *  - convection : terme convectif upwind
 */
class Liquide {
    private:
    double visc;     // viscosité
    double rho_l;    // densité
    Grille grid;

    Champ ux, uy, p;
    Champ ux_star, uy_star;

    public:
    Liquide(int nx, int ny, double lx, double ly, double nu, double rho, double cx, double radius, double U, double p0)
        : visc(nu), rho_l(rho),
          grid(nx, ny, lx, ly, cx, radius),
          ux(nx, ny), uy(nx, ny),
          p(nx, ny), ux_star(nx, ny), uy_star(nx, ny)
    {
        for (int i = 0; i < nx*ny; i++){
            Site s = p.site_xy(i);
            int x=s.x();
            int y=s.y();
            if(x <= 0 || x >= nx-1 || y <= 0 || y >= ny-1){
                ux_star[s] = 0.;
                uy_star[s] = 0.;
                ux[s] = 0.;
                uy[s] = U;
                p[s] = p0;
            } else {
                ux_star[s] = 0.;
                uy_star[s] = 0.;
                ux[s] = 0.;
                uy[s] = U;
                p[s] = p0;
                if(grid.Solide()[i] || grid.Solide()[i+1] || grid.Solide()[i-1] || grid.Solide()[i+nx] || grid.Solide()[i-nx]){
                    uy[s] = 0.;
                } else {
                    uy[s] = U;
                }
            }
        }
    }

    // Accès aux champs
    Champ& Ux() { return ux; }
    double& Ux(int x, int y) { return ux(x, y); }

    Champ& Uy() { return uy; }
    double& Uy(int x, int y) { return uy(x, y); }

    Champ& P() { return p; }
    double& P(int x, int y) { return p(x, y); }

    Champ& Ux_star() { return ux_star; }
    double& Ux_star(int x, int y) { return ux_star(x, y); }

    Champ& Uy_star() { return uy_star; }
    double& Uy_star(int x, int y) { return uy_star(x, y); }

    Grille Grid() const { return grid; }

    // Divergence du champ de vitesse
    double div_u(int x,int y) const;
    double div_u_star(int x,int y) const;

    // Terme convectif upwind
    double convection(const Champ& u, int x, int y) const;

    //Résolution eq Poisson pour la pression
    void SolveurPression(double eps, double dt, double omega, int Maxiter);

    private:

    //Calcul vitesse intermédiaire en un point x,y
    void calc_ux_star(int x, int y, double dt);
    void calc_uy_star(int x, int y, double dt);

    public:

    //Calcul total de la vitesse intermédiaire
    void calc_tot_U_star(double dt);

    //calcul contribution temps t+dt
    void Contrib(double dt);

    private:

    //calcul de la vitesse max dans une cellule pour ux ou uy
    double vmax(char x);

    public:

    //calcul de la condition CFL de pas de temps
    double CFL();

    //condition limite:
    void condi_lim(double U);
};

