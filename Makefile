CXXFLAGS= -Wall

sources= Grille.cpp Champ.cpp Liquide.cpp Solveur.cpp
entetes= Grille.h Champ.h Liquide.h Solveur.h
objets=${sources:.cpp=.o}

%: %.o
	$(LINK.cpp) -o $@ $^

Projet: $(objets)
	$(LINK.cpp) -o $@ $^

###

clean:
	rm -f *~ *.o *.bak

mrproper:
	rm -f Projet

depend:
	makedepend $(sources)