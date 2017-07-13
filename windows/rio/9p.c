/*s: windows/rio/9p.c */
#include <u.h>
#include <libc.h>

// for dat.h
#include <draw.h>
#include <mouse.h>
#include <cursor.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <thread.h>

#include "dat.h"
#include "fns.h"

/*s: global messagesize */
int	messagesize = 8192+IOHDRSZ;	/* good start */
/*e: global messagesize */

/*s: function filsysrespond */
Xfid*
filsysrespond(Filsys *fs, Xfid *x, Fcall *fc, char *err)
{
    int n;

    /*s: [[filsysrespond()]] if err */
    if(err){
        fc->type = Rerror;
        fc->ename = err;
    }
    /*e: [[filsysrespond()]] if err */
    else
        fc->type = x->req.type+1; // Reply type just after Transmit type

    fc->fid = x->req.fid;
    fc->tag = x->req.tag;

    /*s: [[filsysrespond()]] sanitize buf */
    if(x->buf == nil)
        x->buf = malloc(messagesize);
    /*e: [[filsysrespond()]] sanitize buf */
    n = convS2M(fc, x->buf, messagesize);
    /*s: [[filsysrespond()]] sanity check n */
    if(n <= 0)
        error("convert error in convS2M");
    /*e: [[filsysrespond()]] sanity check n */

    if(write(fs->sfd, x->buf, n) != n)
        error("write error in respond");
    /*s: [[filsysrespond()]] dump Fcall t if debug */
    if(DEBUG)
        fprint(STDERR, "rio:->%F\n", fc);
    /*e: [[filsysrespond()]] dump Fcall t if debug */
    free(x->buf);
    x->buf = nil;
    return x;
}
/*e: function filsysrespond */

/*s: function filsyscancel */
void
filsyscancel(Xfid *x)
{
    if(x->buf){
        free(x->buf);
        x->buf = nil;
    }
}
/*e: function filsyscancel */
/*e: windows/rio/9p.c */
