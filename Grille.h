#ifndef _GRILLE_H_
#define _GRILLE_H_

class Grille{
    private:
    int Nx, Ny;
    double Lx, Ly;
    double dx, dy;

    public:

    Grille(int nx, int ny, double lx, double ly)
    : Nx(nx), Ny(ny), Lx(lx), Ly(ly)
    {
        dx=lx/nx;
        dy=ly/ny;
    }

    int NX() const {return Nx;}
    int NY() const {return Ny;}
    double LX() const {return Lx;}
    double LY() const {return Ly;}
    double dX() const {return dx;}
    double dY() const {return dy;}
};

#endif