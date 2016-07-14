/*s: windows/rio/threads_window.c */

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

#include <window.h>

#include "dat.h"
#include "fns.h"


/*s: enum Wxxx */
enum { 
    WKey, 
    WMouse, 
    WCtl,
    /*s: [[Wxxx]] cases */
    WMouseread,
    /*x: [[Wxxx]] cases */
    WCread, 
    /*x: [[Wxxx]] cases */
    WCwrite,
    /*x: [[Wxxx]] cases */
    WWread,
    /*e: [[Wxxx]] cases */

    NWALT 
};
/*e: enum Wxxx */

/*s: function deletetimeoutproc */
void
deletetimeoutproc(void *v)
{
    char *s = v;

    sleep(750);	/* remove window from screen after 3/4 of a second */
    sendp(deletechan, s);
}
/*e: function deletetimeoutproc */


/*s: function wctlmesg */
int
wctlmesg(Window *w, int m, Rectangle r, Image *i)
{
    char buf[64];

    switch(m){
    /*s: [[wctlmesg()]] cases */
    case Wakeup:
        break;
    /*x: [[wctlmesg()]] cases */
    case Refresh:
        if(w->deleted || Dx(w->screenr)<=0 || !rectclip(&r, w->i->r))
            break;
        if(!w->mouseopen)
            wrefresh(w, r);
        flushimage(display, true);
        break;
    /*x: [[wctlmesg()]] cases */
    case Deleted:
        /*s: [[wctlmesg()]] break if window was deleted */
        if(w->deleted)
            break;
        /*e: [[wctlmesg()]] break if window was deleted */
        write(w->notefd, "hangup", 6);
        proccreate(deletetimeoutproc, estrdup(w->name), 4096);
        wclosewin(w);
        break;
    /*x: [[wctlmesg()]] cases */
    case Exited:
        frclear(&w->frm, true);
        close(w->notefd);
        chanfree(w->mc.c);
        chanfree(w->ck);
        chanfree(w->cctl);
        chanfree(w->conswrite);
        chanfree(w->consread);
        chanfree(w->mouseread);
        chanfree(w->wctlread);
        free(w->raw);
        free(w->r);
        free(w->dir);
        free(w->label);
        free(w);
        break;
    /*x: [[wctlmesg()]] cases */
    case Moved:
    case Reshaped:
        if(w->deleted){
            freeimage(i);
            break;
        }
        w->screenr = r;
        strcpy(buf, w->name);
        wresize(w, i, m==Moved);
        w->wctlready = true;

        proccreate(deletetimeoutproc, estrdup(buf), 4096);

        if(Dx(r) > 0){
            if(w != input)
                wcurrent(w);
        }else if(w == input)
            wcurrent(nil);
        flushimage(display, true);
        break;
    /*x: [[wctlmesg()]] cases */
    case Movemouse:
        if(sweeping || !ptinrect(r.min, w->i->r))
            break;
        wmovemouse(w, r.min);
        break;
    /*x: [[wctlmesg()]] cases */
    case Rawon:
        // already setup w->rawing in xfidwrite, nothing else todo
        break;
    case Rawoff:
        /*s: [[wctlmesg()]] break if window was deleted */
        if(w->deleted)
            break;
        /*e: [[wctlmesg()]] break if window was deleted */
        /*s: [[wctlmesg()]] When Rawoff, process raw keys in non rawing mode */
        while(w->nraw > 0){
            wkeyctl(w, w->raw[0]);
            --w->nraw;
            runemove(w->raw, w->raw+1, w->nraw);
        }
        /*e: [[wctlmesg()]] When Rawoff, process raw keys in non rawing mode */
        break;
    /*x: [[wctlmesg()]] cases */
    case Holdon:
    case Holdoff:
        /*s: [[wctlmesg()]] break if window was deleted */
        if(w->deleted)
            break;
        /*e: [[wctlmesg()]] break if window was deleted */
        wrepaint(w);
        flushimage(display, true);
        break;
    /*e: [[wctlmesg()]] cases */
    default:
        error("unknown control message");
        break;
    }
    return m;
}
/*e: function wctlmesg */


/*s: function winctl */
void
winctl(void *arg)
{
    Window *w = arg;
    // map<enum<Wxxx>, Alt>
    Alt alts[NWALT+1];
    char buf[128]; // /dev/mouse interface
    /*s: [[winctl()]] other locals */
    Rune *kbdr;
    /*x: [[winctl()]] other locals */
    Wctlmesg wcm;
    /*x: [[winctl()]] other locals */
    Mousestate *mp;
    int lastb = -1;
    /*x: [[winctl()]] other locals */
    Mousereadmesg mrm;
    /*x: [[winctl()]] other locals */
    Mousestate m;
    /*x: [[winctl()]] other locals */
    Consreadmesg crm;
    /*x: [[winctl()]] other locals */
    Stringpair pair;
    char *t;
    int nb;
    int i, c, wid;
    /*x: [[winctl()]] other locals */
    char part[3]; // UTFMAX-1
    int npart = 0;
    /*x: [[winctl()]] other locals */
    Conswritemesg cwm;
    /*x: [[winctl()]] other locals */
    Rune *rp, *bp, *tp, *up;
    int nr;
    int initial;
    uint qh;
    /*x: [[winctl()]] other locals */
    Consreadmesg cwrm;
    /*x: [[winctl()]] other locals */
    char *s;
    /*e: [[winctl()]] other locals */
    
    snprint(buf, sizeof buf, "winctl-id%d", w->id);
    threadsetname(buf);

    /*s: [[winctl()]] channels creation */
    mrm.cm = chancreate(sizeof(Mouse), 0);
    /*x: [[winctl()]] channels creation */
    crm.c1 = chancreate(sizeof(Stringpair), 0);
    crm.c2 = chancreate(sizeof(Stringpair), 0);
    /*x: [[winctl()]] channels creation */
    cwm.cw = chancreate(sizeof(Stringpair), 0);
    /*x: [[winctl()]] channels creation */
    cwrm.c1 = chancreate(sizeof(Stringpair), 0);
    cwrm.c2 = chancreate(sizeof(Stringpair), 0);
    /*e: [[winctl()]] channels creation */

    /*s: [[winctl()]] alts setup */
    alts[WKey].c = w->ck;
    alts[WKey].v = &kbdr;
    alts[WKey].op = CHANRCV;
    /*x: [[winctl()]] alts setup */
    alts[WMouse].c = w->mc.c;
    alts[WMouse].v = &w->mc.Mouse;
    alts[WMouse].op = CHANRCV;
    /*x: [[winctl()]] alts setup */
    alts[WCtl].c = w->cctl;
    alts[WCtl].v = &wcm;
    alts[WCtl].op = CHANRCV;
    /*x: [[winctl()]] alts setup */
    alts[WMouseread].c = w->mouseread;
    alts[WMouseread].v = &mrm;
    alts[WMouseread].op = CHANSND;
    /*x: [[winctl()]] alts setup */
    alts[WCread].c = w->consread;
    alts[WCread].v = &crm;
    alts[WCread].op = CHANSND;
    /*x: [[winctl()]] alts setup */
    alts[WCwrite].c = w->conswrite;
    alts[WCwrite].v = &cwm;
    alts[WCwrite].op = CHANSND;
    /*x: [[winctl()]] alts setup */
    alts[WWread].c = w->wctlread;
    alts[WWread].v = &cwrm;
    alts[WWread].op = CHANSND;
    /*e: [[winctl()]] alts setup */
    alts[NWALT].op = CHANEND;

    for(;;){
        /*s: [[winctl()]] alts adjustments */
        if(w->mouseopen && w->mouse.counter != w->mouse.lastcounter)
            alts[WMouseread].op = CHANSND;
        else
            alts[WMouseread].op = CHANNOP;
        /*x: [[winctl()]] alts adjustments */
        /*s: [[winctl()]] alts adjustments, if holding */
        if(w->holding)
            alts[WCread].op = CHANNOP;
        /*e: [[winctl()]] alts adjustments, if holding */
        else if((w->rawing && w->nraw>0) || npart)
            alts[WCread].op = CHANSND;
        else{
            alts[WCread].op = CHANNOP;
            /*s: [[winctl()]] alts adjustments, revert to CHANSND if newline in queue */
            /* this code depends on NL and EOT fitting in a single byte */
            /* kind of expensive for each loop; worth precomputing? */
            for(i = w->qh; i < w->nr; i++){
                 c = w->r[i];
                 // buffering, until get a newline in which case we are ready to send
                 if(c=='\n' || c=='\004'){
                     alts[WCread].op = CHANSND;
                     break;
                 }
             }
            /*e: [[winctl()]] alts adjustments, revert to CHANSND if newline in queue */
        }
        /*x: [[winctl()]] alts adjustments */
        if(!w->scrolling && !w->mouseopen && w->qh >  w->org + w->frm.nchars)
            alts[WCwrite].op = CHANNOP;
        else
            alts[WCwrite].op = CHANSND;
        /*x: [[winctl()]] alts adjustments */
        if(w->deleted || !w->wctlready)
            alts[WWread].op = CHANNOP;
        else
            alts[WWread].op = CHANSND;
        /*e: [[winctl()]] alts adjustments */

        // event loop
        switch(alt(alts)){
        /*s: [[winctl()]] event loop cases */
        case WKey:
            for(i=0; kbdr[i]!=L'\0'; i++)
                wkeyctl(w, kbdr[i]);
            break;
        /*x: [[winctl()]] event loop cases */
        case WMouse:
            /*s: [[winctl()]] WMouse case if mouseopen */
            if(w->mouseopen) {
                w->mouse.counter++;

                /* queue click events */
                if(!w->mouse.qfull && lastb != w->mc.buttons) {	/* add to ring */

                    //insert_queue(w->mc, w->mouse.queue)
                    mp = &w->mouse.queue[w->mouse.wi];
                    if(++w->mouse.wi == nelem(w->mouse.queue))
                        w->mouse.wi = 0;
                    if(w->mouse.wi == w->mouse.ri)
                        w->mouse.qfull = true;
                    mp->Mouse = w->mc;
                    mp->counter = w->mouse.counter;

                    lastb = w->mc.buttons;
                }
            }
            /*e: [[winctl()]] WMouse case if mouseopen */
            else
            /*s: [[winctl()]] WMouse case if not mouseopen */
            wmousectl(w);
            /*e: [[winctl()]] WMouse case if not mouseopen */
            break;
        /*x: [[winctl()]] event loop cases */
        case WCtl:
            if(wctlmesg(w, wcm.type, wcm.r, wcm.image) == Exited){
                /*s: [[winctl()]] Wctl case, free channels if wctlmesg is Excited */
                chanfree(mrm.cm);
                /*x: [[winctl()]] Wctl case, free channels if wctlmesg is Excited */
                chanfree(crm.c1);
                chanfree(crm.c2);
                /*x: [[winctl()]] Wctl case, free channels if wctlmesg is Excited */
                chanfree(cwm.cw);
                /*x: [[winctl()]] Wctl case, free channels if wctlmesg is Excited */
                chanfree(cwrm.c1);
                chanfree(cwrm.c2);
                /*e: [[winctl()]] Wctl case, free channels if wctlmesg is Excited */
                threadexits(nil);
            }
            continue;
        /*x: [[winctl()]] event loop cases */
        case WMouseread:
            /* send a queued event or, if the queue is empty, the current state */
            /* if the queue has filled, we discard all the events it contained. */
            /* the intent is to discard frantic clicking by the user during long latencies. */
            w->mouse.qfull = false;
            // if produced more than read
            if(w->mouse.wi != w->mouse.ri) {
                m = w->mouse.queue[w->mouse.ri];
                if(++w->mouse.ri == nelem(w->mouse.queue))
                    w->mouse.ri = 0;
            } else
                m = (Mousestate){w->mc.Mouse, w->mouse.counter};

            w->mouse.lastcounter = m.counter;
            // consumed and relay
            send(mrm.cm, &m.Mouse);
            continue;
        /*x: [[winctl()]] event loop cases */
        case WCread:
            recv(crm.c1, &pair);
            t = pair.s;
            nb = pair.ns;

            i = 0;
            /*s: [[winctl()]] when WCRead, copy in t previous partial rune bytes */
            i = npart;
            npart = 0;
            if(i)
                memmove(t, part, i);
            /*e: [[winctl()]] when WCRead, copy in t previous partial rune bytes */

            while(i<nb && (w->nraw > 0 || w->qh < w->nr)){

                if(w->qh == w->nr){
                    wid = runetochar(t+i, &w->raw[0]);
                    w->nraw--;
                    runemove(w->raw, w->raw+1, w->nraw);
                }else
                    wid = runetochar(t+i, &w->r[w->qh++]);

                i += wid;
                /*s: [[winctl()]] when WCRead, break if newline and handle EOF character */
                c = t[i-wid];	/* knows break characters fit in a byte */
                if(!w->rawing && (c == '\n' || c=='\004')){
                    if(c == '\004')
                        i--;
                    break;
                /*e: [[winctl()]] when WCRead, break if newline and handle EOF character */
                }
            }
            /*s: [[winctl()]] when WCRead, handle EOF character after while loop */
            if(i==nb && w->qh < w->nr && w->r[w->qh]=='\004')
                w->qh++;
            /*e: [[winctl()]] when WCRead, handle EOF character after while loop */
            /*s: [[winctl()]] when WCRead, store overflow bytes of partial rune */
            if(i > nb){
                npart = i-nb;
                memmove(part, t+nb, npart);
                i = nb;
            }
            /*e: [[winctl()]] when WCRead, store overflow bytes of partial rune */

            pair.s = t;
            pair.ns = i;
            send(crm.c2, &pair);
            continue;
        /*x: [[winctl()]] event loop cases */
        case WCwrite:
            recv(cwm.cw, &pair);
            rp = pair.s;
            nr = pair.ns;

            bp = rp;
            for(i=0; i<nr; i++) {
                if(*bp++ == '\b'){
                    --bp;
                    initial = 0;
                    tp = runemalloc(nr);
                    runemove(tp, rp, i);
                    up = tp+i;
                    for(; i<nr; i++){
                        *up = *bp++;
                        if(*up == '\b')
                            if(up == tp)
                                initial++;
                            else
                                --up;
                        else
                            up++;
                    }
                    if(initial){
                        if(initial > w->qh)
                            initial = w->qh;
                        qh = w->qh-initial;
                        wdelete(w, qh, qh+initial);
                        w->qh = qh;
                    }
                    free(rp);
                    rp = tp;
                    nr = up-tp;
                    rp[nr] = 0;
                    break;
                }
            }

            w->qh = winsert(w, rp, nr, w->qh) + nr;
            if(w->scrolling || w->mouseopen)
                wshow(w, w->qh);
            wsetselect(w, w->q0, w->q1);
            wscrdraw(w);
            free(rp);
            break;
        /*x: [[winctl()]] event loop cases */
        case WWread:
            w->wctlready = false;
            recv(cwrm.c1, &pair);
            if(w->deleted || w->i==nil)
                pair.ns = sprint(pair.s, "");
            else{
                s = "visible";
                for(i=0; i<nhidden; i++)
                    if(hidden[i] == w){
                        s = "hidden";
                        break;
                    }
                t = "notcurrent";
                if(w == input)
                    t = "current";
                pair.ns = snprint(pair.s, pair.ns, "%11d %11d %11d %11d %s %s ",
                    w->i->r.min.x, w->i->r.min.y, w->i->r.max.x, w->i->r.max.y, t, s);
            }
            send(cwrm.c2, &pair);
            continue;
        /*e: [[winctl()]] event loop cases */
        }

        if(!w->deleted)
            flushimage(display, true);
    }
}
/*e: function winctl */


/*e: windows/rio/threads_window.c */
