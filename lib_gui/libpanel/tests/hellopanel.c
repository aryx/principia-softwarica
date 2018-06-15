/*s: lib_gui/libpanel/tests/hellopanel.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>

// source code from panel.pdf introduction

Panel *root;

/*s: function [[done]] */
void done(Panel *p, int buttons){
    USED(p, buttons);
    exits(nil);
}
/*e: function [[done]] */

/*s: function [[eresized]] */
void eresized(bool new){
    if(new && getwindow(display, Refnone) == ERROR_NEG1) {
        fprint(STDERR, "getwindow: %r\n");
        exits("getwindow");
    }
    plpack(root, view->r);
    pldraw(root, view);
    flushimage(display, true);
}
/*e: function [[eresized]] */

void main(void){
    int i;
    Event e;
    errorneg1 err;
    
    err = initdraw(nil, nil, "hellopanel");
    /*s: [[main()]] sanity check [[err]] */
    if(err < 0)
        sysfatal("initdraw: %r");
    /*e: [[main()]] sanity check [[err]] */
    einit(Emouse);
    plinit(view->depth);
    
    root=plframe(nil, 0);
    pllabel(root, 0, "Hello, world!");
    plbutton(root, 0, "done", done);
    
    eresized(false);
    
    for(;;) {
        i=event(&e);
        plmouse(root, &e.mouse);
    }
}
/*e: lib_gui/libpanel/tests/hellopanel.c */
