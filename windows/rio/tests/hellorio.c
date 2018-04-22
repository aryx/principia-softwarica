/*s: tests/rio/hellorio.c */
#include <u.h>
#include <libc.h>

#include <draw.h>
#include <mouse.h>
#include <keyboard.h>

#include <thread.h>

Image *bgcolor;
Point mouseloc;
Rune str[20];

/*s: type EventType (hellorio.c) */
enum EventType {
  EMouse,
  EKey,
  EResize,

  NALT
};
/*e: type EventType (hellorio.c) */

void redraw(void);

void threadmain(int argc, char* argv[]) {
  int result;
  Keyboardctl* keyboardctl;
  Mousectl*    mousectl;
  /*s: [[threadmain()]] other locals (hellorio.c) */
  // map<EventType, Alt>
  Alt alts[NALT+1];
  /*x: [[threadmain()]] other locals (hellorio.c) */
  Rune keys[20];
  /*x: [[threadmain()]] other locals (hellorio.c) */
  Mouse mouse;
  /*x: [[threadmain()]] other locals (hellorio.c) */
  int i;
  /*e: [[threadmain()]] other locals (hellorio.c) */

  result = initdraw(nil, nil, "Hello Rio");
  /*s: [[threadmain()]] sanity check result (hellorio.c) */
  if (result < 0)
    exits("Error in initdraw");
  /*e: [[threadmain()]] sanity check result (hellorio.c) */
  mousectl = initmouse(nil, view);
  /*s: [[threadmain()]] sanity check mousectl (hellorio.c) */
  if(mousectl == nil)
    exits("can't find mouse");
  /*e: [[threadmain()]] sanity check mousectl (hellorio.c) */
  keyboardctl = initkeyboard(nil);
  /*s: [[threadmain()]] sanity check keyboardctl (hellorio.c) */
  if(keyboardctl == nil)
    exits("can't find keyboard");
  /*e: [[threadmain()]] sanity check keyboardctl (hellorio.c) */

  bgcolor = allocimage(display, Rect(0,0,1,1), RGBA32, true, DMagenta);
  runestrcpy(str, L"Hello Rio");
  mouseloc = Pt(200, 200);

  /*s: [[threadmain()]] alts setup (hellorio.c) */
  alts[EMouse].c = mousectl->c;
  alts[EMouse].v = &mouse;
  alts[EMouse].op = CHANRCV;

  alts[EKey].c = keyboardctl->c;
  alts[EKey].v = keys;
  alts[EKey].op = CHANRCV;

  alts[EResize].c = mousectl->resizec;
  alts[EResize].v = nil;
  alts[EResize].op = CHANRCV;
  /*x: [[threadmain()]] alts setup (hellorio.c) */
  alts[NALT].op = CHANEND;
  /*e: [[threadmain()]] alts setup (hellorio.c) */
  redraw();
  /*s: [[threadmain()]] event loop (hellorio.c) */
  for(;;) {
    switch(alt(alts)){
    /*s: [[threadmain()]] event loop cases (hellorio.c) */
    case EMouse:
      mouseloc = mouse.xy;
      break;
    /*x: [[threadmain()]] event loop cases (hellorio.c) */
    case EKey:
        for(i=1; i<nelem(keys)-1; i++)
            if(nbrecv(keyboardctl->c, keys+i) <= 0)
                break;
        keys[i] = L'\0';
        runestrcpy(str, keys);
        if(keys[0] == L'q')
          exits("done");
        break;
    /*x: [[threadmain()]] event loop cases (hellorio.c) */
    case EResize:
      if(getwindow(display, Refnone) < 0)
        exits("failed to re-attach window");
      break;
    /*e: [[threadmain()]] event loop cases (hellorio.c) */
    }
    redraw();
  }
  /*e: [[threadmain()]] event loop (hellorio.c) */
}

void redraw(void) 
{
  draw(view, view->r, bgcolor, nil, ZP);
  runestring(view, mouseloc, display->black, ZP, font, str);
  flushimage(display, true);
}
/*e: tests/rio/hellorio.c */
