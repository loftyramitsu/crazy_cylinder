#pragma once

#include <vector>

/*
 * Classe Grille
 * Représente une grille 2D et un obstacle circulaire (cylindre)
 * Contient :
 *  - nx, ny : nombre de cellules
 *  - Lx, Ly : dimensions physiques
 *  - dx, dy : pas de discrétisation
 *  - cylindre : position, rayon et masque solide
 */
class Grille {
	private:
		int Nx, Ny;          // nombre de cellules
		double Lx, Ly;       // dimensions physiques
		double dx, dy;       // pas en x et y

		// Cylindre solide
		double rho_c, m_c;
		double pos_x_cyl;    
		double pos_y_cyl;
		double vit_x_cyl;
		double vit_y_cyl;
		double radius_cyl;
		std::vector<bool> solide;  // true si la cellule est dans le cylindre
		std::vector<unsigned char> solide_texture;

		int block_size;  // taille de bloc de base pour le tiling

		// Initialise le masque "solide" selon le cylindre
		void SetBoolCylindre();

	public:
		// Constructeur
		Grille(int nx, int ny, double lx, double ly, double rho, double cx, double cy, double radius=0.)
			: Nx(nx), Ny(ny), Lx(lx), Ly(ly), rho_c(rho), pos_x_cyl(cx), pos_y_cyl(cy), vit_x_cyl(0.), vit_y_cyl(0.), radius_cyl(radius), solide(nx*ny, false), solide_texture(nx*ny, 0)
		{
			dx = lx / nx;
			dy = ly / ny;
			SetBoolCylindre();     // remplit le masque solide
			calc_masse();          //calcule la masse (linéique) du cylindre
			block_size = detect_cache_L1();
		}

		// Getters
		int NX() const { return Nx; }
		int NY() const { return Ny; }
		double LX() const { return Lx; }
		double LY() const { return Ly; }
		double dX() const { return dx; }
		double dY() const { return dy; }
		double PosXCyl() const { return pos_x_cyl; }
		double PosYCyl() const { return pos_y_cyl; }
		double VitXCyl() const { return vit_x_cyl; }
		double VitYCyl() const { return vit_y_cyl; }
		double RadiusCyl() const { return radius_cyl; }
		double Mc() const { return m_c; }
		double Rhoc() const { return rho_c; }

		//calcule la taille du bloc de base pour le tiling
		int detect_cache_L1();

		//calcule la taille du bloc optimale pour une boucle for donnée
		int Taille_Bloc(int nb_double, int nb_bool) const;

		// Accès au masque de cellules solides
		const std::vector<bool>& Solide() const { return solide; }

		const std::vector<unsigned char>& SolideTexture() const { return solide_texture; }

		void calc_masse();

		void update_posx(double dt, double Fx);

		void Affiche_cyl();
};

