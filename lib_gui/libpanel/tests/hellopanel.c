/*s: lib_gui/libpanel/tests/hellopanel.c */
// source code from panel.pdf introduction
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>

Panel *root;

/*s: function [[done]] */
void done(Panel *p, buttons buttons){
    USED(p, buttons);
    exits(nil);
}
/*e: function [[done]] */

/*s: function [[eresized]] */
void eresized(bool new){
    /*s: [[eresized()]] if [[new]] get a new window */
    if(new && getwindow(display, Refnone) == ERROR_NEG1) {
        fprint(STDERR, "getwindow: %r\n");
        exits("getwindow");
    }
    /*e: [[eresized()]] if [[new]] get a new window */
    plpack(root, view->r);
    pldraw(root, view);
}
/*e: function [[eresized]] */

void main(void){
    errorneg1 err;
    Event e;
    int i;
    
    err = initdraw(nil, nil, "hellopanel");
    /*s: [[main()]] sanity check [[err]] */
    if(err < 0)
        sysfatal("initdraw: %r");
    /*e: [[main()]] sanity check [[err]] */
    einit(Emouse);
    plinit(view->depth);
    
    root=plframe(nil, NOFLAG);
    pllabel(root, NOFLAG, "Hello, world!");
    plbutton(root, NOFLAG, "done", done);
    
    eresized(false);
    
    for(;;) {
        i=event(&e);
        plmouse(root, &e.mouse);
    }
}
/*e: lib_gui/libpanel/tests/hellopanel.c */
