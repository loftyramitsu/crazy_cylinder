# Compilateur et options
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Sources et headers
sources = Grille.cpp Champ.cpp Liquide.cpp Solveur.cpp
entetes = Grille.h Champ.h Liquide.h Solveur.h
objets = $(sources:.cpp=.o)

# Nom de l'exécutable
TARGET = Projet

# Cible par défaut
all: $(TARGET)

# Règle pour l'exécutable
$(TARGET): $(objets)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Règle générique pour compiler les .o
%.o: %.cpp $(entetes)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Nettoyage des fichiers temporaires
clean:
	rm -f *~ *.o *.bak

mrproper: clean
	rm -f $(TARGET)

# Génération automatique des dépendances (optionnel)
depend:
	makedepend $(sources)