/*s: port/atexit.c */
#include <u.h>
#include <libc.h>

/*s: constant [[NEXIT]] */
#define NEXIT   33
/*e: constant [[NEXIT]] */

typedef struct Onex Onex;
/*s: struct [[Onex]] */
struct Onex{
    void    (*f)(void);
    int pid;
};
/*e: struct [[Onex]] */

/*s: global [[onexlock]] */
static Lock onexlock;
/*e: global [[onexlock]] */
/*s: global [[onex]] */
Onex onex[NEXIT];
/*e: global [[onex]] */

/*s: function [[atexit]] */
int
atexit(void (*f)(void))
{
    int i;

    lock(&onexlock);
    for(i=0; i<NEXIT; i++)
        if(onex[i].f == nil) {
            onex[i].pid = getpid();
            onex[i].f = f;
            unlock(&onexlock);
            return 1;
        }
    unlock(&onexlock);
    return 0;
}
/*e: function [[atexit]] */

#pragma profile off
/*s: function [[exits]] */
void
exits(char *s)
{
    int i, pid;
    void (*f)(void);

    pid = getpid();
    for(i = NEXIT-1; i >= 0; i--)
        if((f = onex[i].f) && pid == onex[i].pid) {
            onex[i].f = nil;
            (*f)();
        }
    _exits(s);
}
/*e: function [[exits]] */
#pragma profile on
/*e: port/atexit.c */
