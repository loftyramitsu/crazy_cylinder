#pragma once

#include <vector>
#include "Champ.h"
#include "Grille.h"

/*
 * Contient les opérateurs discrets pour calcul CFD 2D :
 *  - Laplacien
 *  - Gradient central
 *  - Gradient upwind
 * Les champs sont passés par const référence pour éviter des copies inutiles.
 */
namespace Solveur {

    // Laplacien central 2D d'un champ tab au point (x,y)
    double Laplacien(const Champ& tab, const Grille& g, int x, int y);

    // Gradient central en X au point (x,y)
    double GradX_c(const Champ& tab, const Grille& g, int x, int y);

    // Gradient central en Y au point (x,y)
    double GradY_c(const Champ& tab, const Grille& g, int x, int y);

    // Gradient upwind en X selon la vitesse ux
    double GradX_upwind(const Champ& tab, const Champ& ux, const Grille& g, int x, int y);

    // Gradient upwind en Y selon la vitesse uy
    double GradY_upwind(const Champ& tab, const Champ& uy, const Grille& g, int x, int y);

    //Solveur Poisson via méthode SOR
    void PoissonSOR(Champ& phi, const Champ& rhs, const Grille& grid, double omega, int maxIter, double tol);

    // Restreint un champ fin vers un champ grossier (injection ou moyenne)
    Champ Restriction(const Champ& fine);

    // Prolonge un champ grossier vers un champ fin (interpolation bilinéaire)
    Champ Prolongation(const Champ& coarse, int nx_fine, int ny_fine);

    // Calcule le résidu r = rhs - L(phi)
    Champ Residuel(const Champ& phi, const Champ& rhs, const Grille& g);

    // V-cycle multigrid
    void VCycle(Champ& phi, const Champ& rhs, const Grille& g, int niveau, int max_niveaux, int nu1, int nu2);

    // Remplace PoissonSOR pour l'appel depuis Liquide
    void PoissonMultigrid(Champ& phi, const Champ& rhs, const Grille& g, int maxiter, double tol, int nu1=8, int nu2=8);
}

