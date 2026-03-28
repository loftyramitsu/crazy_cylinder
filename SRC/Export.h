#pragma once

#include <string>
#include "Champ.h"
#include "Grille.h"

/*
 * Namespace Export
 * Fournit des fonctions d'export des champs physiques en CSV.
 *
 * Format de sortie :
 *   x,y,valeur
 * Une ligne par cellule non-solide (ou toutes si include_solid=true).
 *
 * Les fichiers sont écrits dans le dossier output_dir/.
 */
namespace Export {

    /*
     * Exporte un champ scalaire en CSV.
     * Colonnes : x, y, valeur
     * Le nom du fichier sera : output_dir/nom_champ.csv
     */
    void champCSV(const Champ& champ,
                  const Grille& grille,
                  const std::string& output_dir,
                  const std::string& nom_champ);

    /*
     * Exporte ux, uy et p en trois fichiers CSV séparés.
     * Appelle champCSV pour chacun.
     */
    void exportTout(const Champ& ux,
                    const Champ& uy,
                    const Champ& p,
                    const Grille& grille,
                    const std::string& output_dir);

} // namespace Export
