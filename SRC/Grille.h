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
		double pos_x_cyl;    
		double pos_y_cyl;
		double radius_cyl;
		std::vector<bool> solide;  // true si la cellule est dans le cylindre
		std::vector<unsigned char> solide_texture;

		// Initialise le masque "solide" selon le cylindre
		void SetBoolCylindre();

	public:
		// Constructeur
		Grille(int nx, int ny, double lx, double ly, double cx, double cy, double radius=0.)
			: Nx(nx), Ny(ny), Lx(lx), Ly(ly), pos_x_cyl(cx), pos_y_cyl(cy), radius_cyl(radius), solide(nx*ny, false), solide_texture(nx*ny, 0)
		{
			dx = lx / nx;
			dy = ly / ny;
			SetBoolCylindre();     // remplit le masque solide
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
		double RadiusCyl() const { return radius_cyl; }

		// Accès au masque de cellules solides
		const std::vector<bool>& Solide() const { return solide; }

		const std::vector<unsigned char>& SolideTexture() const { return solide_texture; }

		void Affiche_cyl();
};

