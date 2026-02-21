#include<vector>
#include"Champ.h"
using namespace std;

Site Champ::site_index(int x, int y) const{
    int a=x;
    int b=y;
    a = (a % nx + nx) % nx;
    b = (b % ny + ny) % ny;
    Site s=Site(a+nx*b,a,b);
    return s;
}

Site Champ::site_xy(int index) const{
    int nx= this->nx;
    int r=index % nx;
    int q=index / nx;
    return Site(index,r,q);
}

double Champ::operator() (int x, int y) const{
    int i=y*this->nx+x;
    return this->tab[i];
}

double& Champ::operator() (int x, int y){
    int i=y*this->nx+x;
    return this->tab[i];
}

double Champ::operator[] (Site s) const{
    int i=s.y()*this->nx+s.x();
    return this->tab[i];
}

double& Champ::operator[] (Site s){
    int i=s.y()*this->nx+s.x();
    return this->tab[i];
}