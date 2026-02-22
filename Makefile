CXX = g++
CXXFLAGS = -Wall -std=c++17

sources = Main.cpp Grille.cpp Champ.cpp Liquide.cpp Solveur.cpp
entetes = Grille.h Champ.h Liquide.h Solveur.h
objets = $(sources:.cpp=.o)

TARGET = Projet

all: $(TARGET)

$(TARGET): $(objets)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(entetes)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *~ *.o *.bak

mrproper: clean
	rm -f $(TARGET)

depend:
	makedepend $(sources)