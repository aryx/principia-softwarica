/*s: fns.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

int (*isaconfig)(char*, int, ISAConf*) = 0;

// was in main.c, could be in lib/386/libc.c (or even lib/libc.c)

/*s: function cistrncmp */
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
/*e: function cistrncmp */

/*s: function cistrcmp */
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
/*e: function cistrcmp */

/*e: fns.c */
