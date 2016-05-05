/*s: lib_graphics/libdraw/keyboard.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <keyboard.h>

/*s: function _ioproc */
static
void
_ioproc(void *arg)
{
    Keyboardctl *kc = arg;
    int m, n;
    char buf[20];
    Rune r;

    threadsetname("kbdproc");
    kc->pid = getpid();

    n = 0;
    for(;;){
        while(n>0 && fullrune(buf, n)){
            m = chartorune(&r, buf);
            n -= m;
            memmove(buf, buf+m, n);
            // sent it!
            send(kc->c, &r);
        }
        // blocking call
        m = read(kc->consfd, buf+n, sizeof buf-n);
        /*s: [[_ioproc()]] sanity check m from read (keyboard.c) */
        if(m <= 0){
            yield();	/* if error is due to exiting, we'll exit here */
            fprint(2, "keyboard read error: %r\n");
            threadexits("error");
        }
        /*e: [[_ioproc()]] sanity check m from read (keyboard.c) */
        n += m;
    }
}
/*e: function _ioproc */

/*s: function ctlkeyboard */
int
ctlkeyboard(Keyboardctl *kc, char *m)
{
    return write(kc->ctlfd, m, strlen(m));
}
/*e: function ctlkeyboard */

/*s: function initkeyboard */
Keyboardctl*
initkeyboard(char *file)
{
    Keyboardctl *kc;
    char *t;

    kc = mallocz(sizeof(Keyboardctl), 1);
    /*s: [[initkeyboard()]] sanity check kc */
    if(kc == nil)
        return nil;
    /*e: [[initkeyboard()]] sanity check kc */

    if(file == nil)
        file = "/dev/cons";
    kc->file = strdup(file);
    kc->consfd = open(file, ORDWR|OCEXEC);

    t = malloc(strlen(file)+16);
    /*s: [[initkeyboard()]] sanity check t and consfd */
    if(kc->consfd<0 || t == nil){
    Error1:
        free(kc);
        return nil;
    }
    /*e: [[initkeyboard()]] sanity check t and consfd */
    sprint(t, "%sctl", file);
    kc->ctlfd = open(t, OWRITE|OCEXEC);
    /*s: [[initkeyboard()]] sanity check ctlfd */
    if(kc->ctlfd < 0){
        fprint(2, "initkeyboard: can't open %s: %r\n", t);
    Error2:
        close(kc->consfd);
        free(t);
        goto Error1;
    }
    /*e: [[initkeyboard()]] sanity check ctlfd */

    if(ctlkeyboard(kc, "rawon") < 0){
        fprint(2, "initkeyboard: can't turn on raw mode on %s: %r\n", t);
        close(kc->ctlfd);
        goto Error2;
    }
    free(t);

    kc->c = chancreate(sizeof(Rune), 20);
    proccreate(_ioproc, kc, 4096);

    return kc;
}
/*e: function initkeyboard */


/*s: function closekeyboard */
void
closekeyboard(Keyboardctl *kc)
{
    if(kc == nil)
        return;

    postnote(PNPROC, kc->pid, "kill");
    close(kc->ctlfd);
    close(kc->consfd);
    free(kc->file);
    free(kc->c);
    free(kc);
}
/*e: function closekeyboard */


/*e: lib_graphics/libdraw/keyboard.c */
