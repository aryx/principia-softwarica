#include <u.h>
#include <libc.h>
#include <draw.h>

void main(void) {
  
  int res;
  Image *color;

  res = initdraw(nil, nil, "test_draw");
  if (res < 0) {
    print("error in initdraw\n");
    exits("done");
  }

  color = allocimage(display, Rect(0,0,1,1), view->chan, 1, DMagenta);
  draw(view, view->r, color, nil, ZP);

  line(view, Pt(10, 10), Pt(100, 100), Endarrow, Endarrow, 10, display->black, ZP);
  string(view, Pt(200, 200), display->black, ZP, font, "this is a test");

  flushimage(display, 1);
  sleep(5000); // msec

}
