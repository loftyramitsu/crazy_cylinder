#include <vector>
#include <math.h>
#include "Grille.h"
#include<iostream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

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

	#pragma omp parallel for schedule(static)

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

int Grille::detect_cache_L1() {
#ifdef _WIN32
    DWORD buffer_size = 0;
    GetLogicalProcessorInformation(nullptr, &buffer_size);
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(
        buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    GetLogicalProcessorInformation(buffer.data(), &buffer_size);
    for (auto& info : buffer) {
        if (info.Relationship == RelationCache && info.Cache.Level == 1)
            return info.Cache.Size;
    }
    return 32 * 1024;
#elif defined(__linux__)
    long size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    if (size > 0) return (int)size;
    return 32 * 1024;
#else
    return 32 * 1024;  // valeur par défaut pour macOS et autres
#endif
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

	#pragma omp parallel for collapse(2) schedule(static) reduction(+:count)

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

int Grille::Taille_Bloc(int nb_doubles, int nb_bools) const {
    // taille mémoire d'un bloc B x B
    // nb_doubles champs double + nb_bools champs bool (solide)
    // B² * (nb_doubles * 8 + nb_bools * 1) <= cache_L1
    
    int bytes_per_cell = nb_doubles * sizeof(double) + nb_bools * sizeof(bool);
    int B = (int)std::sqrt((double)(this->block_size / bytes_per_cell));
    return std::max(8, B);
}
