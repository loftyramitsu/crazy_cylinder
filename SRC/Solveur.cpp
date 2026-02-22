#include <vector>
#include "Champ.h"
#include "Grille.h"
#include "Solveur.h"
#include<math.h>
#include<iostream>

using namespace std;

namespace Solveur {

    // Laplacien discretisé avec différences centrales
    double Laplacien(const Champ& tab, Grille g, int x, int y) {
        int nx = tab.Taille_hor();
        int ny = tab.Taille_vert();
        double dx = g.dX();
        double dy = g.dY();

        int index = x + y*nx;

        // bord → renvoie 0 pour éviter dépassement
        if (x <= 0 || x >= nx-1 || y <= 0 || y >= ny-1) return 0.;

        // cylindre -> renvoie 0
        if(g.Solide()[index]) return 0.;

        double tx_plus = g.Solide()[index + 1] ? tab(x,y) : tab (x+1, y);
        double tx_moins = g.Solide()[index - 1] ? tab(x,y) : tab (x-1, y);
        double ty_plus = g.Solide()[index + nx] ? tab(x,y) : tab (x, y+1);
        double ty_moins = g.Solide()[index - nx] ? tab(x,y) : tab (x, y-1);

        double deriv_xx = (tx_plus + tx_moins - 2*tab(x, y)) / (dx*dx);
        double deriv_yy = (ty_plus + ty_moins - 2*tab(x, y)) / (dy*dy);

        return deriv_xx + deriv_yy;
    }

    // Gradient central en X
    double GradX_c(const Champ& tab, Grille g, int x, int y) {
        int nx = tab.Taille_hor();
        double dx = g.dX();

        int index = x + nx*y;

        if (x <= 0 || x >= nx-1) return 0.;
        if (g.Solide()[index]) return 0.;

        double tx_plus = g.Solide()[index + 1] ? tab(x,y) : tab(x+1, y);
        double tx_moins = g.Solide()[index - 1] ? tab(x,y) : tab(x-1, y);

        return (tx_plus - tx_moins) / (2*dx);
    }

    // Gradient central en Y
    double GradY_c(const Champ& tab, Grille g, int x, int y) {
        int nx = tab.Taille_hor();
        int ny = tab.Taille_vert();
        double dy = g.dY();
        int index = x + y*nx;

        if (y <= 0 || y >= ny-1) return 0.;
        if (g.Solide()[index]) return 0.;

        double ty_plus = g.Solide()[index + nx] ? tab(x,y) : tab(x, y+1);
        double ty_moins = g.Solide()[index - nx] ? tab(x,y) : tab(x, y-1);

        return (ty_plus - ty_moins) / (2*dy);
    }

    // Gradient upwind en X selon ux
    double GradX_upwind(const Champ& tab, const Champ& ux, Grille g, int x, int y) {
        int nx = tab.Taille_hor();
        double dx = g.dX();
        int index = x + y*nx;

        if (x <= 0 || x >= nx-1) return 0.;
        if (g.Solide()[index]) return 0.;

        double tx_plus = g.Solide()[index + 1] ? tab(x,y) : tab(x+1, y);
        double tx_moins = g.Solide()[index - 1] ? tab(x,y) : tab(x-1, y);

        double u = ux(x, y);
        if (u >= 0.) return (tab(x, y) - tx_moins) / dx;
        else return (tx_plus - tab(x, y)) / dx;
    }

    // Gradient upwind en Y selon uy
    double GradY_upwind(const Champ& tab, const Champ& uy, Grille g, int x, int y) {
        int ny = tab.Taille_vert();
        int nx = tab.Taille_hor();
        double dy = g.dY();
        int index = x + y*nx;

        if (y <= 0 || y >= ny - 1) return 0.;
        if (g.Solide()[index]) return 0.;

        double ty_plus = g.Solide()[index + nx] ? tab(x, y) : tab(x, y+1);
        double ty_moins = g.Solide()[index - nx] ? tab(x,y) : tab(x, y-1);


        double u = uy(x, y);
        if (u >= 0.) return (tab(x, y) - ty_moins) / dy;
        else return (ty_plus - tab(x, y)) / dy;
    }

    //implémentation solveur SOR
    void PoissonSOR(Champ& phi, const Champ& rhs, const Grille& g, double omega, int maxiter, double tol){
        int nx = g.NX();
        int ny = g.NY();

        double dx = g.dX();
        double dy = g.dY();

        double coef=2.*(1./(dx*dx) + 1./(dy*dy));

        for(int iter = 0; iter < maxiter; iter++){
            double err = 0.;
            for(int x=1; x < nx-1; x++){
                for(int y=1; y < ny-1; y++){
                    int index = x + y*nx;
                    if(g.Solide()[index]) continue;

                    double phi_old = phi(x,y);

                    //gestion des voisins solides

                    double px_plus = g.Solide()[index + 1] ? phi(x,y) : phi(x+1, y);
                    double px_moins = g.Solide()[index - 1] ? phi(x,y) : phi(x-1, y);
                    double py_plus = g.Solide()[index + nx] ? phi(x,y) : phi(x, y+1);
                    double py_moins = g.Solide()[index - nx] ? phi(x,y) : phi(x, y-1);

                    double phi_GS =((px_plus+px_moins)/(dx*dx) + (py_plus+py_moins)/(dy*dy) - rhs(x,y)) / coef;

                    double phi_new = (1. - omega)*phi_old + omega*phi_GS;

                    phi(x,y) = phi_new;

                    err += std::abs(phi_new-phi_old);
                }
            }
            if(err < tol) return;
        }
        std::cout << "Iteration max atteinte pour poisson" << endl;
    }


}
