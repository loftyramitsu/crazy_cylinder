#ifndef _CHAMP_H_
#define _CHAMP_H_


class Site{

    private:
    int _index;
    int _x,_y;

    friend class Champ;

    public:

    Site (int i, int x, int y) : _index(i), _x(x), _y(y) {}
    Site () = default;

    int x () const { return _x; }
    int y () const { return _y; }
    int i () const { return _index;}
};

class Champ{

    private:

    int nx, ny;

    std::vector<double> tab;

    public:

    Champ(int _nx, int _ny)
    :nx(_nx),ny(_ny){
        tab=std::vector<double>(_nx*_ny);
    }

    std::vector<double>& GetTab() {return tab;}

    int Taille_hor() const {return nx;}
    int Taille_vert() const {return ny;}

    Site site_index (int x, int y) const;

    private:
    Site site_xy (int index) const;

    public:
    double operator() (int x, int y) const;
    double& operator() (int x, int y);
    double operator[] (Site s) const;
    double& operator[] (Site s);
};

#endif