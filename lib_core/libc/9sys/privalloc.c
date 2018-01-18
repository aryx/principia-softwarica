/*s: 9sys/privalloc.c */
#include <u.h>
#include <libc.h>

/*s: global privlock (9sys/privalloc.c) */
static Lock privlock;
/*e: global privlock (9sys/privalloc.c) */
/*s: global [[privinit]] */
static int  privinit;
/*e: global [[privinit]] */
/*s: global [[privs]] */
static void **privs;
/*e: global [[privs]] */

extern void **_privates;
extern int  _nprivates;

/*s: function [[privalloc]] */
void **
privalloc(void)
{
    void **p;
    int i;

    lock(&privlock);
    if(!privinit){
        privinit = 1;
        if(_nprivates){
            _privates[0] = 0;
            for(i = 1; i < _nprivates; i++)
                _privates[i] = &_privates[i - 1];
            privs = &_privates[i - 1];
        }
    }
    p = privs;
    if(p != nil){
        privs = *p;
        *p = nil;
    }
    unlock(&privlock);
    return p;
}
/*e: function [[privalloc]] */

/*s: function [[privfree]] */
void
privfree(void **p)
{
    lock(&privlock);
    if(p != nil && privinit){
        *p = privs;
        privs = p;
    }
    unlock(&privlock);
}
/*e: function [[privfree]] */
/*e: 9sys/privalloc.c */
