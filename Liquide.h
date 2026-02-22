#ifndef _LIQUIDE_H_
#define _LIQUIDE_H_

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
    Liquide(int nx, int ny, double lx, double ly, double nu, double rho, double cx, double radius)
        : visc(nu), rho_l(rho),
          grid(nx, ny, lx, ly, cx, radius),
          ux(nx, ny), uy(nx, ny),
          p(nx, ny), ux_star(nx, ny), uy_star(nx, ny)
    {}

    // Accès aux champs
    std::vector<double>& Ux() { return ux.GetTab(); }
    double& Ux(int x, int y) { return ux(x, y); }

    std::vector<double>& Uy() { return uy.GetTab(); }
    double& Uy(int x, int y) { return uy(x, y); }

    std::vector<double>& P() { return p.GetTab(); }
    double& P(int x, int y) { return p(x, y); }

    std::vector<double>& Ux_star() { return ux_star.GetTab(); }
    double& Ux_star(int x, int y) { return ux_star(x, y); }

    std::vector<double>& Uy_star() { return uy_star.GetTab(); }
    double& Uy_star(int x, int y) { return uy_star(x, y); }

    // Divergence du champ de vitesse
    double div_u(int x,int y) const;
    double div_u_star(int x,int y) const;

    // Terme convectif upwind
    double convection(Champ u, int x, int y) const;
};

#endif