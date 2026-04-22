/*s: mothra/getpix.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "mothra.h"

typedef struct Pix Pix;

/*s: struct [[Pix]](mothra) */
struct Pix{
    Pix *next;
    Image *b;
    int width;
    int height;
    char name[NNAME];
};
/*e: struct [[Pix]](mothra) */

/*s: global [[pixcmd]](mothra) */
char *pixcmd[]={
[GIF]   "gif -9t",
[JPEG]  "jpg -9t",
[PNG]   "png -9t",
[BMP]   "bmp -9t",
[ICO]   "ico -c",
};
/*e: global [[pixcmd]](mothra) */

/*s: function [[getimage]](mothra) */
void getimage(Rtext *t, Www *w){
    Action *ap;
    Url *url;
    Image *b;
    int fd, typ;
    char err[512], buf[80], *s;
    Pix *p;

    ap=t->user;
    url=emalloc(sizeof(Url));
    seturl(url, ap->image, w->url->fullname);
    if(debug)
        fprint(STDERR, "getimage: %s from %s\n", ap->image, w->url->fullname);

    if(urlresolve(url) < 0) {
        sysfatal("xxx: %r");
    };
    //snprint(url->fullname, sizeof(url->fullname), "%s%s", 
    //        w->url->fullname,
    //        ap->image
    //        );
    //url->reltext[0] = 0;
    //url->basename[0] = 0;

    if(debug)
        fprint(STDERR, "getimage resolved: %s\n", urlstr(url));

    for(p=w->pix;p!=nil; p=p->next)
        if(strcmp(ap->image, p->name)==0 && ap->width==p->width && ap->height==p->height){
            t->b = p->b;
            w->changed=true;
            return;
        }
    fd=urlget(url, -1);
    if(fd==-1){
    Err:
        snprint(err, sizeof(err), "[img: %s: %r]", urlstr(url));
        free(t->text);
        t->text=strdup(err);
        w->changed=true;
        close(fd);
        goto Out;
    }
    typ = snooptype(fd);
    if(typ < 0 || typ >= nelem(pixcmd) || pixcmd[typ] == nil){
        werrstr("unknown image type");
        goto Err;
    }
    if((fd = pipeline(fd, "exec %s", pixcmd[typ])) < 0)
        goto Err;
    if(ap->width>0 || ap->height>0){
        s = buf;
        /*
         * claude: under -d, save a copy of each decoded bitmap that's
         * about to be piped into resize, one file per image. If resize
         * crashes on an image, the corresponding .bit file survives and
         * can be replayed as `resize -x N -y N < /tmp/mothra-NNN.bit`.
         * We use a counter rather than a hash of the image URL because
         * mothra keeps loading images after a child resize crashes, and
         * we want every intermediate bitmap preserved.
         */
        static int imgcount;
        if(debug)
            s += sprint(s, "tee /tmp/mothra-%d.bit | ", imgcount);
        s += sprint(s, "exec resize");
        if(ap->width>0)
            s += sprint(s, " -x %d", ap->width);
        if(ap->height>0)
            s += sprint(s, " -y %d", ap->height);
        USED(s);
        if(debug){
            fprint(STDERR, "getimage[%d]: piping %s through `%s` for %s\n",
                imgcount, pixcmd[typ], buf, ap->image);
            imgcount++;
        }
        if((fd = pipeline(fd, buf)) < 0)
            goto Err;
    }
    b=readimage(display, fd, 1);
    if(b==0){
        werrstr("can't read image");
        goto Err;
    }
    close(fd);
    p=emalloc(sizeof(Pix));
    nstrcpy(p->name, ap->image, sizeof(p->name));
    p->b=b;
    p->width=ap->width;
    p->height=ap->height;
    p->next=w->pix;
    w->pix=p;
    t->b=b;
    w->changed=true;
Out:
    freeurl(url);
}
/*e: function [[getimage]](mothra) */

/*s: function [[getpix]](mothra) */
void getpix(Rtext *t, Www *w){
    int i, pid, nworker, worker[NXPROC];
    Action *ap;

    nworker = 0;
    for(i=0; i<nelem(worker); i++)
        worker[i] = -1;

    for(;t!=0;t=t->next){
        ap=t->user;
        if(ap && ap->image){
            pid = rfork(RFFDG|RFPROC|RFMEM);
            switch(pid){
            case -1:
                fprint(2, "fork: %r\n");
                break;
            case 0:
                getimage(t, w);
                exits(0);
            default:
                for(i=0; i<nelem(worker); i++)
                    if(worker[i] == -1){
                        worker[i] = pid;
                        nworker++;
                        break;
                    }

                while(nworker == nelem(worker)){
                    if((pid = waitpid()) < 0)
                        break;
                    for(i=0; i<nelem(worker); i++)
                        if(worker[i] == pid){
                            worker[i] = -1;
                            nworker--;
                            break;
                        }
                }
            }
            
        }
    }
    while(nworker > 0){
        if((pid = waitpid()) < 0)
            break;
        for(i=0; i<nelem(worker); i++)
            if(worker[i] == pid){
                worker[i] = -1;
                nworker--;
                break;
            }
    }
}
/*e: function [[getpix]](mothra) */

/*s: function [[countpix]](mothra) */
ulong countpix(void *p){
    ulong n=0;
    Pix *x;
    for(x = p; x; x = x->next)
        n += Dy(x->b->r)*bytesperline(x->b->r, x->b->depth);
    return n;
}
/*e: function [[countpix]](mothra) */

/*s: function [[freepix]](mothra) */
void freepix(void *p){
    Pix *x, *xx;
    xx = p;
    while(x = xx){
        xx = x->next;
        freeimage(x->b);
        free(x);
    }
}
/*e: function [[freepix]](mothra) */
/*e: mothra/getpix.c */
