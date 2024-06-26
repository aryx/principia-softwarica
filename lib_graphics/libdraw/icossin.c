/*s: lib_graphics/libdraw/icossin.c */
#include	<u.h>
#include	<libc.h>
#include	<draw.h>

/*s: global [[sinus]] */
/*
 * Integer sine and cosine for integral degree argument.
 * Tables computed by (sin,cos)(PI*d/180).
 */
static short sinus[91] = {
    0,	/* 0 */
    18,	/* 1 */
    36,	/* 2 */
    54,	/* 3 */
    71,	/* 4 */
    89,	/* 5 */
    107,	/* 6 */
    125,	/* 7 */
    143,	/* 8 */
    160,	/* 9 */
    178,	/* 10 */
    195,	/* 11 */
    213,	/* 12 */
    230,	/* 13 */
    248,	/* 14 */
    265,	/* 15 */
    282,	/* 16 */
    299,	/* 17 */
    316,	/* 18 */
    333,	/* 19 */
    350,	/* 20 */
    367,	/* 21 */
    384,	/* 22 */
    400,	/* 23 */
    416,	/* 24 */
    433,	/* 25 */
    449,	/* 26 */
    465,	/* 27 */
    481,	/* 28 */
    496,	/* 29 */
    512,	/* 30 */
    527,	/* 31 */
    543,	/* 32 */
    558,	/* 33 */
    573,	/* 34 */
    587,	/* 35 */
    602,	/* 36 */
    616,	/* 37 */
    630,	/* 38 */
    644,	/* 39 */
    658,	/* 40 */
    672,	/* 41 */
    685,	/* 42 */
    698,	/* 43 */
    711,	/* 44 */
    724,	/* 45 */
    737,	/* 46 */
    749,	/* 47 */
    761,	/* 48 */
    773,	/* 49 */
    784,	/* 50 */
    796,	/* 51 */
    807,	/* 52 */
    818,	/* 53 */
    828,	/* 54 */
    839,	/* 55 */
    849,	/* 56 */
    859,	/* 57 */
    868,	/* 58 */
    878,	/* 59 */
    887,	/* 60 */
    896,	/* 61 */
    904,	/* 62 */
    912,	/* 63 */
    920,	/* 64 */
    928,	/* 65 */
    935,	/* 66 */
    943,	/* 67 */
    949,	/* 68 */
    956,	/* 69 */
    962,	/* 70 */
    968,	/* 71 */
    974,	/* 72 */
    979,	/* 73 */
    984,	/* 74 */
    989,	/* 75 */
    994,	/* 76 */
    998,	/* 77 */
    1002,	/* 78 */
    1005,	/* 79 */
    1008,	/* 80 */
    1011,	/* 81 */
    1014,	/* 82 */
    1016,	/* 83 */
    1018,	/* 84 */
    1020,	/* 85 */
    1022,	/* 86 */
    1023,	/* 87 */
    1023,	/* 88 */
    1024,	/* 89 */
    1024,	/* 90 */
};
/*e: global [[sinus]] */

/*s: function [[icossin]] */
void
icossin(int deg, int *cosp, int *sinp)
{
    int sinsign, cossign;
    short *stp, *ctp;

    deg %= 360;
    if(deg < 0)
        deg += 360;
    sinsign = 1;
    cossign = 1;
    stp = 0;
    ctp = 0;
    switch(deg/90){
    case 2:
        sinsign = -1;
        cossign = -1;
        deg -= 180;
        /* fall through */
    case 0:
        stp = &sinus[deg];
        ctp = &sinus[90-deg];
        break;
    case 3:
        sinsign = -1;
        cossign = -1;
        deg -= 180;
        /* fall through */
    case 1:
        deg = 180-deg;
        cossign = -cossign;
        stp = &sinus[deg];
        ctp = &sinus[90-deg];
        break;
    }
    *sinp = sinsign*stp[0];
    *cosp = cossign*ctp[0];
}
/*e: function [[icossin]] */
/*e: lib_graphics/libdraw/icossin.c */
