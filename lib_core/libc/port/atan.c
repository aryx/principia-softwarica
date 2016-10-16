/*s: port/atan.c */
/*
    floating-point arctangent

    atan returns the value of the arctangent of its
    argument in the range [-pi/2,pi/2].

    atan2 returns the arctangent of arg1/arg2
    in the range [-pi,pi].

    there are no error returns.

    coefficients are #5077 from Hart & Cheney. (19.56D)
*/

#include <u.h>
#include <libc.h>

/*s: constant sq2p1 */
#define sq2p1 2.414213562373095048802e0
/*e: constant sq2p1 */
/*s: constant sq2m1 */
#define sq2m1  .414213562373095048802e0
/*e: constant sq2m1 */
/*s: constant p4 */
#define p4  .161536412982230228262e2
/*e: constant p4 */
/*s: constant p3 */
#define p3  .26842548195503973794141e3
/*e: constant p3 */
/*s: constant p2 */
#define p2  .11530293515404850115428136e4
/*e: constant p2 */
/*s: constant p1 */
#define p1  .178040631643319697105464587e4
/*e: constant p1 */
/*s: constant p0 */
#define p0  .89678597403663861959987488e3
/*e: constant p0 */
/*s: constant q4 */
#define q4  .5895697050844462222791e2
/*e: constant q4 */
/*s: constant q3 */
#define q3  .536265374031215315104235e3
/*e: constant q3 */
/*s: constant q2 */
#define q2  .16667838148816337184521798e4
/*e: constant q2 */
/*s: constant q1 */
#define q1  .207933497444540981287275926e4
/*e: constant q1 */
/*s: constant q0 */
#define q0  .89678597403663861962481162e3
/*e: constant q0 */


/*s: function xatan */
/*
    xatan evaluates a series valid in the
    range [-0.414...,+0.414...]. (tan(pi/8))
 */

static
double
xatan(double arg)
{
    double argsq, value;

    argsq = arg*arg;
    value = ((((p4*argsq + p3)*argsq + p2)*argsq + p1)*argsq + p0);
    value = value/(((((argsq + q4)*argsq + q3)*argsq + q2)*argsq + q1)*argsq + q0);
    return value*arg;
}
/*e: function xatan */

/*s: function satan */
/*
    satan reduces its argument (known to be positive)
    to the range [0,0.414...] and calls xatan.
 */

static
double
satan(double arg)
{

    if(arg < sq2m1)
        return xatan(arg);
    if(arg > sq2p1)
        return PIO2 - xatan(1/arg);
    return PIO2/2 + xatan((arg-1)/(arg+1));
}
/*e: function satan */

/*s: function atan */
/*
    atan makes its argument positive and
    calls the inner routine satan.
 */

double
atan(double arg)
{

    if(arg > 0)
        return satan(arg);
    return -satan(-arg);
}
/*e: function atan */
/*e: port/atan.c */
