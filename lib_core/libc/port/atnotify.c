/*s: port/atnotify.c */
#include <u.h>
#include <libc.h>

/*s: constant NFN (port/atnotify.c) */
#define NFN 33
/*e: constant NFN (port/atnotify.c) */
/*s: global onnot */
static  int (*onnot[NFN])(void*, char*);
/*e: global onnot */
/*s: global onnotlock */
static  Lock    onnotlock;
/*e: global onnotlock */

/*s: function notifier */
static
void
notifier(void *v, char *s)
{
    int i;

    for(i=0; i<NFN; i++)
        if(onnot[i] && ((*onnot[i])(v, s))){
            noted(NCONT);
            return;
        }
    noted(NDFLT);
}
/*e: function notifier */

/*s: function atnotify */
int
atnotify(int (*f)(void*, char*), int in)
{
    int i, n, ret;
    static bool init;

    if(!init){
        notify(notifier);
        init = true;       /* assign = */
    }
    ret = 0;
    lock(&onnotlock);
    if(in){
        for(i=0; i<NFN; i++)
            if(onnot[i] == 0) {
                onnot[i] = f;
                ret = 1;
                break;
            }
    }else{
        n = 0;
        for(i=0; i<NFN; i++)
            if(onnot[i]){
                if(ret==0 && onnot[i]==f){
                    onnot[i] = nil;
                    ret = 1;
                }else
                    n++;
            }
        if(n == 0){
            init = false;
            notify(0);
        }
    }
    unlock(&onnotlock);
    return ret;
}
/*e: function atnotify */
/*e: port/atnotify.c */
