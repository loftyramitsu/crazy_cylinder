#include <vector>
#include "Champ.h"
#include "Grille.h"
#include "Solveur.h"

using namespace std;

namespace Solveur {

    // Laplacien discretisé avec différences centrales
    double Laplacien(const Champ& tab, Grille g, int x, int y) {
        int nx = tab.Taille_hor();
        int ny = tab.Taille_vert();
        double dx = g.dX();
        double dy = g.dY();

        // bord → renvoie 0 pour éviter dépassement
        if (x <= 0 || x >= nx-1 || y <= 0 || y >= ny-1) return 0.;

        double deriv_xx = (tab(x+1, y) + tab(x-1, y) - 2*tab(x, y)) / (dx*dx);
        double deriv_yy = (tab(x, y+1) + tab(x, y-1) - 2*tab(x, y)) / (dy*dy);

        return deriv_xx + deriv_yy;
    }

    // Gradient central en X
    double GradX_c(const Champ& tab, Grille g, int x, int y) {
        int nx = tab.Taille_hor();
        double dx = g.dX();

        if (x <= 0 || x >= nx-1) return 0.;
        return (tab(x+1, y) - tab(x-1, y)) / (2*dx);
    }

    // Gradient central en Y
    double GradY_c(const Champ& tab, Grille g, int x, int y) {
        int ny = tab.Taille_vert(); // corrigé pour grille rectangulaire
        double dy = g.dY();

        if (y <= 0 || y >= ny-1) return 0.;
        return (tab(x, y+1) - tab(x, y-1)) / (2*dy);
    }

    // Gradient upwind en X selon ux
    double GradX_upwind(const Champ& tab, const Champ& ux, Grille g, int x, int y) {
        int nx = tab.Taille_hor();
        double dx = g.dX();

        if (x <= 0 || x >= nx-1) return 0.;

        double u = ux(x, y);
        if (u >= 0.) return (tab(x, y) - tab(x-1, y)) / dx;
        else return (tab(x+1, y) - tab(x, y)) / dx;
    }

    // Gradient upwind en Y selon uy
    double GradY_upwind(const Champ& tab, const Champ& uy, Grille g, int x, int y) {
        int ny = tab.Taille_vert();
        double dy = g.dY();

        if (y <= 0 || y >= ny-1) return 0.;

        double u = uy(x, y);
        if (u >= 0.) return (tab(x, y) - tab(x, y-1)) / dy;
        else return (tab(x, y+1) - tab(x, y)) / dy;
    }
}