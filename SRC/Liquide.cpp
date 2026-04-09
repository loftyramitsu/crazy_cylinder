#include <vector>
#include <math.h>
#include "Liquide.h"
#include "Grille.h"
#include "Champ.h"
#include "Solveur.h"

using namespace Solveur;

std::vector<double> Lignes::tranche_p(Champ& _p) const{
	int Nx=_p.Taille_hor();
	int Ny=_p.Taille_vert();
	std::vector<double> list_p = std::vector<double>(this->N_lignes_pression);

	double pmin=_p(0,0);
	double pmax=_p(0,0);

	for(int i=1; i<Nx*Ny; i++){
		Site s = _p.site_xy(i);
		pmin = std::min(pmin, _p[s]);
		pmax = std::max(pmax, _p[s]);
	}

	double delta= pmax-pmin;

	for(int j=1; j<=this->N_lignes_pression ; j++){
		list_p[j-1]=pmin + delta*j/(1+this->N_lignes_pression);
	}
	return list_p;
}

void Lignes::Update_lp( Champ& _p) {
	int Nx=this->lignes_p.Taille_hor();
	int Ny=this->lignes_p.Taille_vert();
	std::vector<double> liste_p = (*this).tranche_p(_p);

	for(int i=0; i<Nx*Ny; i++){
		Site s = this->lignes_p.site_xy(i);
		for(int j=0; j<this->N_lignes_pression; j++){
			if(_p[s] >= liste_p[j]-this->delta_Pression && _p[s] <= liste_p[j]+this->delta_Pression){
				this->lignes_p[s] = true;
			}
		}
	}
}

std::vector<int> Lignes::cases_dep_v( Champ& _uy) const{
	std::vector<int> list_v;
	int Nx = _uy.Taille_hor();

	if(Nx%this->N_lignes_vitesse == 0){
		int quotient = Nx/this->N_lignes_vitesse;

		for(int i=1; i<this->N_lignes_vitesse-1; i++){
			list_v.push_back(i*quotient);
		}
	}else{
		int quotient = Nx/this->N_lignes_vitesse;

		for(int i=1; i<this->N_lignes_vitesse; i++){
			list_v.push_back(i*quotient);
		}
	}

	return list_v;
}

void Lignes::Update_lv( Champ& _ux,  Champ& _uy, double _dx, double _dy){
	int Nx=this->lignes_v.Taille_hor();
	int Ny=this->lignes_v.Taille_vert();

	std::vector<int> X = (*this).cases_dep_v(_uy);
	int N = X.size();

	double alpha_lim =_dy/_dx;

	for(int i=0;i<N; i++){
		int y=0;
		int x=X[i];
		this->lignes_v(x,y)=true;

	
		int maxSteps = Nx * Ny; // on ne peut pas visiter plus de cellules que ça
		int steps = 0;
		/*
		do{
			if(steps++ > maxSteps) break;
			if(_ux(x,y) == 0. && _uy(x,y) == 0.) break;
			if(_ux(x,y)==0.){
				if(_uy(x,y)<0.){
					this->lignes_v(x,y-1) = true;
					y--;
				}else{
					this->lignes_v(x,y+1) = true;
					y++;
				}
			} else if(_ux(x,y)>0.){
				double alpha = _uy(x,y)/_ux(x,y);
				if(alpha < -alpha_lim){
					this->lignes_v(x,y-1) = true;
					y--;
				}else if(alpha > alpha_lim){
					this->lignes_v(x,y+1) = true;
					y++;
				} else {
					this->lignes_v(x+1,y) = true;
					x++;
				}
			} else {
				double alpha = -_uy(x,y)/_ux(x,y);
				if(alpha < -alpha_lim){
					this->lignes_v(x,y-1) = true;
					y--;
				}else if(alpha > alpha_lim){
					this->lignes_v(x,y+1) = true;
					y++;
				} else {
					this->lignes_v(x-1,y) = true;
					x--;
				}
			}
		}while(x>0 && y>0 && x<Nx-1 && y<Ny-1);*/
		double fx = x, fy = y;  // position en flottant

		do {
		    if (steps++ > maxSteps) break;
		    double u = _ux((int)fx, (int)fy);
		    double v = _uy((int)fx, (int)fy);
		    double speed = std::sqrt(u*u + v*v);
 		   if (speed < 1e-10) break;

		    // avance d'un pas normalisé
		    fx += u / speed;
		    fy += v / speed;

		    int ix = (int)fx, iy = (int)fy;
		    if (ix <= 0 || iy <= 0 || ix >= Nx-1 || iy >= Ny-1) break;
		    this->lignes_v(ix, iy) = true;

		} while (true);
	}
}

void Lignes::Reset(){
	int Nx=this->lignes_p.Taille_hor();
	int Ny=this->lignes_p.Taille_vert();
	for(int i=0; i< Nx*Ny; i++){
		Site s= lignes_p.site_xy(i);
		lignes_p[s]=false;
		lignes_v[s]=false;
	}
}

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
 * vorticite=rot(u) = ∂uy/∂x - ∂ux/∂y
 * Utilise les gradients centraux
 */

double Liquide::rot_u(int x, int y) const{
	double duydx = GradX_c(this->uy, this->grid, x, y);
	double duxdy = GradY_c(this->ux, this->grid, x ,y);
	return duydx - duxdy;
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

void Liquide::SolveurPression(double eps, double dt, int Maxiter){
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

	PoissonMultigrid(this->p, rhs, this->grid, Maxiter, eps);
}

void Liquide::calc_ux_star(int x, int y, double dt){
	double ConvecX=(*this).convection(this->ux,x,y);
	double DifusX=this->visc*Laplacien(this->ux,this->grid,x,y);

	(*this).Ux_star(x,y) = this->ux(x,y)+dt*(DifusX-ConvecX);
}

void Liquide::calc_uy_star(int x, int y, double dt){
	double g = 0.;

	double ConvecY=(*this).convection(this->uy,x,y);
	double DifusY=this->visc*Laplacien(this->uy,this->grid,x,y);

	(*this).Uy_star(x,y) = this->uy(x,y) + dt*( DifusY - ConvecY + g);
}

void Liquide::calc_tot_U_star(double dt){
	int nx=(*this).Grid().NX();
	int ny=(*this).Grid().NY();
#pragma omp parallel for collapse(2) schedule(static)
	for(int y=1; y < ny-1; y++){
		for(int x=1; x < nx-1; x++){
			if(!grid.Solide()[x+y*nx]){
				calc_ux_star(x,y,dt);
				calc_uy_star(x,y,dt);
			}
		}
	}
	this -> ux_star(0, 0) = this -> ux(0, 0);
	this -> ux_star(nx-1, 0) = this -> ux(nx-1, 0);
	this -> ux_star(0, ny-1) = this -> ux(0, ny-1);
	this -> ux_star(nx-1, ny-1) = this -> ux(nx-1, ny-1);
}

void Liquide::Contrib(double dt){
	int nx = grid.NX(), ny = grid.NY();
#pragma omp parallel for collapse(2) schedule(static)
	for(int y=1; y < ny-1; y++){
		for(int x=1; x < nx-1 ; x++){
			if(!grid.Solide()[x+y*nx]){
				double dpdx = GradX_c(p, grid,x,y);
				double dpdy = GradY_c(p, grid,x,y);

				ux(x,y) = ux_star(x,y) - dpdx * dt / rho_l ;
				uy(x,y) = uy_star(x,y) - dpdy * dt / rho_l ;
			}

		}
	}
}

void Liquide::calc_vort() {
	int nx = this->grid.NX();
	int ny = this->grid.NY();
	for(int y=1; y<ny-1; y++){
		for(int x=1; x<nx-1; x++){
			this->vorticite(x,y)=(*this).rot_u(x,y);
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
	double dx = this -> grid.dX();
	double dy = this -> grid.dY();
	double uxmax= std::abs((*this).vmax('x'));
	double uymax= std::abs((*this).vmax('y'));
	double coef = 4 * this -> visc;

	double dt_x_inert = dx / uxmax;
	double dt_y_inert = dy / uymax;

	double dt_x_visc  = dx*dx / coef;
	double dt_y_visc  = dy*dy / coef;

	double dt_min_inert= std::min(dt_x_inert,dt_y_inert);
	double dt_min_visc = std::min(dt_x_visc,dt_y_visc);

	newdt=std::min(dt_min_inert, dt_min_visc);

	return 0.3*newdt;
}
/*
void Liquide::condi_lim(double U){
	int nx = this->grid.NX();
	int ny = this->grid.NY();
	for(int i=0;i<nx*ny;i++){
		Site s = this->p.site_xy(i);
		std::vector<bool> S = this->grid.Solide();
		if(s.x() == 0 || s.x() == nx-1 || s.y() == 0 || s.y() == ny-1){
			if(s.x()==0 || s.x()==nx-1){
    			ux[s] = 0.;
			}
			
			//if(s.x()==0){
    		//	ux(0,s.y()) = ux(nx-2,s.y());
			//}			
			//if(s.x()==nx-1){
			//    ux(nx-1,s.y()) = ux(1,s.y());
			//}
			if(s.y() == 0){
				this->uy[s]=U;
			}
			if(s.y()==ny-1){     // sortie
    			this->uy[s]=U;
			}
			
			
		}else{
			if(S[i+1] || S[i-1] || S[i+nx] || S[i-nx]){
				this->ux[s]=0.;
				this->uy[s]=0.;
			}
		}
	}
}
 */

void Liquide::condi_lim(double U) {
    int nx = this->grid.NX();
    int ny = this->grid.NY();
    const std::vector<bool>& S = this->grid.Solide();

    for (int i = 0; i < nx*ny; i++) {
        Site s = this->p.site_xy(i);
        int x = s.x(), y = s.y();

        // Entrée (bas)
        if (y == 0) {
            uy[s] = U;
            ux[s] = 0.;
        }
        // Sortie (haut) — condition convective : le champ sort librement
        else if (y == ny-1) {
            uy(x, ny-1) = uy(x, ny-2);
            ux(x, ny-1) = ux(x, ny-2);
        }
        // Parois latérales — glissement : ux=0, uy libre
        else if (x == 0) {
            ux[s] = 0.;
            //uy[s] = uy(1, y);          // Neumann : gradient nul
			uy[s]=U;
        }
        else if (x == nx-1) {
            ux[s] = 0.;
            //uy[s] = uy(nx-2, y);       // Neumann : gradient nul
			uy[s]=U;
        }
        // No-slip cylindre
        else if (S[i+1] || S[i-1] || S[i+nx] || S[i-nx]) {
            ux[s] = 0.;
            uy[s] = 0.;
        }
    }
	// Correction débit : la moyenne de uy en sortie doit valoir U
    double debit_sortie = 0.;
    for (int x = 0; x < nx; x++)
        debit_sortie += uy(x, ny-1);
    debit_sortie /= nx;

    double correction = U - debit_sortie;
    for (int x = 0; x < nx; x++)
        uy(x, ny-1) += correction;
}

void Liquide::lignes_champ_niveau(){
	this->ligne.Update_lp(this->p);
	this->ligne.Update_lv(this->ux, this->uy, this->grid.dX(), this->grid.dY());
}

void Liquide::Reset_lignes(){
	this->ligne.Reset();
}
