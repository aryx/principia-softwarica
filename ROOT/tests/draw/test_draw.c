#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

void  eresized(int x) {
    print("eresized %d\n", x);
}

extern	void	_exits(char *);

void main(void) {
  
  int res;
  Image *color;

  res = initdraw(nil, nil, "test_draw");
  if (res < 0) {
    print("error in initdraw\n");
    exits("done");
  }

  color = allocimage(display, Rect(0,0,1,1), view->chan, true, DMagenta);
  draw(view, view->r, color, nil, ZP);

  line(view, Pt(10, 10), Pt(100, 100), Endarrow, Endarrow, 10, display->black, ZP);
  string(view, Pt(200, 200), display->black, ZP, font, "this is a test");

  // oops ... can overflow on desktop and other windows when run under rio
  line(display->image, Pt(100, 100), Pt(700, 700), 0, 0, 3, display->black, ZP);

  flushimage(display, true);

  //sleep(5000); // msec
  einit(Ekeyboard);

  for(;;) {
    res = ekbd();
    print("key =%d\n", res);

    if(res == 9 || res == 10) {
      //_exits("done");
      exits("done");
    }
  }
}
