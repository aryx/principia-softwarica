#include <stdlib.h>
#include <unistd.h>
#include <draw.h>
#include <caml/mlvalues.h>

void caml_draw_initdraw(value x, value y) {
  USED(x); USED(y);
  initdraw(nil, nil, "test_draw");
}

Image *current_color = NULL;

void caml_draw_set_color(value red, value green, value blue, value alpha) {

  int rgba = (Int_val(red)<<3*8) | 
             (Int_val(green)<<2*8) | 
             (Int_val(blue)<<1*8) | 
             (Int_val(alpha)<<0*8);
  current_color = allocimage(display, Rect(0,0,1,1), view->chan, 1, rgba);

}

void caml_draw_line(value x1, value y1, value x2, value y2) {

  line(view, Pt(Int_val(x1), Int_val(y1)), 
             Pt(Int_val(x2), Int_val(y2)), 
       0, 0, 0, current_color, ZP);
  flushimage(display, 1);
}

void caml_draw_string(value x, value y, value str) {
  string(view, Pt(Int_val(x), Int_val(y)), current_color, ZP, font,
         String_val(str));
  flushimage(display, 1);
}
