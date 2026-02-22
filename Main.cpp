#include <iostream>
#include <math.h>
#include "Grille.h"
#include "Champ.h"
#include "Liquide.h"
#include "Solveur.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    // ----------------------
    // Paramètres de la grille / fluide
    int nx = 10, ny = 10;
    double lx = 1.0, ly = 1.0;
    double nu = 0.0, rho = 1.0;
    double U = 0.0, p0 = 0.0;
    double cx = lx/2., radius = 0.1; // pas de cylindre

    // Création du fluide
    Liquide fluide(nx, ny, lx, ly, nu, rho, cx, radius, U, p0);

    // ----------------------
    // 1️⃣ Test divergence
    // Champ simple : ux = x, uy = y
    for (int x = 0; x < nx; x++){
        for (int y = 0; y < ny; y++){
            fluide.Ux(x,y) = x * fluide.Grid().dX();
            fluide.Uy(x,y) = y * fluide.Grid().dY();
        }
    }

    std::cout << "Divergence du champ ux=x, uy=y :\n";
    for (int y = 1; y < ny - 1; y++){
        for (int x = 1; x < nx - 1; x++){
            double div = fluide.div_u(x,y);
            std::cout << div << "\t";
        }
        std::cout << "\n";
    }

    // ----------------------
    // 2️⃣ Test convection
    // Champ u = ux^2 + uy^2
    Champ u(nx, ny);
    for (int x = 0; x < nx; x++){
        for (int y = 0; y < ny; y++){
            u(x,y) = fluide.Ux(x,y)*fluide.Ux(x,y) + fluide.Uy(x,y)*fluide.Uy(x,y);
        }
    }

    std::cout << "\nConvection du champ u=Ux^2+Uy^2 :\n";
    for (int y = 1; y < ny-1; y++){
        for (int x = 1; x < nx-1; x++){
            double conv = fluide.convection(u, x, y);
            std::cout << conv << "\t";
        }
        std::cout << "\n";
    }

    // ----------------------
    // 3️⃣ Test Laplacien
    // Champ arbitraire : sin(pi*x)*sin(pi*y)
    Champ v(nx, ny);
    double dx = lx / nx;
    double dy = ly / ny;

    for (int x = 0; x < nx; x++){
        for (int y = 0; y < ny; y++){
            double X = x*dx + dx/2.0; // centre de la cellule
            double Y = y*dy + dy/2.0;
            v(x,y) = sin(M_PI*X)*sin(M_PI*Y);
        }
    }

    std::cout << "\nLaplacien du champ sin(pi*x)*sin(pi*y) :\n";
    for (int y = 1; y < ny-1; y++){
        for (int x = 1; x < nx - 1; x++){
            double lap = Solveur::Laplacien(v, fluide.Grid(), x, y);
            std::cout << lap << "\t";
        }
        std::cout << "\n";
    }

    return 0;
}
