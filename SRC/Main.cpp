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

    // Grille
    int    nx     = cfg.getInt   ("grille", "nx",  1024);
    int    ny     = cfg.getInt   ("grille", "ny",  1024);
    double lx     = cfg.getDouble("grille", "lx",  1.0);
    double ly     = cfg.getDouble("grille", "ly",  15.0);

    // Fluide
    double nu     = cfg.getDouble("fluide", "nu",  1e-6);
    double rho    = cfg.getDouble("fluide", "rho", 1.0);
    double U      = cfg.getDouble("fluide", "U",   0.5);
    double p0     = cfg.getDouble("fluide", "p0",  1e5);

    // Cylindre
    double cx     = cfg.getDouble("cylindre", "cx",     lx / 2.);
    double radius = cfg.getDouble("cylindre", "radius", 0.0);

    // Simulation
    double Tmax    = cfg.getDouble("simulation", "Tmax",    2.0);
    double eps     = cfg.getDouble("simulation", "eps",     1e-1);
    int    maxiter = cfg.getInt   ("simulation", "maxiter", 10);

    // Export
    std::string output_dir = cfg.getString("export", "output_dir", "output");

    // Paramètre SOR (calculé, non exposé dans le INI car dépend de ny)
    double omega = 2. / (1. + sin(M_PI / ny));

    // -------------------------------------------------------
    // Création du fluide et de la simulation
    // -------------------------------------------------------
    Liquide fluide(nx, ny, lx, ly, nu, rho, cx, radius, U, p0);
    Simulation sim(nx, ny, "Simulation cylindre", fluide);

    // Now, .run() contains the physical loop and the OpenGL loop
    sim.run(Tmax, U, eps, omega, maxiter);

    // -------------------------------------------------------
    // Boucle temporelle
    // -------------------------------------------------------
//    double T = 0.;
//    while (T < Tmax) {
//        double dt = fluide.CFL();
//        std::cout << "T = " << T << "  dt = " << dt << "\n";
//
//        fluide.calc_tot_U_star(dt);
//        fluide.SolveurPression(eps, dt, omega, maxiter);
//        fluide.Contrib(dt);
//        fluide.condi_lim(U);
//
//        T += dt;
//    }

    // -------------------------------------------------------
    // Export final des champs
    // -------------------------------------------------------
    std::cout << "\nExport des champs vers '" << output_dir << "/'...\n";
    Export::exportTout(fluide.Ux(), fluide.Uy(), fluide.P(), fluide.Grid(), output_dir);
    std::cout << "Export terminé.\n";

    return 0;
}
