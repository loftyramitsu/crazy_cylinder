#include <vector>
#include "Grille.h"
#include<iostream>

/*
 * Remplit le tableau "solide" avec true si la cellule est à l'intérieur du cylindre
 */
void Grille::SetBoolCylindre() {
	double cx_phys = this -> pos_x_cyl;  // position du centre en x (physique)
	double cy_phys = this -> pos_y_cyl;  // position du centre en y (physique)
	double r_phys  = this -> radius_cyl; // rayon du cylindre

	// Vérification que le cylindre est complètement dans la grille
	if (cx_phys - r_phys < 0 || cx_phys + r_phys > Lx || cy_phys - r_phys < 0 || cy_phys + r_phys > Ly){
		std::cerr << "Erreur : le cylindre dépasse de la grille !\n";
		exit(1);
	}

	// ---> Distance in grid coordinate
	double cx_grid = cx_phys / dx;
	double cy_grid = cy_phys / dy;
	double r_grid = r_phys / dx;

	double r_phys2 = r_phys * r_phys;
	double r_grid2 = r_grid * r_grid;

	for (int y = 0; y < this->Ny; y++){
		double ddy_phys = (y + 0.5) * dy - cy_phys;
		double ddy_grid = y - cy_grid;
		for (int x = 0; x < this->Nx; x++){
			double ddx_phys = (x + 0.5) * dx - cx_phys;
			double ddx_grid = x - cx_grid;

			//true si cellule solide
			// ---> Physical Mask
			this -> solide[x + y * Nx] = (ddx_phys*ddx_phys + ddy_phys*ddy_phys <= r_phys2);
			// ---> Grid Mask, for OpenGL texture
			this -> solide_texture[x + y * Nx] = (ddx_grid*ddx_grid + ddy_grid*ddy_grid <= r_grid2) ? 255 : 0;
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

void Grille::calc_masse(){
	int count = 0;
	double Dx = this->dx;
	double Dy = this->dy;

	for(int x=0; x<this->Nx; x++){
		for(int y=0; y<this->Ny; y++){
			if (this->solide[x +this->Nx*y]) count++;
		}
	}

	this->m_c = this->rho_c * count * Dx * Dy;
}

void Grille::update_posx(double dt, double Fx){
	double acc_x = Fx/this->m_c;
	
	double deltax = (this->vit_x_cyl + acc_x*dt/2)*dt;

	if (pos_x_cyl + radius_cyl + deltax >= Lx - 2*dx || pos_x_cyl - radius_cyl + deltax <= 2*dx){
		return;
	} else {
		this->pos_x_cyl += deltax;
		this->vit_x_cyl += acc_x*dt;
	}

	(*this).SetBoolCylindre();
}
