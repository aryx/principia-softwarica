#include <u.h>
#include <libc.h>

#include <draw.h>

extern void drawsetdebug(bool);

void main(void) 
{
  int result;

  result = initdraw(nil, nil, "Hello Draw");
  if (result < 0) {
    exits("Error in initdraw");
  }

  drawsetdebug(true);
  flushimage(display, true);
  flushimage(display, true);
  flushimage(display, true);

  color = allocimage(display, Rect(0,0,1,1), RGBA32, true, DMagenta);
  border(view, Rect(10, 10, 100, 100), 1, color, ZP);
  string(view, Pt(10, 10), display->black, ZP, font, "Hello World");


  flushimage(display, true);
  while(true) { }
}
