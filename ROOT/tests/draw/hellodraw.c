#include <u.h>
#include <libc.h>

#include <draw.h>

void main(void) 
{
  int result;
  Image *color;

  result = initdraw(nil, nil, "Hello Draw");
  if (result < 0) {
    exits("Error in initdraw");
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
  //line(display->image, Pt(100,100), Pt(700,700), 0, 0, 3, display->black, ZP);
}
