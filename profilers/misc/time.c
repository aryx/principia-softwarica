/*s: misc/time.c */
#include <u.h>
#include <libc.h>

/*s: global [[output]] */
char	output[4096];
/*e: global [[output]] */
void	add(char*, ...);
void	notifyf(void*, char*);

/*s: function [[error]] */
static void
error(char *s)
{

    fprint(2, "time: %s: %r\n", s);
    exits(s);
}
/*e: function [[error]] */

/*s: function [[main]] */
void
main(int argc, char *argv[])
{
    int i;
    Waitmsg *w;
    long l;
    char *p;
    char err[ERRMAX];

    if(argc <= 1){
        fprint(2, "usage: time command\n");
        exits("usage");
    }

    switch(fork()){
    case -1:
        error("fork");
    case 0:
        exec(argv[1], &argv[1]);
        if(argv[1][0] != '/' && strncmp(argv[1], "./", 2) &&
           strncmp(argv[1], "../", 3)){
            sprint(output, "/bin/%s", argv[1]);
            exec(output, &argv[1]);
        }
        error(argv[1]);
    }

    notify(notifyf);

    loop:
    w = wait();
    if(w == nil){
        errstr(err, sizeof err);
        if(strcmp(err, "interrupted") == 0)
            goto loop;
        error("wait");
    }
    l = w->time[0];
    add("%ld.%.2ldu", l/1000, (l%1000)/10);
    l = w->time[1];
    add("%ld.%.2lds", l/1000, (l%1000)/10);
    l = w->time[2];
    add("%ld.%.2ldr", l/1000, (l%1000)/10);
    add("\t");
    for(i=1; i<argc; i++){
        add("%s", argv[i], 0);
        if(i>4){
            add("...");
            break;
        }
    }
    if(w->msg[0]){
        p = utfrune(w->msg, ':');
        if(p && p[1])
            p++;
        else
            p = w->msg;
        add(" # status=%s", p);
    }
    fprint(2, "%s\n", output);
    exits(w->msg);
}
/*e: function [[main]] */

/*s: function [[add]] */
void
add(char *a, ...)
{
    static int beenhere=0;
    va_list arg;

    if(beenhere)
        strcat(output, " ");
    va_start(arg, a);
    vseprint(output+strlen(output), output+sizeof(output), a, arg);
    va_end(arg);
    beenhere++;
}
/*e: function [[add]] */


/*s: function [[notifyf]] */
void
notifyf(void *a, char *s)
{
    USED(a);
    if(strcmp(s, "interrupt") == 0)
        noted(NCONT);
    noted(NDFLT);
}
/*e: function [[notifyf]] */
/*e: misc/time.c */
