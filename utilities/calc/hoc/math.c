/*s: hoc/math.c */
#include <u.h>
#include <libc.h>

#include "hoc.h"

double	errcheck(double, char*);

/*s: function [[Log]](hoc) */
double
Log(double x)
{
    return errcheck(log(x), "log");
}
/*e: function [[Log]](hoc) */
/*s: function [[Log10]](hoc) */
double
Log10(double x)
{
    return errcheck(log10(x), "log10");
}
/*e: function [[Log10]](hoc) */

/*s: function [[Sqrt]](hoc) */
double
Sqrt(double x)
{
    return errcheck(sqrt(x), "sqrt");
}
/*e: function [[Sqrt]](hoc) */

/*s: function [[Exp]](hoc) */
double
Exp(double x)
{
    return errcheck(exp(x), "exp");
}
/*e: function [[Exp]](hoc) */

/*s: function [[Asin]](hoc) */
double
Asin(double x)
{
    return errcheck(asin(x), "asin");
}
/*e: function [[Asin]](hoc) */

/*s: function [[Acos]](hoc) */
double
Acos(double x)
{
    return errcheck(acos(x), "acos");
}
/*e: function [[Acos]](hoc) */

/*s: function [[Sinh]](hoc) */
double
Sinh(double x)
{
    return errcheck(sinh(x), "sinh");
}
/*e: function [[Sinh]](hoc) */
/*s: function [[Cosh]](hoc) */
double
Cosh(double x)
{
    return errcheck(cosh(x), "cosh");
}
/*e: function [[Cosh]](hoc) */
/*s: function [[Pow]](hoc) */
double
Pow(double x, double y)
{
    return errcheck(pow(x,y), "exponentiation");
}
/*e: function [[Pow]](hoc) */

/*s: function [[integer]](hoc) */
double
integer(double x)
{
    if(x<-2147483648.0 || x>2147483647.0)
        execerror("argument out of domain", 0);
    return (double)(long)x;
}
/*e: function [[integer]](hoc) */

/*s: function [[errcheck]](hoc) */
double
errcheck(double d, char* s)	/* check result of library call */
{
    if(isNaN(d))
        execerror(s, "argument out of domain");
    if(isInf(d, 0))
        execerror(s, "result out of range");
    return d;
}
/*e: function [[errcheck]](hoc) */
/*e: hoc/math.c */
