#pragma once

#include <vector>
#include "Grille.h"
#include "Champ.h"

/*
 * Classe Liquide
 * Contient :
 *  - ux, uy : vitesses
 *  - p : pression
 *  - ux_star, uy_star : champs intermédiaires
 * Fournit des fonctions :
 *  - div_u, div_u_star : divergence
 *  - convection : terme convectif upwind
 */


class Lignes{
	private:

		double delta_Pression;
		int N_lignes_pression;
		int N_lignes_vitesse;

		Champ_bool lignes_p;
		Champ_bool lignes_v;
	public:

		Lignes(int nx, int ny, double dp, int Np, int Nv) : delta_Pression(dp), N_lignes_pression(Np), N_lignes_vitesse(Nv), lignes_p(nx,ny), lignes_v(nx,ny) {
			for(int i=0; i< nx*ny; i++){
				Site s= lignes_p.site_xy(i);
				lignes_p[s]=false;
				lignes_v[s]=false;
			}
		}

		//Getters
		double DP() const { return delta_Pression; }
		int NLP() const { return N_lignes_pression; }
		int NLV() const { return N_lignes_vitesse; }

		Champ_bool& L_P() { return lignes_p; }
		std::vector<bool>::reference L_P(int x, int y) {return lignes_p(x,y); }

		Champ_bool& L_V() { return lignes_v; }
		std::vector<bool>::reference L_V(int x, int y) {return lignes_v(x,y); }

		std::vector<double> tranche_p( Champ& p) const;
		void Update_lp( Champ& p);

		std::vector<int> cases_dep_v( Champ& uy) const;
		void Update_lv( Champ& ux,  Champ& uy, double dx, double dy);

		void Reset();
};
class Liquide {
	private:
		double visc;     // viscosité
		double rho_l;    // densité
		Grille grid;

		Champ ux, uy, p;
		Champ ux_star, uy_star;
		Champ vorticite;

		std::vector<Champ> tenseur_deform;  //tenseur des déformations, dans l'ordre xx, yy, xy

		Lignes ligne;

	public:
		Liquide(int nx, int ny, double lx, double ly, double nu, double rho, double rhoc, double cx, double cy, double radius, double U, double p0, double dp, int Np, int Nv)
			: visc(nu), rho_l(rho),
			grid(nx, ny, lx, ly, rhoc, cx, cy, radius),
			ux(nx, ny), uy(nx, ny),
			p(nx, ny), ux_star(nx, ny), uy_star(nx, ny), vorticite(nx,ny), tenseur_deform(3, Champ(nx,ny)), ligne(nx, ny, dp, Np, Nv)
	{
		for (int i = 0; i < nx*ny; i++){
			Site s = p.site_xy(i);
			int x=s.x();
			int y=s.y();
			if(x <= 0 || x >= nx-1 || y <= 0 || y >= ny-1){
				ux_star[s] = 0.;
				uy_star[s] = 0.;
				vorticite[s] = 0.;
				ux[s] = 0.;
				uy[s] = U;
				p[s] = p0;
			} else {
				ux_star[s] = 0.;
				uy_star[s] = 0.;
				vorticite[s] = 0.;
				ux[s] = 0.;
				uy[s] = U;
				p[s] = p0;
				if(grid.Solide()[i] || grid.Solide()[i+1] || grid.Solide()[i-1] || grid.Solide()[i+nx] || grid.Solide()[i-nx]){
					uy[s] = 0.;
				}
			}
		}
	}

		// Accès aux champs (const pour le thread de rendu)
		const Champ& Ux() const { return ux; }
		const Champ& Uy() const { return uy; }
		const Champ& P()  const { return p;  }

		// Accès aux champs
		Champ& Ux() { return ux; }
		double& Ux(int x, int y) { return ux(x, y); }

		Champ& Uy() { return uy; }
		double& Uy(int x, int y) { return uy(x, y); }

		Champ& P() { return p; }
		double& P(int x, int y) { return p(x, y); }

		Champ& Ux_star() { return ux_star; }
		double& Ux_star(int x, int y) { return ux_star(x, y); }

		Champ& Uy_star() { return uy_star; }
		double& Uy_star(int x, int y) { return uy_star(x, y); }

		Champ& Vort() { return vorticite; }
		double& Vort(int x, int y) { return vorticite(x, y); }

		std::vector<Champ>& Deform() { return tenseur_deform; }

		Champ& Compo_Deform(int i) { return tenseur_deform[i]; }
		double& Compo_Deform(int i, int x, int y) { return tenseur_deform[i](x, y); }

		const Grille& Grid() const { return grid; }

		Lignes& Ligne() { return ligne; }

		// Divergence du champ de vitesse
		double div_u(int x,int y) const;
		double div_u_star(int x,int y) const;

		// Rotationnel du champ de vitesse

		double rot_u(int x, int y) const;

		// Terme convectif upwind
		double convection(const Champ& u, int x, int y) const;

		//Résolution eq Poisson pour la pression
		void SolveurPression(double eps, double dt, int Maxiter);

	private:

		//Calcul vitesse intermédiaire en un point x,y
		void calc_ux_star(int x, int y, double dt);
		void calc_uy_star(int x, int y, double dt);

	public:

		//Calcul total de la vitesse intermédiaire
		void calc_tot_U_star(double dt);

		//calcul contribution temps t+dt
		void Contrib(double dt);

		//calcul vorticité
		void calc_vort();

	private:

		//calcul de la vitesse max dans une cellule pour ux ou uy
		double vmax(char x);

	public:

		//calcul de la condition CFL de pas de temps
		double CFL();

		//condition limite:
		void condi_lim(double U);

		//calcul lignes de champs et ligne de niveau
		void lignes_champ_niveau();
		void Reset_lignes();

		//calcul du tenseur des déformation
		void calc_deform();

		//calcul de la force s'exerçant sur le cylindre
		void calc_force(double* Fx, double* Fy) const;
};

