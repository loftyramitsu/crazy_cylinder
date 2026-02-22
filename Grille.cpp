#include <vector>
#include "Grille.h"

/*
 * Remplit le tableau "solide" avec true si la cellule est à l'intérieur du cylindre
 * La conversion indices → coordonnées physiques est maintenant correcte
 */
void Grille::SetBoolCylindre() {
    double cx = this->pos_x_cyl;  // position du centre en x (physique)
    double cy = this->pos_y_cyl;  // position du centre en y (physique)
    double r  = this->radius_cyl; // rayon du cylindre

    for(int y = 0; y < this->Ny; y++){
        for(int x = 0; x < this->Nx; x++){
            // distance en coordonnées physiques
            double ddx = x * dx - cx;
            double ddy = y * dy - cy;
            // la condition ddx>=0 && ddy>=0 est conservée mais pas strictement nécessaire
            if(ddx >= 0 && ddy >= 0){
                // cellule solide si à l'intérieur du cylindre
                this->solide[x + y*Nx] = (ddx*ddx + ddy*ddy <= r*r);
            }
        }
    }
}