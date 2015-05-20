
#include "endoh.h"
#include <QString>

//extern "C"{

Endoh::Endoh()
{
    char bb[100] = "\x1b[2J\x1b[1;1H     ";
    for(int i=0; i<100; i++){
        b[i] = bb[i];
    }
    o = b;
}

int Endoh::Getw(int character){
    /*
    if (character > 10){
        return (character > 32 ? 4[*r++ = w, r] = w + 1, *r = r[5] = character == 35, r += 9 : 0, w - I);
    }
    else {
        return character = w + 2;
    }
    */

    return 0;
}


int Endoh::endoh(void)
{
    /*
    // initial w=0
    while ((character = getc(stdin)) > 0)
        w = character > 10 ?
                    (character > 32 ? (*r = w, r++, r[4] = w + 1, *r = r[5] = character == 35, r += 9) : 0, w - I) :
            (character = w + 2);

    while (1)
    {
        puts(o);
        o = b + 4;

        for (p = a; p[2] = p[1] * 9, p < r; p += 5)
            for (q = a; w = cabs(d = *p - *q) / 2 - 1, q < r; q += 5)
                if (0 < (x = 1 - w))
                    p[2] += w*w;

        for (p = a; p[3] = 1, p < r; p += 5)
            for (q = a; w = cabs(d = *p - *q) / 2 - 1, q < r; q += 5)
                if (0 < (x = 1 - w))
                    p[3] += w*(d*(3 - p[2] - q[2]) * 4 + p[4] * 8 - q[4] * 8) / p[2];

        for (x = 011; x < 2011; x++) b[x] = 0;

        for (p = a; (t = b + 10 + (x = *p*I) + 80 * (y = *p / 2), *p += p[4] += p[3] / 10 * !p[1]), p < r; p += 5)
            x = 0 <= x &&x < 79 && 0 <= y&&y < 23 ? 1[1[*t |= 8, t] |= 4, t += 80] = 1, *t |= 2 : 0;

        for (x = 011; x < 2011; x++)
            b[x] = " '`-.|//,\\" "|\\_" "\\/\x23\n"[x % 80 - 9 ? x[b] : 16];

        //sleep(12321);
    }

    */

    return 0;
}


//}
