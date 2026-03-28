#pragma once

#include <string>
#include <map>

/*
 * Classe Config
 * Lit un fichier INI de la forme :
 *   [section]
 *   clé = valeur
 * et expose des accesseurs typés (int, double, string).
 * Les commentaires commencent par ';' ou '#'.
 */
class Config {
private:
    // Stockage : section -> (clé -> valeur)
    std::map<std::string, std::map<std::string, std::string>> data;

    // Supprime les espaces en début/fin de chaîne
    static std::string trim(const std::string& s);

public:
    // Charge et parse le fichier INI. Lève std::runtime_error si introuvable.
    void load(const std::string& filepath);

    // Accesseurs typés avec valeur par défaut si clé absente
    std::string getString(const std::string& section, const std::string& key, const std::string& defaultVal = "") const;
    int         getInt   (const std::string& section, const std::string& key, int    defaultVal = 0)    const;
    double      getDouble(const std::string& section, const std::string& key, double defaultVal = 0.0)  const;
};
