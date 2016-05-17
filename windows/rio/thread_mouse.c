/*s: windows/rio/thread_mouse.c */
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

/*s: enum Mxxx */
enum {
    MMouse,
    /*s: [[Mxxx]] cases */
    MReshape,
    /*e: [[Mxxx]] cases */
    NALT
};
/*e: enum Mxxx */


/*s: function keyboardhide */
/*
 * Button 6 - keyboard toggle - has been pressed.
 * Send event to keyboard, wait for button up, send that.
 * Note: there is no coordinate translation done here; this
 * is just about getting button 6 to the keyboard simulator.
 */
void
keyboardhide(void)
{
    send(wkeyboard->mc.c, mouse);
    do
        readmouse(mousectl);
    while(mouse->buttons & (1<<5));
    send(wkeyboard->mc.c, mouse);
}
/*e: function keyboardhide */

/*s: global viewr */
Rectangle	viewr;
/*e: global viewr */

/*s: function resized */
void
resized(void)
{
    Image *im;
    int i, j;
    bool ishidden;
    Rectangle r;
    Point o, n;
    Window *w;

    // updates view (and screen)
    if(getwindow(display, Refnone) < 0)
        error("failed to re-attach window");

    freescrtemps();
    freescreen(wscreen);

    wscreen = allocscreen(view, background, false);
    /*s: [[resized()]] sanity check wscreen */
    if(wscreen == nil)
        error("can't re-allocate screen");
    /*e: [[resized()]] sanity check wscreen */
    draw(view, view->r, background, nil, ZP);

    // old view rectangle
    o = subpt(viewr.max, viewr.min);
    n = subpt(view->clipr.max, view->clipr.min);

    for(i=0; i<nwindow; i++){
        w = windows[i];
        /*s: [[resized()]] continue if window was deleted */
        if(w->deleted)
            continue;
        /*e: [[resized()]] continue if window was deleted */
        r = rectsubpt(w->i->r, viewr.min);
        r.min.x = (r.min.x*n.x)/o.x;
        r.min.y = (r.min.y*n.y)/o.y;
        r.max.x = (r.max.x*n.x)/o.x;
        r.max.y = (r.max.y*n.y)/o.y;
        r = rectaddpt(r, view->clipr.min);

        ishidden = false;
        for(j=0; j<nhidden; j++)
            if(w == hidden[j]){
                ishidden = true;
                break;
            }
        if(ishidden){
            im = allocimage(display, r, view->chan, false, DWhite);
            r = ZR;
        }else
            im = allocwindow(wscreen, r, Refbackup, DWhite);

        if(im)
            wsendctlmesg(w, Reshaped, r, im);
    }
    viewr = view->r;
    flushimage(display, true);
}
/*e: function resized */


/*s: function mousethread */
void
mousethread(void*)
{
    /*s: [[mousethread()]] locals */
    // map<enum<Mxxx>, Alt>
    static Alt alts[NALT+1];
    /*x: [[mousethread()]] locals */
    Window *winput;
    Point xy; // logical coord
    /*x: [[mousethread()]] locals */
    bool sending = false;
    /*x: [[mousethread()]] locals */
    Mouse tmp;
    /*x: [[mousethread()]] locals */
    Window *w;
    /*x: [[mousethread()]] locals */
    bool moving = false;
    /*x: [[mousethread()]] locals */
    bool inside, band;
    Window *oin;
    Image *i;
    Rectangle r;
    /*x: [[mousethread()]] locals */
    bool scrolling = false;
    /*e: [[mousethread()]] locals */

    threadsetname("mousethread");

    /*s: [[mousethread()]] alts setup */
    // listen
    alts[MMouse].c = mousectl->c;
    alts[MMouse].v = &mousectl->Mouse;
    alts[MMouse].op = CHANRCV;
    /*x: [[mousethread()]] alts setup */
    alts[MReshape].c = mousectl->resizec;
    alts[MReshape].v = nil;
    alts[MReshape].op = CHANRCV;
    /*e: [[mousethread()]] alts setup */
    alts[NALT].op = CHANEND;

    for(;;)
        // message loop
        switch(alt(alts)){
        /*s: [[mousethread()]] event loop cases */
        case MMouse:
            /*s: [[mousethread()]] if wkeyboard and button 6 */
            if(wkeyboard!=nil && (mouse->buttons & (1<<5))){
                keyboardhide();
                break;
            }
            /*e: [[mousethread()]] if wkeyboard and button 6 */
        Again:
            winput = input;
            /*s: [[mousethread()]] if wkeyboard and ptinrect */
            /* override everything for the keyboard window */
            if(wkeyboard!=nil && ptinrect(mouse->xy, wkeyboard->screenr)){
                /* make sure it's on top; this call is free if it is */
                wtopme(wkeyboard);
                winput = wkeyboard;
            }
            /*e: [[mousethread()]] if wkeyboard and ptinrect */

            if(winput != nil && winput->i != nil){
                /* convert to logical coordinates */
                xy.x = mouse->xy.x + (winput->i->r.min.x - winput->screenr.min.x);
                xy.y = mouse->xy.y + (winput->i->r.min.y - winput->screenr.min.y);

                /*s: [[mousethread()]] goto Sending if scroll buttons */
                /* the up and down scroll buttons are not subject to the usual rules */
                if((mouse->buttons&(8|16)) && !winput->mouseopen)
                    goto Sending;
                /*e: [[mousethread()]] goto Sending if scroll buttons */

                inside = ptinrect(mouse->xy, insetrect(winput->screenr, Selborder));

                /*s: [[mousethread()]] set scrolling */
                if(winput->mouseopen)
                    scrolling = false;
                else 
                  if(scrolling)
                    scrolling = mouse->buttons;
                  else
                    scrolling = mouse->buttons && ptinrect(xy, winput->scrollr);
                /*e: [[mousethread()]] set scrolling */
                /*s: [[mousethread()]] set moving to true for some conditions */
                /* topped will be zero or less if window has been bottomed */
                if(sending == false && !scrolling 
                   && winborder(winput, mouse->xy) && winput->topped > 0){
                    moving = true;
                }
                /*e: [[mousethread()]] set moving to true for some conditions */
                else 
                   /*s: [[mousethread()]] set sending to true for some conditions */
                   if(inside && 
                      ((mouse->buttons&1) || winput->mouseopen || scrolling))
                        sending = true;
                   /*e: [[mousethread()]] set sending to true for some conditions */
            }else
                sending = false;

            /*s: [[mousethread()]] if sending */
            if(sending){
            Sending:
                /*s: [[mousethread()]] when sending mouse message to window, set the cursor */
                if(mouse->buttons == 0){
                    // cornercursor will call wsetcursor if cursor not on the border
                    cornercursor(winput, mouse->xy, false);
                    sending = false;
                }else
                    wsetcursor(winput, false);
                /*e: [[mousethread()]] when sending mouse message to window, set the cursor */

                tmp = mousectl->Mouse;
                tmp.xy = xy; // logical coordinates

                // Dispatch, to current window thread!
                send(winput->mc.c, &tmp);
                continue;
            }
            /*e: [[mousethread()]] if sending */
            /*s: [[mousethread()]] if not sending */
            w = wpointto(mouse->xy);

            /* change cursor if over anyone's border */
            if(w != nil)
                cornercursor(w, mouse->xy, false);
            else
                riosetcursor(nil, false);

            /*s: [[mousethread()]] if moving and buttons */
            if(moving && (mouse->buttons&7)){
                oin = winput;
                band = mouse->buttons & 3; // left or middle click

                sweeping = true;
                if(band)
                    i = bandsize(winput);
                else
                    i = drag(winput, &r);
                sweeping = false;

                if(i != nil){
                    if(winput == oin){
                        if(band)
                            wsendctlmesg(winput, Reshaped, i->r, i);
                        else
                            wsendctlmesg(winput, Moved, r, i);
                        cornercursor(winput, mouse->xy, true);
                    }else
                        freeimage(i);
                }
            }
            /*e: [[mousethread()]] if moving and buttons */

            if(w != nil)
                cornercursor(w, mouse->xy, false);

            /*s: [[mousethread()]] if buttons and was not sending */
            /* we're not sending the event, but if button is down maybe we should */
            if(mouse->buttons){
                /* w->topped will be zero or less if window has been bottomed */
                if(w==nil || (w==winput && w->topped > 0)){
                    if(mouse->buttons & 1){
                        ;
                    }else if(mouse->buttons & 2){
                        if(winput && !winput->mouseopen)
                            /*s: [[mousethread()]] middle click under certain conditions */
                            button2menu(winput);
                            /*e: [[mousethread()]] middle click under certain conditions */
                    }else if(mouse->buttons & 4)
                            /*s: [[mousethread()]] right click under certain conditions */
                            button3menu();
                            /*e: [[mousethread()]] right click under certain conditions */
                }else{
                    /* if button 1 event in the window, top the window and wait for button up. */
                    /* otherwise, top the window and pass the event on */
                    /*s: [[mousethread()]] click on unfocused window, set w */
                    w = wtop(mouse->xy);
                    /*e: [[mousethread()]] click on unfocused window, set w */
                    if(w && (mouse->buttons!=1 || winborder(w, mouse->xy)))
                        // input changed
                        goto Again;

                    goto Drain;
                }
            }
            /*e: [[mousethread()]] if buttons and was not sending */
            moving = false;
            break;
            /*e: [[mousethread()]] if not sending */

        /*s: [[mousethread()]] Drain label */
        Drain:
            do {
                readmouse(mousectl);
            } while(mousectl->buttons);
            moving = false;
            goto Again;	/* recalculate mouse position, cursor */
        /*e: [[mousethread()]] Drain label */
        /*x: [[mousethread()]] event loop cases */
        case MReshape:
            resized();
            break;
        /*e: [[mousethread()]] event loop cases */
        }
}
/*e: function mousethread */

/*e: windows/rio/thread_mouse.c */
