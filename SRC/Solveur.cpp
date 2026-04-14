#include <vector>
#include "Champ.h"
#include "Grille.h"
#include "Solveur.h"
#include<math.h>
#include<iostream>

using namespace std;

namespace Solveur {

	// Laplacien discretisé avec différences centrales
	double Laplacien(const Champ& tab, const Grille& g, int x, int y) {
		int nx = tab.Taille_hor();
		int ny = tab.Taille_vert();
		double dx = g.dX();
		double dy = g.dY();

		int index = x + y*nx;

		// bord → renvoie 0 pour éviter dépassement
		if (x <= 0 || x >= nx-1 || y <= 0 || y >= ny-1 || g.Solide()[index]) return 0.;
		else {
			double tx_plus = g.Solide()[index + 1] ? tab(x,y) : tab (x+1, y);
			double tx_moins = g.Solide()[index - 1] ? tab(x,y) : tab (x-1, y);
			double ty_plus = g.Solide()[index + nx] ? tab(x,y) : tab (x, y+1);
			double ty_moins = g.Solide()[index - nx] ? tab(x,y) : tab (x, y-1);

			double deriv_xx = (tx_plus + tx_moins - 2*tab(x, y)) / (dx*dx);
			double deriv_yy = (ty_plus + ty_moins - 2*tab(x, y)) / (dy*dy);

			return deriv_xx + deriv_yy;
		}


	}

	// Gradient central en X
	double GradX_c(const Champ& tab, const Grille& g, int x, int y) {
		int nx = tab.Taille_hor();
		double dx = g.dX();
		int index = x + nx*y;

		/*
		if (g.Solide()[index]) return 0.;

		if (x == 0){
			return (tab(1,y) - tab(0,y))/dx;
		} else if (x == nx-1){
			return (tab(nx-1,y) - tab(nx-2,y))/dx;
			*/
		if(x == 0 || x == nx-1 || g.Solide()[index]){
			return 0.;
		} else {
			double tx_plus = g.Solide()[index + 1] ? tab(x,y) : tab(x+1, y);
			double tx_moins = g.Solide()[index - 1] ? tab(x,y) : tab(x-1, y);

			dx = (g.Solide()[index + 1] || g.Solide()[index - 1]) ? dx : 2*dx;

			return (tx_plus - tx_moins) / dx;
		}
	}

	// Gradient central en Y
	double GradY_c(const Champ& tab, const Grille& g, int x, int y) {
		int nx = tab.Taille_hor();
		int ny = tab.Taille_vert();
		double dy = g.dY();

		int index = x + y*nx;

		/*
		if (g.Solide()[index]) return 0.;

		if (y == 0){
			return (tab(x,1) - tab(x,0))/dy;
		} else if (y == ny-1){
			return (tab(x,ny-1) - tab(x,ny-2))/dy;
			*/
		if( y == 0 || y == ny-1 || g.Solide()[index]){
			return 0.;
		} else {
			double ty_plus = g.Solide()[index + nx] ? tab(x,y) : tab(x, y+1);
			double ty_moins = g.Solide()[index - nx] ? tab(x,y) : tab(x, y-1);

			dy = (g.Solide()[index + nx] || g.Solide()[index - nx]) ? dy : 2*dy;

			return (ty_plus - ty_moins) / dy;
		}
	}

	// Gradient upwind en X selon ux
	double GradX_upwind(const Champ& tab, const Champ& ux, const Grille& g, int x, int y) {
		int nx = tab.Taille_hor();
		double dx = g.dX();
		int index = x + y*nx;
		double u = ux(x, y);

		
		if (g.Solide()[index]) return 0.;

		if (x == 0){
			if (u >= 0.) return 0.;
			else return (tab(1,y) - tab(0,y))/dx;
		} else if (x == nx-1){
			if (u <= 0.) return 0.;
			else return (tab(nx-1,y) - tab(nx-2,y))/dx;
			
		/*
		if(x == 0 || x == nx-1 || g.Solide()[index]){
			return 0.;
			*/
		} else {
			double tx_plus = g.Solide()[index + 1] ? tab(x,y) : tab(x+1, y);
			double tx_moins = g.Solide()[index - 1] ? tab(x,y) : tab(x-1, y);

			if (u >= 0.) return (tab(x, y) - tx_moins) / dx;
			else return (tx_plus - tab(x, y)) / dx;
		}
	}

	// Gradient upwind en Y selon uy
	double GradY_upwind(const Champ& tab, const Champ& uy, const Grille& g, int x, int y) {
		int ny = tab.Taille_vert();
		int nx = tab.Taille_hor();
		double dy = g.dY();
		int index = x + y*nx;
		double u = uy(x, y);

		
		if (g.Solide()[index]) return 0.;

		if (y == 0){
			if (u >= 0.) return 0.;
			else return (tab(x,1) - tab(x,0))/dy;
		} else if (y == ny-1){
			if (u <= 0.) return 0.;
			else return (tab(x,ny-1) - tab(x,ny-2))/dy;
			
		/*
		if( y == 0 || y == ny-1 || g.Solide()[index]){
			return 0.;*/
		} else {
			double ty_plus = g.Solide()[index + nx] ? tab(x, y) : tab(x, y+1);
			double ty_moins = g.Solide()[index - nx] ? tab(x,y) : tab(x, y-1);

			if (u >= 0.) return (tab(x, y) - ty_moins) / dy;
			else return (ty_plus - tab(x, y)) / dy;
		}
	}

	double GradX_avant(const Champ& tab, const Grille& g, int x, int y){
		int nx = tab.Taille_hor();
		double dx = g.dX();
		int index = x + nx*y;

		if (g.Solide()[index] || x == nx-1 || g.Solide()[index+1]) return 0.;
		else return (tab(x+1,y) - tab(x,y))/dx;
	}

	double GradY_avant(const Champ& tab, const Grille& g, int x, int y){
		int ny = tab.Taille_vert();
		int nx = tab.Taille_hor();
		double dy = g.dY();
		int index = x + nx*y;

		if (g.Solide()[index] || y == ny-1 || g.Solide()[index+nx]) return 0.;
		else return (tab(x,y+1) - tab(x,y))/dy;
	}

	double GradX_arriere(const Champ& tab, const Grille& g, int x, int y){
		int nx = tab.Taille_hor();
		double dx = g.dX();
		int index = x + nx*y;

		if (g.Solide()[index] || x == 0 || g.Solide()[index-1]) return 0.;
		else return (tab(x,y) - tab(x-1,y))/dx;
	}

	double GradY_arriere(const Champ& tab, const Grille& g, int x, int y){
		int ny = tab.Taille_vert();
		int nx = tab.Taille_hor();
		double dy = g.dY();
		int index = x + nx*y;

		if (g.Solide()[index] || y == 0 || g.Solide()[index-nx]) return 0.;
		else return (tab(x,y) - tab(x,y-1))/dy;
	}

	//implémentation solveur SOR (sur relaxation)
	void PoissonSOR(Champ& phi, const Champ& rhs, const Grille& g, double omega, int maxiter, double tol){
		int nx = g.NX(), ny = g.NY();
		double dx = g.dX(), dy = g.dY();
		double coef=2.*(1./(dx*dx) + 1./(dy*dy));

		int B = 32; // taille bloc
		

		for(int iter = 0; iter < maxiter; iter++){

			double err = 0.;
		#pragma omp parallel
		{
		
			// Cases rouges (x+y pair)
        	#pragma omp for collapse(2) schedule(static) reduction(+:err)
        	for(int xx=1; xx<nx-1; xx+=B){
        	    for(int yy=1; yy<ny-1; yy+=B){

					int y_max = std::min(yy + B, ny-1);
        			int x_max = std::min(xx + B, nx-1);

        			for (int y = yy; y < y_max; y++){
            			for (int x = xx; x < x_max; x++){

        	        		if ((x+y)%2 != 0) continue;
        	        		int index = x + y*nx;
        	        		if(x <= 0 || x >= nx-1 || y <= 0 || y >= ny-1 || g.Solide()[index]) continue;

            	    		double phi_old = phi(x,y);
                			double px_plus  = g.Solide()[index+1]  ? phi(x,y) : phi(x+1,y);
    	            		double px_moins = g.Solide()[index-1]  ? phi(x,y) : phi(x-1,y);
        	        		double py_plus  = g.Solide()[index+nx] ? phi(x,y) : phi(x,y+1);
            	    		double py_moins = g.Solide()[index-nx] ? phi(x,y) : phi(x,y-1);

            	    		double phi_GS = ((px_plus+px_moins)/(dx*dx) + (py_plus+py_moins)/(dy*dy) - rhs(x,y)) / coef;

        	    			double phi_new = (1.-omega)*phi_old + omega*phi_GS;

        	        		phi(x,y) = phi_new;

        	        		err += std::abs(phi_new - phi_old);
						}
					}
        		}
        	}

			// Cases noires (x+y impair)
        	#pragma omp for collapse(2) schedule(static) reduction(+:err)
        	for(int xx=1; xx<nx-1; xx+=B){
        	    for(int yy=1; yy<ny-1; yy+=B){

					int y_max = std::min(yy + B, ny-1);
        			int x_max = std::min(xx + B, nx-1);

        			for (int y = yy; y < y_max; y++){
            			for (int x = xx; x < x_max; x++){

        	        		if ((x+y)%2 != 1) continue;
        	        		int index = x + y*nx;
        	        		if(x <= 0 || x >= nx-1 || y <= 0 || y >= ny-1 || g.Solide()[index]) continue;

        	        		double phi_old = phi(x,y);

        	        		double px_plus  = g.Solide()[index+1]  ? phi(x,y) : phi(x+1,y);
        	        		double px_moins = g.Solide()[index-1]  ? phi(x,y) : phi(x-1,y);
        	        		double py_plus  = g.Solide()[index+nx] ? phi(x,y) : phi(x,y+1);
        	        		double py_moins = g.Solide()[index-nx] ? phi(x,y) : phi(x,y-1);

        	        		double phi_GS = ((px_plus+px_moins)/(dx*dx) + (py_plus+py_moins)/(dy*dy) - rhs(x,y)) / coef;

        	        		double phi_new = (1.-omega)*phi_old + omega*phi_GS;

        	        		phi(x,y) = phi_new;

        	        		err += std::abs(phi_new - phi_old);
						}
					}
        	    }
        	}
		}

			if(err < tol) return;
		}
	}
    
	void Restriction(const Champ& fine, Champ& coarse) {
		int nx  = fine.Taille_hor()  / 2;
		int ny  = fine.Taille_vert() / 2;
		int fnx = fine.Taille_hor();
		int fny = fine.Taille_vert();

		int B = 32;
		#pragma omp parallel for collapse(2) schedule(static)

		for (int yy = 0; yy < ny; yy += B){
			for (int xx = 0; xx < nx; xx += B) {

				int y_max = std::min(yy + B, ny);
        		int x_max = std::min(xx + B, nx);

        		for (int y = yy; y < y_max; y++){
            		for (int x = xx; x < x_max; x++){
						int fx = 2*x;
						int fy = 2*y;

						// Voisins avec réflexion (Neumann) aux bords
						int fxm = (fx > 0)      ? fx-1 : fx+1;   // réflexion gauche
						int fxp = (fx < fnx-1)  ? fx+1 : fx-1;   // réflexion droite
						int fym = (fy > 0)      ? fy-1 : fy+1;   // réflexion bas
						int fyp = (fy < fny-1)  ? fy+1 : fy-1;   // réflexion haut

						coarse(x, y) = (
								4.0 * fine(fx,  fy)  +
								2.0 * fine(fxm, fy)  +
								2.0 * fine(fxp, fy)  +
								2.0 * fine(fx,  fym) +
								2.0 * fine(fx,  fyp) +
								fine(fxm, fym) +
								fine(fxp, fym) +
								fine(fxm, fyp) +
								fine(fxp, fyp)
							       ) / 16.0;
					}
				}
			}
		}
	}

	void Prolongation(const Champ& coarse, Champ& fine) {
		int nx_c = coarse.Taille_hor();
		int ny_c = coarse.Taille_vert();

	int B = 32; // taille bloc	
	#pragma omp parallel
	{
		#pragma omp for collapse(2) schedule(static)

		for (int yy = 0; yy < ny_c-1; yy += B) {
			for (int xx = 0; xx < nx_c-1; xx += B) {

				int y_max = std::min(yy + B, ny_c-1);
        		int x_max = std::min(xx + B, nx_c-1);

        		for (int y = yy; y < y_max; y++){
            		for (int x = xx; x < x_max; x++){

						double c00 = coarse(x,   y);
						double c10 = coarse(x+1, y);
						double c01 = coarse(x,   y+1);
						double c11 = coarse(x+1, y+1);

						// Cellule coïncidant avec noeud grossier
						fine(2*x,   2*y)   = c00;

						// Cellule entre deux noeuds grossiers en x
						fine(2*x+1, 2*y)   = 0.5 * (c00 + c10);

						// Cellule entre deux noeuds grossiers en y
						fine(2*x,   2*y+1) = 0.5 * (c00 + c01);

						// Cellule centrale entre 4 noeuds grossiers
						fine(2*x+1, 2*y+1) = 0.25 * (c00 + c10 + c01 + c11);
					}
				}
			}
		}

		#pragma omp for schedule(static)

		// Dernière colonne
		for (int y = 0; y < ny_c-1; y++) {
			fine(2*(nx_c-1), 2*y)   = coarse(nx_c-1, y);
			fine(2*(nx_c-1), 2*y+1) = 0.5 * (coarse(nx_c-1, y) + coarse(nx_c-1, y+1));
		}

		#pragma omp for schedule(static)

		// Dernière ligne
		for (int x = 0; x < nx_c-1; x++) {
			fine(2*x,   2*(ny_c-1)) = coarse(x, ny_c-1);
			fine(2*x+1, 2*(ny_c-1)) = 0.5 * (coarse(x, ny_c-1) + coarse(x+1, ny_c-1));
		}
	}

		// Coin
		fine(2*(nx_c-1), 2*(ny_c-1)) = coarse(nx_c-1, ny_c-1);
	}

	void Residuel(const Champ& phi, const Champ& rhs, const Grille& g, Champ& res) {
		int nx = g.NX(), ny = g.NY();

		int B = 32;
		#pragma omp parallel for collapse(2) schedule(static)

		for (int yy = 0; yy < ny; yy += B){
			for (int xx = 0; xx < nx; xx +=B) {

				int y_max = std::min(yy + B, ny);
        		int x_max = std::min(xx + B, nx);

        		for (int y = yy; y < y_max; y++){
            		for (int x = xx; x < x_max; x++){

						if (g.Solide()[x + y*nx]) continue;
						double Lphi = Laplacien(phi, g, x, y);
						res(x, y) = rhs(x, y) - Lphi;
					}
				}
			}
		}
	}

	void VCycle(Champ& phi, const Champ& rhs, const Grille& g, int niveau, int max_niveaux, int nu1, int nu2, std::vector<Champ>& res_levels, std::vector<Champ>& err_levels)
	{
		int nx = g.NX(), ny = g.NY();

		double omega=1.7; //valeur de convergence optimale
		int B = 32; // taille bloc

		// Cas de base : grille trop petite, résoudre directement
		if (niveau >= max_niveaux || nx <= 4 || ny <= 4) {
			PoissonSOR(phi, rhs, g, omega, 50, 1e-10);
			return;
		}

		// 1. Pre-smoothing
		PoissonSOR(phi, rhs, g, omega, nu1, 1e-10);

		// 2. Calcul du résidu
		Champ& res_local = res_levels[niveau];
		Residuel(phi, rhs, g, res_local);

		// 3. Restriction du résidu sur grille grossière
		Champ& res_coarse = res_levels[niveau+1];
		Restriction(res_local, res_coarse);

		// 4. Grille grossière
		Grille g_coarse(nx/2, ny/2, g.LX(), g.LY(), g.Rhoc(), g.PosXCyl(), g.PosYCyl(), g.RadiusCyl());
		Champ err_coarse(nx/2, ny/2); // initialisée à 0

		// 5. Résolution récursive
		VCycle(err_coarse, res_coarse, g_coarse, niveau+1, max_niveaux, nu1, nu2, res_levels, err_levels);

		// 6. Prolongation et correction
		Champ& err_fine = err_levels[niveau];
		Prolongation(err_coarse, err_fine);

		#pragma omp parallel for collapse(2) schedule(static)

		for (int yy = 0; yy < ny; yy += B){
			for (int xx = 0; xx < nx; xx += B){

				int y_max = std::min(yy + B, ny);
        		int x_max = std::min(xx + B, nx);

        		for (int y = yy; y < y_max; y++){
            		for (int x = xx; x < x_max; x++){

						if (g.Solide()[x + y*nx]) continue;
						
						phi(x, y) += err_fine(x, y);
					}
				}
			}
		}

		// 7. Post-smoothing
		PoissonSOR(phi, rhs, g, omega, nu2, 1e-10);
	}

	void PoissonMultigrid(Champ& phi, const Champ& rhs, const Grille& g, int maxiter, double tol, int nu1, int nu2){
		int Nx=g.NX();
		int Ny=g.NY();
		int max_niveaux= int(log2(std::min(Nx,Ny))-2);
		Champ res(Nx, Ny);

		std::vector<Champ> err_levels;
		std::vector<Champ> res_levels;

		err_levels.reserve(max_niveaux);
		res_levels.reserve(max_niveaux);

		for (int i = 0; i <= max_niveaux; i++) {
		    int nx_i = Nx >> i;  // division entière par 2^i
			int ny_i = Ny >> i;
			if (nx_i < 4 || ny_i < 4) break;

		    err_levels.emplace_back(nx_i, ny_i);
    		res_levels.emplace_back(nx_i, ny_i);
		}

		for (int iter = 0; iter < maxiter; iter++) {
			VCycle(phi, rhs, g, 0, max_niveaux, nu1, nu2, res_levels, err_levels);

			// Vérification convergence
			Residuel(phi, rhs, g, res);
			double err = 0.;
			int nx = g.NX(), ny = g.NY();

			#pragma omp parallel for collapse(2) schedule(static) reduction(max:err)

			for (int y = 0; y < ny; y++)
				for (int x = 0; x < nx; x++)
					err = std::max(err, std::abs(res(x, y)));
			std::cout << "V-cycle " << iter+1 << " err = " << err << std::endl;
			if (err < tol) return;
		}
	}

}
