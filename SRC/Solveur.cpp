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

		if (x <= 0 || x >= nx-1 || g.Solide()[index]) return 0.;
		else {
			double tx_plus = g.Solide()[index + 1] ? tab(x,y) : tab(x+1, y);
			double tx_moins = g.Solide()[index - 1] ? tab(x,y) : tab(x-1, y);

			return (tx_plus - tx_moins) / (2*dx);
		}
	}

	// Gradient central en Y
	double GradY_c(const Champ& tab, const Grille& g, int x, int y) {
		int nx = tab.Taille_hor();
		int ny = tab.Taille_vert();
		double dy = g.dY();
		int index = x + y*nx;

		if (y <= 0 || y >= ny-1 || g.Solide()[index]) return 0.;
		else {
			double ty_plus = g.Solide()[index + nx] ? tab(x,y) : tab(x, y+1);
			double ty_moins = g.Solide()[index - nx] ? tab(x,y) : tab(x, y-1);

			return (ty_plus - ty_moins) / (2*dy);
		}
	}

	// Gradient upwind en X selon ux
	double GradX_upwind(const Champ& tab, const Champ& ux, const Grille& g, int x, int y) {
		int nx = tab.Taille_hor();
		double dx = g.dX();
		int index = x + y*nx;

		if (x <= 0 || x >= nx-1 || g.Solide()[index]) return 0.;
		else {
			double tx_plus = g.Solide()[index + 1] ? tab(x,y) : tab(x+1, y);
			double tx_moins = g.Solide()[index - 1] ? tab(x,y) : tab(x-1, y);

			double u = ux(x, y);
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

		if (y <= 0 || y >= ny-1 || g.Solide()[index]) return 0.;
		else {
			double ty_plus = g.Solide()[index + nx] ? tab(x, y) : tab(x, y+1);
			double ty_moins = g.Solide()[index - nx] ? tab(x,y) : tab(x, y-1);

			double u = uy(x, y);
			if (u >= 0.) return (tab(x, y) - ty_moins) / dy;
			else return (ty_plus - tab(x, y)) / dy;
		}
	}

	//implémentation solveur SOR (sur relaxation)
	void PoissonSOR(Champ& phi, const Champ& rhs, const Grille& g, double omega, int maxiter, double tol){
		int nx = g.NX(), ny = g.NY();
		double dx = g.dX(), dy = g.dY();
		double coef=2.*(1./(dx*dx) + 1./(dy*dy));

		for(int iter = 0; iter < maxiter; iter++){
			double err = 0.;
			for(int x=1; x < nx-1; x++){
				for(int y=1; y < ny-1; y++){
					int index = x + y*nx;
					if(x <= 0 || x >= nx-1 || y <= 0 || y >= ny-1 || g.Solide()[index]) continue;

					double phi_old = phi(x,y);

					//gestion des voisins solides

					double px_plus = g.Solide()[index + 1] ? phi(x,y) : phi(x+1, y);
					double px_moins = g.Solide()[index - 1] ? phi(x,y) : phi(x-1, y);
					double py_plus = g.Solide()[index + nx] ? phi(x,y) : phi(x, y+1);
					double py_moins = g.Solide()[index - nx] ? phi(x,y) : phi(x, y-1);

					double phi_GS =((px_plus+px_moins)/(dx*dx) + (py_plus+py_moins)/(dy*dy) - rhs(x,y)) / coef;

					//calcul par sur-relaxation
					double phi_new = (1. - omega)*phi_old + omega*phi_GS;

					phi(x,y) = phi_new;

					err += std::abs(phi_new-phi_old);
				}
			}
			if(err < tol) return;
		}
	}


	Champ Restriction(const Champ& fine) {
		int nx  = fine.Taille_hor()  / 2;
		int ny  = fine.Taille_vert() / 2;
		int fnx = fine.Taille_hor();
		int fny = fine.Taille_vert();
		Champ coarse(nx, ny);

		for (int y = 0; y < ny; y++){
			for (int x = 0; x < nx; x++) {
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
		return coarse;
	}

	Champ Prolongation(const Champ& coarse, int nx_fine, int ny_fine) {
		Champ fine(nx_fine, ny_fine);
		int nx_c = coarse.Taille_hor();
		int ny_c = coarse.Taille_vert();

		for (int y = 0; y < ny_c-1; y++) {
			for (int x = 0; x < nx_c-1; x++) {

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

		// Dernière colonne
		for (int y = 0; y < ny_c-1; y++) {
			fine(2*(nx_c-1), 2*y)   = coarse(nx_c-1, y);
			fine(2*(nx_c-1), 2*y+1) = 0.5 * (coarse(nx_c-1, y) + coarse(nx_c-1, y+1));
		}

		// Dernière ligne
		for (int x = 0; x < nx_c-1; x++) {
			fine(2*x,   2*(ny_c-1)) = coarse(x, ny_c-1);
			fine(2*x+1, 2*(ny_c-1)) = 0.5 * (coarse(x, ny_c-1) + coarse(x+1, ny_c-1));
		}

		// Coin
		fine(2*(nx_c-1), 2*(ny_c-1)) = coarse(nx_c-1, ny_c-1);

		return fine;
	}

	Champ Residuel(const Champ& phi, const Champ& rhs, const Grille& g) {
		int nx = g.NX(), ny = g.NY();
		Champ res(nx, ny);

		for (int y = 0; y < ny; y++){
			for (int x = 0; x < nx; x++) {
				if (g.Solide()[x + y*nx]) continue;
				double Lphi = Laplacien(phi, g, x, y);
				res(x, y) = rhs(x, y) - Lphi;
			}
		}
		return res;
	}

	void VCycle(Champ& phi, const Champ& rhs, const Grille& g, int niveau, int max_niveaux, int nu1, int nu2)
	{
		int nx = g.NX(), ny = g.NY();

		double omega=1.7; //valeur de convergence optimale

		// Cas de base : grille trop petite, résoudre directement
		if (niveau >= max_niveaux || nx <= 4 || ny <= 4) {
			PoissonSOR(phi, rhs, g, omega, 50, 1e-10);
			return;
		}

		// 1. Pre-smoothing
		PoissonSOR(phi, rhs, g, omega, nu1, 1e-10);

		// 2. Calcul du résidu
		Champ res = Residuel(phi, rhs, g);

		// 3. Restriction du résidu sur grille grossière
		Champ res_coarse = Restriction(res);

		// 4. Grille grossière
		Grille g_coarse(nx/2, ny/2, g.LX(), g.LY(), g.PosXCyl()/1., g.RadiusCyl());
		Champ err_coarse(nx/2, ny/2); // initialisée à 0

		// 5. Résolution récursive
		VCycle(err_coarse, res_coarse, g_coarse, niveau+1, max_niveaux, nu1, nu2);

		// 6. Prolongation et correction
		Champ err_fine = Prolongation(err_coarse, nx, ny);
		for (int y = 0; y < ny; y++){
			for (int x = 0; x < nx; x++){
				if (!g.Solide()[x + y*nx])
					phi(x, y) += err_fine(x, y);
			}
		}

		// 7. Post-smoothing
		PoissonSOR(phi, rhs, g, omega, nu2, 1e-10);
	}

	void PoissonMultigrid(Champ& phi, const Champ& rhs, const Grille& g, int maxiter, double tol, int nu1, int nu2){
		int Nx=g.NX();
		int Ny=g.NY();
		int max_niveaux= int(log2(std::min(Nx,Ny))-2);
		for (int iter = 0; iter < maxiter; iter++) {
			VCycle(phi, rhs, g, 0, max_niveaux, nu1, nu2);

			// Vérification convergence
			Champ res = Residuel(phi, rhs, g);
			double err = 0.;
			int nx = g.NX(), ny = g.NY();
			for (int y = 0; y < ny; y++)
				for (int x = 0; x < nx; x++)
					err = std::max(err, std::abs(res(x, y)));
			std::cout << "V-cycle " << iter+1 << " err = " << err << std::endl;
			if (err < tol) return;
		}
	}

}
