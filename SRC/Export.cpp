#include "Export.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sys/stat.h>

namespace Export {

    // Crée le dossier de sortie s'il n'existe pas
    static void ensureDir(const std::string& dir) {
        struct stat st;
        if (stat(dir.c_str(), &st) != 0) {
#ifdef _WIN32
            mkdir(dir.c_str());
#else
            mkdir(dir.c_str(), 0755);
#endif
        }
    }

    void champCSV(const Champ& champ,
                  const Grille& grille,
                  const std::string& output_dir,
                  const std::string& nom_champ)
    {
        ensureDir(output_dir);

        std::string path = output_dir + "/" + nom_champ + ".csv";
        std::ofstream file(path);
        if (!file.is_open())
            throw std::runtime_error("Export : impossible d'ouvrir '" + path + "'");

        int nx = grille.NX();
        int ny = grille.NY();
        double dx = grille.dX();
        double dy = grille.dY();

        // En-tête
        file << "x,y," << nom_champ << "\n";

        file << std::scientific << std::setprecision(6);

        for (int y = 0; y < ny; y++) {
            for (int x = 0; x < nx; x++) {
                // Coordonnées physiques du centre de la cellule
                double px = (x + 0.5) * dx;
                double py = (y + 0.5) * dy;
                file << px << "," << py << "," << champ(x, y) << "\n";
            }
        }

        file.close();
        std::cout << "Export : " << path << " écrit (" << nx*ny << " cellules)\n";
    }

    void exportTout(const Champ& ux,
                    const Champ& uy,
                    const Champ& p,
                    const Grille& grille,
                    const std::string& output_dir)
    {
        champCSV(ux, grille, output_dir, "ux");
        champCSV(uy, grille, output_dir, "uy");
        champCSV(p,  grille, output_dir, "p");
    }

} // namespace Export
