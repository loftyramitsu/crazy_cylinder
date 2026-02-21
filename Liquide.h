#ifndef _LIQUIDE_H_
#define _LIQUIDE_H_

#include<vector>

class Liquide{
    private:
    double visc;
    double rho_l;
    std::vector<double> ux, uy, p;
    std::vector<double> ux_star, uy_star;


};

#endif