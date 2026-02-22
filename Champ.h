#ifndef _CHAMP_H_
#define _CHAMP_H_

#include <vector>

/*
 * Classe Site
 * Représente un point de la grille avec index 1D et coordonnées 2D
 */
class Site {
private:
    int _index;  // index 1D
    int _x, _y;  // coordonnées
    friend class Champ;

public:
    Site(int i, int x, int y) : _index(i), _x(x), _y(y) {}
    Site() = default;

    int x() const { return _x; }
    int y() const { return _y; }
    int i() const { return _index; }
};

/*
 * Classe Champ
 * Représente un champ scalaire ou vecteur sur la grille
 */
class Champ {
private:
    int nx, ny;             // dimensions
    std::vector<double> tab; // stockage 1D

public:
    // Constructeur
    Champ(int _nx, int _ny) : nx(_nx), ny(_ny) {
        tab = std::vector<double>(_nx*_ny);
    }

    // Accès au vecteur complet
    std::vector<double>& GetTab() { return tab; }

    // Getters dimension
    int Taille_hor() const { return nx; }
    int Taille_vert() const { return ny; }

    // Convertit coordonnées x,y en Site (modulo pour conditions périodiques)
    Site site_index(int x, int y) const;

private:
    // Convertit un index 1D en Site
    Site site_xy(int index) const;

public:
    // Accès à une valeur par coordonnées
    double operator()(int x, int y) const;
    double& operator()(int x, int y);

    // Accès via Site
    double operator[](Site s) const;
    double& operator[](Site s);
};

#endif