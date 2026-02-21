#ifndef _GRILLE_H_
#define _GRILLE_H_

class Grille{
    private:
    int Nx, Ny;
    double Lx, Ly;
    double dx, dy;

    double pos_x_cyl;
    double pos_y_cyl;
    double radius_cyl;
    std::vector<bool> solide;


    void SetBoolCylindre();

    public:

    Grille(int nx, int ny, double lx, double ly, double cx, double radius=0.)
    : Nx(nx), Ny(ny), Lx(lx), Ly(ly), pos_x_cyl(cx), radius_cyl(radius), solide(nx*ny, false)
    {
        dx=lx/nx;
        dy=ly/ny;
        pos_y_cyl=ly/2.;
        SetBoolCylindre();
    }

    int NX() const {return Nx;}
    int NY() const {return Ny;}
    double LX() const {return Lx;}
    double LY() const {return Ly;}
    double dX() const {return dx;}
    double dY() const {return dy;}
};

#endif