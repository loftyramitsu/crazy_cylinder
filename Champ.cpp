#include <vector>
#include "Champ.h"

/* 
 * Convertit (x,y) en Site avec gestion périodique
 */
Site Champ::site_index(int x, int y) const {
    int a = (x % nx + nx) % nx;
    int b = (y % ny + ny) % ny;
    return Site(a + nx*b, a, b);
}

/*
 * Convertit un index 1D en Site
 */
Site Champ::site_xy(int index) const {
    int r = index % nx;
    int q = index / nx;
    return Site(index, r, q);
}

/*
 * Accès par coordonnées (lecture seule)
 */
double Champ::operator()(int x, int y) const {
    int i = y*nx + x;
    return this->tab[i];
}

/*
 * Accès par coordonnées (modifiable)
 */
double& Champ::operator()(int x, int y) {
    int i = y*nx + x;
    return this->tab[i];
}

/*
 * Accès via Site (lecture seule)
 */
double Champ::operator[](Site s) const {
    int i = s.y()*nx + s.x();
    return tab[i];
}

/*
 * Accès via Site (modifiable)
 */
double& Champ::operator[](Site s) {
    int i = s.y()*nx + s.x();
    return tab[i];
}