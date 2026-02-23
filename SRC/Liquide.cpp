#include <vector>
#include <math.h>
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
            if (this->grid.Solide()[x+y*nx]==false){
                rhs(x,y) = (rho_l/dt)*(*this).div_u_star(x,y);
            } 
        }
    }

    PoissonSOR(this->p,rhs, this->grid, omega, Maxiter, eps);
}

void Liquide::calc_ux_star(int x, int y, double dt){
    double ConvecX=(*this).convection(this->ux,x,y);
    double DifusX=this->visc*Laplacien(this->ux,this->grid,x,y);

    (*this).Ux_star(x,y) = this->ux(x,y)+dt*(DifusX-ConvecX);
}

void Liquide::calc_uy_star(int x, int y, double dt){
    double g = 9.81;
    
    double ConvecY=(*this).convection(this->uy,x,y);
    double DifusY=this->visc*Laplacien(this->uy,this->grid,x,y);

    (*this).Uy_star(x,y) = this->uy(x,y) + dt*( DifusY - ConvecY + g);
}

void Liquide::calc_tot_U_star(double dt){
    int nx=(*this).Grid().NX();
    int ny=(*this).Grid().NY();
    for(int y=0; y<ny;y++){
        for(int x=0; x<nx; x++){
            if(this->grid.Solide()[x+y*nx]==false){
                (*this).calc_ux_star(x,y,dt);
                (*this).calc_uy_star(x,y,dt);
            }
        }
    }
}

void Liquide::Contrib(double dt){
    int nx = this->grid.NX();
    int ny = this->grid.NY();
    for(int y=1;y < ny; y++){
        for(int x=0; x<nx ; x++){
            if(this->grid.Solide()[x+y*nx]==false){
                double dpdx = GradX_c( (*this).P(),this->grid,x,y);
                double dpdy = GradY_c( (*this).P(),this->grid,x,y);

                this->ux(x,y) = this->ux_star(x,y) - dpdx * dt/ this->rho_l ;
                this->uy(x,y) = this->uy_star(x,y) - dpdy * dt/ this->rho_l ;
            }
            
        }
    }
}

double Liquide::vmax(char c) {
    double v;
    int nx = this->grid.NX();
    int ny = this->grid.NY();
    if(c=='x'){
        v= this->ux(0,0);
        for(int i=1;i<nx*ny;i++){
            Site s = this->ux.site_xy(i);
            if (abs(this->ux[s]) >= v) v= this->ux[s];
        }
    } else if(c=='y'){
        v= this->uy(0,0);
        for(int i=1;i<nx*ny;i++){
            Site s = this->uy.site_xy(i);
            if (abs(this->uy[s]) >= v) v= this->uy[s];
        }
    }

    return v;
}

double Liquide::CFL(){
    double newdt;
    double dx = this->grid.dX();
    double dy = this->grid.dY();
    double uxmax= (*this).vmax('x');
    double uymax= (*this).vmax('y');
    double coef = 4*this->visc;

    double dt_x_inert=dx / uxmax;
    double dt_y_inert=dy / uymax;
    double dt_x_visc= dx*dx/coef;
    double dt_y_visc= dy*dy/coef;

    if (dt_x_inert >= dt_y_inert){
        if (dt_y_visc >= dt_x_visc){
            if (dt_y_inert >= dt_x_visc){
                newdt = dt_x_visc;
            } else {
                newdt = dt_y_inert;
            }
        } else {
            if (dt_y_inert >= dt_y_visc){
                newdt = dt_y_visc;
            } else {
                newdt = dt_y_inert;
            }
        }
    } else {
        if (dt_y_visc >= dt_x_visc){
            if (dt_x_inert >= dt_x_visc){
                newdt = dt_x_visc;
            } else {
                newdt = dt_x_inert;
            }
        } else {
            if (dt_x_inert >= dt_y_visc){
                newdt = dt_y_visc;
            } else {
                newdt = dt_x_inert;
            }
        }
    }

    return 0.99*newdt;
}
