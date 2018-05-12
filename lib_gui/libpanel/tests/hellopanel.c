#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>

// source code from panel.pdf introduction

Panel *root;

void done(Panel *p, int buttons){
  USED(p, buttons);
  exits(0);
}


void ereshaped(Rectangle r){
  view->r=r;
  plpack(root, r);
  //draw(view, r.min, view, r, ZP);
  pldraw(root, view);
}


void main(void){
  if(initdraw(0, 0, "hellopanel") < 0)
    sysfatal("initdraw: %r");
  einit(Emouse);
  plinit(view->depth);
  root=plframe(0, 0);
  pllabel(root, 0, "Hello, world!");
  plbutton(root, 0, "done", done);
  ereshaped(view->r);
  for(;;) 
    plmouse(root, emouse);
}


