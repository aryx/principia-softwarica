/*s: port/readn.c */
#include <u.h>
#include <libc.h>

/*s: function [[readn]] */
long
readn(fdt f, void *av, long n)
{
    char *a;
    long m, t;

    a = av;
    t = 0;
    while(t < n){
        m = read(f, a+t, n-t);
        if(m <= 0){
            if(t == 0)
                return m;
            break;
        }
        t += m;
    }
    return t;
}
/*e: function [[readn]] */
/*e: port/readn.c */
