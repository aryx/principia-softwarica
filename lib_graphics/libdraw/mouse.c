/*s: lib_graphics/libdraw/mouse.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>
#include <thread.h>
#include <cursor.h>
#include <mouse.h>

/*s: function moveto */
void
moveto(Mousectl *m, Point pt)
{
    fprint(m->mfd, "m%d %d", pt.x, pt.y);
    m->xy = pt;
}
/*e: function moveto */

/*s: function readmouse */
errorneg1
readmouse(Mousectl *mc)
{
    /*s: [[readmouse()]] flush image before reading mouse */
    if(mc->image)
        flushimage(mc->image->display, true);
    /*e: [[readmouse()]] flush image before reading mouse */
    if(recv(mc->c, &mc->Mouse) < 0){
        fprint(2, "readmouse: %r\n");
        return ERROR_NEG1;
    }
    return OK_0;
}
/*e: function readmouse */

/*s: function _ioproc (lib_graphics/libdraw/mouse.c) */
static
void
_ioproc(void *arg)
{
    int n, nerr;
    char buf[1+5*12]; // /dev/mouse first code char and 5 numbers
    Mouse m;
    Mousectl *mc = arg;
    /*s: [[_ioproc()]] other locals (mouse.c) */
    int one = 1;
    /*e: [[_ioproc()]] other locals (mouse.c) */
 
    threadsetname("mouseproc");
    memset(&m, 0, sizeof m);
    mc->pid = getpid();

    nerr = 0;
    for(;;){
        // blocking call
        n = read(mc->mfd, buf, sizeof buf);
        /*s: [[_ioproc()]] sanity check n from read (mouse.c) */
        if(n != 1+4*12){
            yield();	/* if error is due to exiting, we'll exit here */
            fprint(2, "mouse: bad count %d not 49: %r\n", n);
            if(n<0 || ++nerr>10)
                threadexits("read error");
            continue;
        }
        /*e: [[_ioproc()]] sanity check n from read (mouse.c) */
        nerr = 0;
        switch(buf[0]){
        /*s: [[_ioproc()]] switch buf, resize case, fallthrough m case (mouse.c) */
        case 'r':
            send(mc->resizec, &one);
            /* fall through */
        /*e: [[_ioproc()]] switch buf, resize case, fallthrough m case (mouse.c) */
        case 'm':

            m.xy.x    = atoi(buf+1+0*12);
            m.xy.y    = atoi(buf+1+1*12);
            m.buttons = atoi(buf+1+2*12);
            m.msec    = atoi(buf+1+3*12);

            // send it!
            send(mc->c, &m);

            /*
             * mc->Mouse is updated after send so it doesn't have wrong value
             * if we block during send.
             * This means that programs should receive into mc->Mouse 
             * (see readmouse() above) if they want full synchrony.
             */
            mc->Mouse = m;
            break;
        }
    }
}
/*e: function _ioproc (lib_graphics/libdraw/mouse.c) */

/*s: function initmouse */
Mousectl*
initmouse(char *file, Image *i)
{
    Mousectl *mc;
    /*s: [[initmouse()]] other locals */
    char *t;
    char *sl;
    /*e: [[initmouse()]] other locals */

    mc = mallocz(sizeof(Mousectl), true);

    /*s: [[initmouse()]] sanitize file */
    if(file == nil)
        file = "/dev/mouse";
    /*e: [[initmouse()]] sanitize file */
    mc->mfd = open(file, ORDWR|OCEXEC);
    /*s: [[initmouse()]] sanity check mfd */
    if(mc->mfd < 0 && strcmp(file, "/dev/mouse")==0){
        bind("#m", "/dev", MAFTER);
        mc->mfd = open(file, ORDWR|OCEXEC);
    }
    if(mc->mfd < 0){
        free(mc);
        return nil;
    }
    /*e: [[initmouse()]] sanity check mfd */
    mc->file = strdup(file);

    /*s: [[initmouse()]] set cursor field */
    /*s: [[initmouse()]] set t to /dev/cursor */
    // t = "{basename(file)}/cursor"
    t = malloc(strlen(file)+16);
    /*s: [[initmouse()]] sanity check t */
    if (t == nil) {
        close(mc->mfd);
        free(mc);
        return nil;
    }
    /*e: [[initmouse()]] sanity check t */
    strcpy(t, file);
    sl = utfrrune(t, '/');
    if(sl)
        strcpy(sl, "/cursor");
    else
        strcpy(t, "/dev/cursor");
    /*e: [[initmouse()]] set t to /dev/cursor */
    mc->cfd = open(t, ORDWR|OCEXEC);
    free(t);
    /*e: [[initmouse()]] set cursor field */
    mc->image = i;
    /*s: [[initmouse()]] set channels */
    mc->c       = chancreate(sizeof(Mouse), 0);
    /*x: [[initmouse()]] set channels */
    mc->resizec = chancreate(sizeof(int), 2);
    /*e: [[initmouse()]] set channels */
    /*s: [[initmouse()]] create process */
    proccreate(_ioproc, mc, 4096);
    /*e: [[initmouse()]] create process */

    return mc;
}
/*e: function initmouse */

/*s: function setcursor */
void
setcursor(Mousectl *mc, Cursor *c)
{
    char curs[2*4 + 2*2*16]; // sizeof Cursor

    if(c == nil)
        write(mc->cfd, curs, 0);
    else{
        BPLONG(curs+0*4, c->offset.x);
        BPLONG(curs+1*4, c->offset.y);
        memmove(curs+2*4, c->clr, 2*2*16);
        write(mc->cfd, curs, sizeof curs);
    }
}
/*e: function setcursor */

/*s: function closemouse */
void
closemouse(Mousectl *mc)
{
    /*s: [[closemouse()]] sanity check mc */
    if(mc == nil)
        return;
    /*e: [[closemouse()]] sanity check mc */

    postnote(PNPROC, mc->pid, "kill");

    do ; while(nbrecv(mc->c, &mc->Mouse) > 0);

    close(mc->mfd);
    close(mc->cfd);
    free(mc->file);
    free(mc->c);
    free(mc->resizec);
    free(mc);
}
/*e: function closemouse */


/*s: function mousescrollsize */
int
mousescrollsize(int maxlines)
{
    static int lines, pcnt;
    char *mss;

    if(lines == 0 && pcnt == 0){
        mss = getenv("mousescrollsize");
        if(mss){
            if(strchr(mss, '%') != nil)
                pcnt = atof(mss);
            else
                lines = atoi(mss);
            free(mss);
        }
        if(lines == 0 && pcnt == 0)
            lines = 1;
        if(pcnt>=100)
            pcnt = 100;
    }

    if(lines)
        return lines;
    return pcnt * maxlines/100.0;	
}
/*e: function mousescrollsize */

/*e: lib_graphics/libdraw/mouse.c */
