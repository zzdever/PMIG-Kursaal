
#include <complex>

#define G 1
#define P 4
#define V 8

using namespace std;

class Endoh{
public:
    Endoh();
    int endoh(void);

private:
    complex<double> a[97687];
    complex<double> *p, *q, *r = a, w = 0, d;
    int character, x, y;
    char b[6856];
    char *o, *t;

    int Getw(int character);
};

