#include <iostream>
#include <math.h>

#include "Grille.h"
#include "Champ.h"
#include "Liquide.h"
#include "Solveur.h"
#include "Simulation.h"
#include "Config.h"
#include "Export.h"

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(int argc, char* argv[]) {

	// -------------------------------------------------------
	// Chargement de la configuration
	// -------------------------------------------------------
	std::string configPath = "config.ini";
	if (argc >= 2) configPath = argv[1];  // chemin optionnel en argument

	Config cfg;
	try {
		cfg.load(configPath);
		std::cout << "Configuration chargée depuis : " << configPath << "\n";
	} catch (const std::exception& e) {
		std::cerr << "[ERREUR] " << e.what() << "\n";
		std::cerr << "Usage : " << argv[0] << " [chemin/vers/config.ini]\n";
		return 1;
	}

	// -------------------------------------------------------
	// Lecture des paramètres
	// -------------------------------------------------------

<<<<<<< HEAD
	// Grille
	int    nx     = cfg.getInt   ("grille", "nx",  1024);
	int    ny     = cfg.getInt   ("grille", "ny",  1024);
	double lx     = cfg.getDouble("grille", "lx",  1.0);
	double ly     = cfg.getDouble("grille", "ly",  15.0);

	// Fluide
	double nu     = cfg.getDouble("fluide", "nu",  1e-6);
	double rho    = cfg.getDouble("fluide", "rho", 1.0);
	double U      = cfg.getDouble("fluide", "U",   0.2);
	double p0     = cfg.getDouble("fluide", "p0",  1e5);

	// Cylindre
	double cx     = cfg.getDouble("cylindre", "cx",     lx / 2.);
	double radius = cfg.getDouble("cylindre", "radius", lx / 8);

	// Simulation
	double Tmax    = cfg.getDouble("simulation", "Tmax",    2.0);
	double eps     = cfg.getDouble("simulation", "eps",     1e-2);
	int    maxiter = cfg.getInt   ("simulation", "maxiter", 25);
=======
    // Grille
    int    nx     = cfg.getInt   ("grille", "nx",  256);
    int    ny     = cfg.getInt   ("grille", "ny",  256);
    double lx     = cfg.getDouble("grille", "lx",  0.5);
    double ly     = cfg.getDouble("grille", "ly",  1.0);

    // Fluide
    double nu     = cfg.getDouble("fluide", "nu",  1e-6);
    double rho    = cfg.getDouble("fluide", "rho", 1.0);
    double U      = cfg.getDouble("fluide", "U",   0.15);
    double p0     = cfg.getDouble("fluide", "p0",  1e5);

    // Cylindre
    double cx     = cfg.getDouble("cylindre", "cx",     lx / 2.);
    double radius = cfg.getDouble("cylindre", "radius", 0.2);

    // Simulation
    double Tmax    = cfg.getDouble("simulation", "Tmax",    10.0);
    double eps     = cfg.getDouble("simulation", "eps",     1e-2);
    int    maxiter = cfg.getInt   ("simulation", "maxiter", 25);
>>>>>>> dde40de488d14fbc11225dd58894cd1b4e50bc36

	// Export
	std::string output_dir = cfg.getString("export", "output_dir", "output");

	// Paramètre SOR (calculé, non exposé dans le INI car dépend de ny)
	double omega = 2. / (1. + sin(M_PI / ny));

	// Champ Affiché
	std::string champ = cfg.getString("affichage", "champ", "u_norm");

	// Calcul taille fenêtre
	int maxSize = 800;
	int windowWidth, windowHeight;
	if (lx >= ly) {
		windowWidth  = maxSize;
		windowHeight = static_cast<int>(maxSize * ly / lx);
	} else {
		windowHeight = maxSize;
		windowWidth  = static_cast<int>(maxSize * lx / ly);
	}

	// -------------------------------------------------------
	// Création du fluide et de la simulation
	// -------------------------------------------------------
	Liquide fluide(nx, ny, lx, ly, nu, rho, cx, radius, U, p0);
	Simulation sim(windowWidth, windowHeight, "Simulation cylindre", fluide, champ);

	// Now, .run() contains the physical loop and the OpenGL loop
	sim.run(Tmax, U, eps, omega, maxiter);

	// -------------------------------------------------------
	// Export final des champs
	// -------------------------------------------------------
	std::cout << "\nExport des champs vers '" << output_dir << "/'...\n";
	Export::exportTout(fluide.Ux(), fluide.Uy(), fluide.P(), fluide.Grid(), output_dir);
	std::cout << "Export terminé.\n";

	return 0;
}
