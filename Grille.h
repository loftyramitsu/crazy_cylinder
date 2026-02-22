#ifndef _GRILLE_H_
#define _GRILLE_H_

#include <vector>

/*
 * Classe Grille
 * Représente une grille 2D et un obstacle circulaire (cylindre)
 * Contient :
 *  - nx, ny : nombre de cellules
 *  - Lx, Ly : dimensions physiques
 *  - dx, dy : pas de discrétisation
 *  - cylindre : position, rayon et masque solide
 */
class Grille {
private:
    int Nx, Ny;          // nombre de cellules
    double Lx, Ly;       // dimensions physiques
    double dx, dy;       // pas en x et y

    // Cylindre solide
    double pos_x_cyl;    
    double pos_y_cyl;
    double radius_cyl;
    std::vector<bool> solide;  // true si la cellule est dans le cylindre

    // Initialise le masque "solide" selon le cylindre
    void SetBoolCylindre();

public:
    // Constructeur
    Grille(int nx, int ny, double lx, double ly, double cx, double radius=0.)
        : Nx(nx), Ny(ny), Lx(lx), Ly(ly), pos_x_cyl(cx), radius_cyl(radius), solide(nx*ny, false)
    {
        dx = lx / nx;
        dy = ly / ny;
        pos_y_cyl = ly / 2.;   // centre en y
        SetBoolCylindre();     // remplit le masque solide
    }

    // Getters
    int NX() const { return Nx; }
    int NY() const { return Ny; }
    double LX() const { return Lx; }
    double LY() const { return Ly; }
    double dX() const { return dx; }
    double dY() const { return dy; }

    // Accès au masque de cellules solides
    const std::vector<bool>& Solide() const { return solide; }
};

#endif