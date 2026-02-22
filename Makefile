CXX = g++
CXXFLAGS = -Wall -std=c++17

VPATH = SRC:DEP:OBJ

sources = Main.cpp Grille.cpp Champ.cpp Liquide.cpp Solveur.cpp
entetes = Grille.h Champ.h Liquide.h Solveur.h glfw3.h
objets = $(addprefix OBJ/, $(sources:.cpp=.o))

TARGET = Projet

all: $(TARGET)

$(TARGET): $(objets)
	$(CXX) $(CXXFLAGS) -o $@ $^

OBJ/%.o: %.cpp $(entetes)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean mrproper

clean:
	rm -f *~ *.o *.bak, OBJ/*.o

mrproper: clean
	rm -f $(TARGET)

depend:
	makedepend $(sources)
