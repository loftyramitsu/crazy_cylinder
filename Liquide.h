#ifndef _LIQUIDE_H_
#define _LIQUIDE_H_

#include<vector>
#include"Grille.h"
#include"Champ.h"


class Liquide{

    private:
    double visc;
    double rho_l;
    Grille grid;

    Champ ux,uy,p;

    Champ ux_star,uy_star;

    public:

    Liquide(int nx, int ny, double lx, double ly, double nu, double rho)
    : grid(nx,ny,lx,ly), visc(nu), rho_l(rho), ux(nx,ny),uy(nx,ny)
    ,p(nx,ny),ux_star(nx,ny),uy_star(nx,ny){}

    std::vector<double>& Ux() {return ux.GetTab();}
    double& Ux(int x, int y) {return ux(x,y);}

    std::vector<double>& Uy() {return uy.GetTab();}
    double& Uy(int x, int y) {return uy(x,y);}

    std::vector<double>& P() {return p.GetTab();}
    double& P(int x, int y) {return p(x,y);}

    std::vector<double>& Ux_star() {return ux_star.GetTab();}
    double& Ux_star(int x, int y) {return ux_star(x,y);}

    std::vector<double>& Uy_star() {return uy_star.GetTab();}
    double& Uy_star(int x, int y) {return uy_star(x,y);}

};

#endif