/*s: port/exp.c */
/*
    exp returns the exponential function of its
    floating-point argument.

    The coefficients are #1069 from Hart and Cheney. (22.35D)
*/

#include <u.h>
#include <libc.h>

/*s: constant p0 (port/exp.c) */
#define p0  .2080384346694663001443843411e7
/*e: constant p0 (port/exp.c) */
/*s: constant p1 (port/exp.c) */
#define p1  .3028697169744036299076048876e5
/*e: constant p1 (port/exp.c) */
/*s: constant p2 (port/exp.c) */
#define p2  .6061485330061080841615584556e2
/*e: constant p2 (port/exp.c) */
/*s: constant q0 (port/exp.c) */
#define q0  .6002720360238832528230907598e7
/*e: constant q0 (port/exp.c) */
/*s: constant q1 (port/exp.c) */
#define q1  .3277251518082914423057964422e6
/*e: constant q1 (port/exp.c) */
/*s: constant q2 (port/exp.c) */
#define q2  .1749287689093076403844945335e4
/*e: constant q2 (port/exp.c) */
/*s: constant log2e */
#define log2e  1.4426950408889634073599247
/*e: constant log2e */
/*s: constant sqrt2 */
#define sqrt2  1.4142135623730950488016887
/*e: constant sqrt2 */
/*s: constant maxf */
#define maxf  10000
/*e: constant maxf */

/*s: function exp */
double
exp(double arg)
{
    double fract, temp1, temp2, xsq;
    int ent;

    if(arg == 0)
        return 1;
    if(arg < -maxf)
        return 0;
    if(arg > maxf)
        return Inf(1);
    arg *= log2e;
    ent = floor(arg);
    fract = (arg-ent) - 0.5;
    xsq = fract*fract;
    temp1 = ((p2*xsq+p1)*xsq+p0)*fract;
    temp2 = ((xsq+q2)*xsq+q1)*xsq + q0;
    return ldexp(sqrt2*(temp2+temp1)/(temp2-temp1), ent);
}
/*e: function exp */
/*e: port/exp.c */
