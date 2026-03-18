/*s: misc/time.c */
#include <u.h>
#include <libc.h>

/*s: global [[output]](time.c) */
char	output[4096];
/*e: global [[output]](time.c) */

//forward decl
void	add(char*, ...);
void	notifyf(void*, char*);

/*s: function [[error]](time.c) */
static void
error(char *s)
{

    fprint(STDERR, "time: %s: %r\n", s);
    exits(s);
}
/*e: function [[error]](time.c) */
/*s: function [[main]](time.c) */
void
main(int argc, char *argv[])
{
    Waitmsg *w;
    long l;
    /*s: [[main()]](time.c) other locals */
    char err[ERRMAX];
    /*x: [[main()]](time.c) other locals */
    int i;
    char *p;
    /*e: [[main()]](time.c) other locals */

    /*s: [[main()]](time.c) usage if no args */
    if(argc <= 1){
        fprint(2, "usage: time command\n");
        exits("usage");
    }
    /*e: [[main()]](time.c) usage if no args */

    switch(fork()){
    case 0:
        // in child
        exec(argv[1], &argv[1]);

        /*s: [[main()]](time.c) in child after [[exec]] */
        if(argv[1][0] != '/' && strncmp(argv[1], "./", 2) &&
           strncmp(argv[1], "../", 3)){
            sprint(output, "/bin/%s", argv[1]);
            exec(output, &argv[1]);
        }
        /*e: [[main()]](time.c) in child after [[exec]] */
        error(argv[1]);
    case -1:
        error("fork");
    }
    // else, in parent

    notify(notifyf);

    loop:
    w = wait();
    /*s: [[main()]](time.c) if [[w == nil]] */
    if(w == nil){
        errstr(err, sizeof err);
        if(strcmp(err, "interrupted") == 0)
            goto loop;
        error("wait");
    }
    /*e: [[main()]](time.c) if [[w == nil]] */
    l = w->time[0];
    add("%ld.%.2ldu", l/1000, (l%1000)/10);
    l = w->time[1];
    add("%ld.%.2lds", l/1000, (l%1000)/10);
    l = w->time[2];
    add("%ld.%.2ldr", l/1000, (l%1000)/10);
    add("\t");

    /*s: [[main()]](time.c) display also argv */
    for(i=1; i<argc; i++){
        add("%s", argv[i], 0);
        if(i>4){
            add("...");
            break;
        }
    }
    /*e: [[main()]](time.c) display also argv */
    /*s: [[main()]](time.c) display also wait message and status */
    if(w->msg[0]){
        p = utfrune(w->msg, ':');
        if(p && p[1])
            p++;
        else
            p = w->msg;
        add(" # status=%s", p);
    }
    /*e: [[main()]](time.c) display also wait message and status */
    fprint(STDERR, "%s\n", output);
    exits(w->msg);
}
/*e: function [[main]](time.c) */
/*s: function [[add]](time.c) */
void
add(char *a, ...)
{
    static bool beenhere=false;
    va_list arg;

    if(beenhere)
        strcat(output, " ");
    va_start(arg, a);
    vseprint(output+strlen(output), output+sizeof(output), a, arg);
    va_end(arg);
    beenhere++;
}
/*e: function [[add]](time.c) */
/*s: function [[notifyf]](time.c) */
void
notifyf(void *a, char *s)
{
    USED(a);
    if(strcmp(s, "interrupt") == 0)
        noted(NCONT);
    noted(NDFLT);
}
/*e: function [[notifyf]](time.c) */
/*e: misc/time.c */
