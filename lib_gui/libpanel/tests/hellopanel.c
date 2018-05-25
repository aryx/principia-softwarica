/*s: lib_gui/libpanel/tests/hellopanel.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>

// source code from panel.pdf introduction

Panel *root;

void done(Panel *p, int buttons){
  USED(p, buttons);
  exits(nil);
}

void eresized(bool new){
 if(new && getwindow(display, Refnone) == -1) {
  fprint(STDERR, "getwindow: %r\n");
  exits("getwindow");
 }
    plpack(root, view->r);
    pldraw(root, view);
 flushimage(display, true);
}


void main(void){
  int i;
  Event e;

  if(initdraw(0, 0, "hellopanel") < 0)
    sysfatal("initdraw: %r");
  einit(Emouse);
  plinit(view->depth);

  root=plframe(0, 0);
  pllabel(root, 0, "Hello, world!");
  plbutton(root, 0, "done", done);

  eresized(false);

  for(;;) {
    i=event(&e);
    plmouse(root, &e.mouse);
  }
}
/*e: lib_gui/libpanel/tests/hellopanel.c */
