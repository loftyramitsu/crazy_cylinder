#include <vector>
#include "Liquide.h"
#include "Grille.h"
#include "Champ.h"
#include "Solveur.h"

using namespace Solveur;

/*
 * div(u) = ∂ux/∂x + ∂uy/∂y
 * Utilise les gradients centraux
 */
double Liquide::div_u(int x,int y) const {
    double duxdx = GradX_c(this->ux, this->grid, x, y);
    double duydy = GradY_c(this->uy, this->grid, x, y);
    return duxdx + duydy;
}

/*
 * Divergence du champ intermédiaire u* = (ux_star, uy_star)
 */
double Liquide::div_u_star(int x,int y) const {
    double duxdx = GradX_c(this->ux_star, this->grid, x, y);
    double duydy = GradY_c(this->uy_star, this->grid, x, y);
    return duxdx + duydy;
}

/*
 * Terme convectif d'un champ u selon ux, uy (upwind)
 * u peut être ux, uy
 * Retourne ux*dudx + uy*dudy
 */
double Liquide::convection(const Champ& u, int x, int y) const {
    double dudx = GradX_upwind(u, this->ux, this->grid, x, y);
    double dudy = GradY_upwind(u, this->uy, this->grid, x, y);
    return this->ux(x,y)*dudx + this->uy(x,y)*dudy;
}

void Liquide::SolveurPression(double eps, double dt, double omega, int Maxiter){
    int nx = this->grid.NX();
    int ny = this->grid.NY();
    Champ rhs(nx,ny);

    for(int x = 0; x < nx; x++){
        for(int y = 0; y < ny; y++){
            rhs(x,y) = (rho_l/dt)*(*this).div_u_star(x,y);
        }
    }

    PoissonSOR(this->p,rhs, this->grid, omega, Maxiter, eps);
}

void Liquide::calc_ux_star(int x, int y, double dt){
    double ConvecX=(*this).convection(this->ux,x,y);
    double DifusX=this->visc*Laplacien(this->ux,this->grid,x,y);

    (*this).Ux_star(x,y)=this->ux(x,y)+dt*(DifusX-ConvecX);

}

void Liquide::calc_uy_star(int x, int y, double dt){
    double ConvecY=(*this).convection(this->uy,x,y);
    double DifusY=this->visc*Laplacien(this->uy,this->grid,x,y);

    (*this).Uy_star(x,y)=this->uy(x,y)+dt*(DifusY-ConvecY);

}

void Liquide::calc_tot_U_star(double dt){
    int nx=(*this).Grid().NX();
    int ny=(*this).Grid().NY();
    for(int y=1; y<ny-1;y++){
        for(int x=1; x<nx-1; x++){
            (*this).calc_ux_star(x,y,dt);
            (*this).calc_uy_star(x,y,dt);
        }
    }
}
