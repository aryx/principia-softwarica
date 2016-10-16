/*s: port/sin.c */
/*
    C program for floating point sin/cos.
    Calls modf.
    There are no error exits.
    Coefficients are #3370 from Hart & Cheney (18.80D).
*/

#include <u.h>
#include <libc.h>

/*s: constant p0 (port/sin.c) */
#define p0      .1357884097877375669092680e8
/*e: constant p0 (port/sin.c) */
/*s: constant p1 (port/sin.c) */
#define p1     -.4942908100902844161158627e7
/*e: constant p1 (port/sin.c) */
/*s: constant p2 (port/sin.c) */
#define p2      .4401030535375266501944918e6
/*e: constant p2 (port/sin.c) */
/*s: constant p3 (port/sin.c) */
#define p3     -.1384727249982452873054457e5
/*e: constant p3 (port/sin.c) */
/*s: constant p4 (port/sin.c) */
#define p4      .1459688406665768722226959e3
/*e: constant p4 (port/sin.c) */
/*s: constant q0 (port/sin.c) */
#define q0      .8644558652922534429915149e7
/*e: constant q0 (port/sin.c) */
/*s: constant q1 (port/sin.c) */
#define q1      .4081792252343299749395779e6
/*e: constant q1 (port/sin.c) */
/*s: constant q2 (port/sin.c) */
#define q2      .9463096101538208180571257e4
/*e: constant q2 (port/sin.c) */
/*s: constant q3 (port/sin.c) */
#define q3      .1326534908786136358911494e3
/*e: constant q3 (port/sin.c) */

/*s: function sinus */
static
double
sinus(double arg, int quad)
{
    double e, f, ysq, x, y, temp1, temp2;
    int k;

    x = arg;
    if(x < 0) {
        x = -x;
        quad += 2;
    }
    x *= 1/PIO2;    /* underflow? */
    if(x > 32764) {
        y = modf(x, &e);
        e += quad;
        modf(0.25*e, &f);
        quad = e - 4*f;
    } else {
        k = x;
        y = x - k;
        quad += k;
        quad &= 3;
    }
    if(quad & 1)
        y = 1-y;
    if(quad > 1)
        y = -y;

    ysq = y*y;
    temp1 = ((((p4*ysq+p3)*ysq+p2)*ysq+p1)*ysq+p0)*y;
    temp2 = ((((ysq+q3)*ysq+q2)*ysq+q1)*ysq+q0);
    return temp1/temp2;
}
/*e: function sinus */

/*s: function cos */
double
cos(double arg)
{
    if(arg < 0)
        arg = -arg;
    return sinus(arg, 1);
}
/*e: function cos */

/*s: function sin */
double
sin(double arg)
{
    return sinus(arg, 0);
}
/*e: function sin */
/*e: port/sin.c */
