/*s: windows/rio/globals.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <cursor.h>
#include <mouse.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>

#include "dat.h"
#include "fns.h"

/*s: global font */
Font		*font;
/*e: global font */
/*s: global mousectl */
Mousectl	*mousectl;
/*e: global mousectl */
/*s: global mouse */
Mouse	*mouse;
/*e: global mouse */
/*s: global keyboardctl */
Keyboardctl	*keyboardctl;
/*e: global keyboardctl */
/*s: global display */
Display	*display;
/*e: global display */
/*s: global view */
Image	*view;
/*e: global view */
/*s: global wscreen */
Screen	*wscreen;
/*e: global wscreen */
/*s: global background */
Image	*background;
/*e: global background */
/*s: global lightgrey */
Image	*lightgrey;
/*e: global lightgrey */
/*s: global red */
Image	*red;
/*e: global red */
/*s: global window */
Window	**window;
/*e: global window */
/*s: global wkeyboard */
Window	*wkeyboard;	/* window of simulated keyboard */
/*e: global wkeyboard */
/*s: global nwindow */
int		nwindow;
/*e: global nwindow */
/*s: global snarffd */
int		snarffd;
/*e: global snarffd */
/*s: global input */
Window	*input;
/*e: global input */
/*s: global all */
QLock	all;			/* BUG */
/*e: global all */
/*s: global filsys */
Filsys	*filsys;
/*e: global filsys */
/*s: global hidden */
Window	*hidden[100];
/*e: global hidden */
/*s: global nhidden */
int		nhidden;
/*e: global nhidden */
/*s: global nsnarf */
int		nsnarf;
/*e: global nsnarf */
/*s: global snarf */
Rune*	snarf;
/*e: global snarf */
/*s: global scrolling */
int		scrolling;
/*e: global scrolling */
/*s: global maxtab */
int		maxtab;
/*e: global maxtab */
/*s: global deletechan */
Channel*	deletechan;
/*e: global deletechan */
/*s: global startdir */
char		*startdir;
/*e: global startdir */
/*s: global sweeping */
int		sweeping;
/*e: global sweeping */
/*s: global wctlfd */
int		wctlfd;
/*e: global wctlfd */
/*s: global menuing */
int		menuing;		/* menu action is pending; waiting for window to be indicated */
/*e: global menuing */
/*s: global snarfversion */
int		snarfversion;	/* updated each time it is written */
/*e: global snarfversion */

/*e: windows/rio/globals.c */
