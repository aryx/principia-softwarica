/*s: core/386/fns.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

// was in main.c, could be in lib/386/libc.c (or even lib/libc.c)

/*s: function [[cistrncmp]](x86) */
int
cistrncmp(char *a, char *b, int n)
{
    unsigned ac, bc;

    while(n > 0){
        ac = *a++;
        bc = *b++;
        n--;

        if(ac >= 'A' && ac <= 'Z')
            ac = 'a' + (ac - 'A');
        if(bc >= 'A' && bc <= 'Z')
            bc = 'a' + (bc - 'A');

        ac -= bc;
        if(ac)
            return ac;
        if(bc == 0)
            break;
    }

    return 0;
}
/*e: function [[cistrncmp]](x86) */

/*s: function [[cistrcmp]](x86) */
int
cistrcmp(char *a, char *b)
{
    int ac, bc;

    for(;;){
        ac = *a++;
        bc = *b++;
    
        if(ac >= 'A' && ac <= 'Z')
            ac = 'a' + (ac - 'A');
        if(bc >= 'A' && bc <= 'Z')
            bc = 'a' + (bc - 'A');
        ac -= bc;
        if(ac)
            return ac;
        if(bc == 0)
            break;
    }
    return 0;
}
/*e: function [[cistrcmp]](x86) */

/*e: core/386/fns.c */
