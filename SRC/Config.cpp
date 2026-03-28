#include "Config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

std::string Config::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

void Config::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Config : impossible d'ouvrir '" + filepath + "'");

    std::string currentSection = "";
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);

        // Lignes vides ou commentaires
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;

        // Section : [nom]
        if (line.front() == '[' && line.back() == ']') {
            currentSection = trim(line.substr(1, line.size() - 2));
            continue;
        }

        // Clé = valeur
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));

        // Supprime commentaire inline (après ';' ou '#')
        for (char c : {';', '#'}) {
            size_t pos = val.find(c);
            if (pos != std::string::npos)
                val = trim(val.substr(0, pos));
        }

        data[currentSection][key] = val;
    }
}

std::string Config::getString(const std::string& section, const std::string& key, const std::string& defaultVal) const {
    auto sit = data.find(section);
    if (sit == data.end()) return defaultVal;
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) return defaultVal;
    return kit->second;
}

int Config::getInt(const std::string& section, const std::string& key, int defaultVal) const {
    std::string val = getString(section, key, "");
    if (val.empty()) return defaultVal;
    try { return std::stoi(val); }
    catch (...) { return defaultVal; }
}

double Config::getDouble(const std::string& section, const std::string& key, double defaultVal) const {
    std::string val = getString(section, key, "");
    if (val.empty()) return defaultVal;
    try { return std::stod(val); }
    catch (...) { return defaultVal; }
}
