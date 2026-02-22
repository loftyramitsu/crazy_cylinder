#ifndef _SOLVEUR_H_
#define _SOLVEUR_H_

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
    double Laplacien(const Champ& tab, Grille g, int x, int y);

    // Gradient central en X au point (x,y)
    double GradX_c(const Champ& tab, Grille g, int x, int y);

    // Gradient central en Y au point (x,y)
    double GradY_c(const Champ& tab, Grille g, int x, int y);

    // Gradient upwind en X selon la vitesse ux
    double GradX_upwind(const Champ& tab, const Champ& ux, Grille g, int x, int y);

    // Gradient upwind en Y selon la vitesse uy
    double GradY_upwind(const Champ& tab, const Champ& uy, Grille g, int x, int y);
}

#endif