#include <vector>
#include "Grille.h"
#include<iostream>

/*
 * Remplit le tableau "solide" avec true si la cellule est à l'intérieur du cylindre
 */
void Grille::SetBoolCylindre() {
    double cx = this->pos_x_cyl;  // position du centre en x (physique)
    double cy = this->pos_y_cyl;  // position du centre en y (physique)
    double r  = this->radius_cyl; // rayon du cylindre

    // Vérification que le cylindre est complètement dans la grille
    if (cx - r < 0 || cx + r > Lx || cy - r < 0 || cy + r > Ly){
        std::cerr << "Erreur : le cylindre dépasse de la grille !\n";
        exit(1);
    }

    for (int y = 0; y < this->Ny; y++){
        for (int x = 0; x < this->Nx; x++){
            double cell_x = x*dx + dx/2.;
            double cell_y = y*dy + dy/2.;
            // distance en coordonnées physiques
            double ddx = cell_x - cx;
            double ddy = cell_y - cy;
            //true si cellule solide
            this -> solide[x + y*Nx] = (ddx*ddx + ddy*ddy <= r*r);
        }
    }
}

void Grille::Affiche_cyl(){
    for (int x = 0; x < this->Nx; x++){
        for (int y = 0; y < this->Ny; y++){
            if (this -> solide[y + x*this->Nx] == true){
                std::cout << "1" << " ";
            } else {
                std::cout << "0" << " ";
            }
        }
        std::cout<<std::endl;
    }
}
