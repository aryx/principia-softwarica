#include <stdlib.h>
#include <unistd.h>
#include <draw.h>
#include <event.h>


extern	void	_EXITS(char *);

void  eresized(int x) {
    print("eresized %d\n", x);
}

void main(void) {
  
  int res;
  Image *color;

  res = initdraw(nil, nil, "test_draw");
  if (res < 0) {
    print("error in initdraw\n");
    exit(0);
  }

  color = allocimage(display, Rect(0,0,1,1), view->chan, 1, DMagenta);
  draw(view, view->r, color, nil, ZP);

  line(view, Pt(10, 10), Pt(100, 100), Endarrow, Endarrow, 10, display->black, ZP);

  sleep(5);

//  einit(Ekeyboard);
//
//  for(;;) {
//    res = ekbd();
//    print("key =%d\n", res);
//    if(res == 9) {
//      //exit(2);
//      _EXITS("foo");
//    }
//  }

}
