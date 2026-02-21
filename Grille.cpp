#include<vector>
#include"Grille.h"
using namespace std;

void Grille::SetBoolCylindre(){
    double cx=this->pos_x_cyl;
    double cy=this->pos_y_cyl;
    double r=this->radius_cyl;
    for(int y=0; y<Ny; y++){
            for(int x=0; x<Nx; x++){
                int ddx = x - cx;
                int ddy = y - cy;
                this->solide[x + y*Nx] = (ddx*ddx + ddy*ddy <= r*r);
            }
        }
}