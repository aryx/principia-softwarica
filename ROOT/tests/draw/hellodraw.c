#include <u.h>
#include <libc.h>
#include <draw.h>

//#include <event.h>

//void  eresized(int x) {
//    print("eresized %d\n", x);
//}
//extern	void	_exits(char *);

void main(void) {
  
  int res;
  Image *color;

  res = initdraw(nil, nil, "Hello Draw");
  if (res < 0) {
    exits("Error in initdraw\n");
  }

  color = allocimage(display, Rect(0,0,1,1), RGBA32, true, DMagenta);
  draw(display->image, display->image->r, color, nil, ZP);

  line(display->image, 
       Pt(10, 10), 
       Pt(100, 100), Endsquare, Endsquare, 10, display->black, ZP);
  string(display->image, 
         Pt(200, 200), display->black, ZP, font, 
         "Hello Graphical World");
  flushimage(display, true);

  sleep(5000); // msec

  // oops ... can overflow on desktop and other windows when run under rio
  //line(display->image, Pt(100, 100), Pt(700, 700), 0, 0, 3, display->black, ZP);

  //einit(Ekeyboard);

  //for(;;) {
    //res = ekbd();
    //print("key =%d\n", res);
    //
    //if(res == 9 || res == 10) {
    //  //_exits("done");
    //  exits("done");
    //}
  //}
}
