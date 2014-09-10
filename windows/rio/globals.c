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

/*s: global font (windows/rio/dat.h) */
Font		*font;
/*e: global font (windows/rio/dat.h) */
/*s: global mousectl */
Mousectl	*mousectl;
/*e: global mousectl */
/*s: global mouse */
Mouse	*mouse;
/*e: global mouse */
/*s: global keyboardctl */
Keyboardctl	*keyboardctl;
/*e: global keyboardctl */
/*s: global display (windows/rio/dat.h) */
Display	*display;
/*e: global display (windows/rio/dat.h) */
/*s: global view */
Image	*view;
/*e: global view */
/*s: global wscreen */
Screen	*wscreen;
/*e: global wscreen */
/*s: global boxcursor */
Cursor	boxcursor;
/*e: global boxcursor */
/*s: global crosscursor */
Cursor	crosscursor;
/*e: global crosscursor */
/*s: global sightcursor */
Cursor	sightcursor;
/*e: global sightcursor */
/*s: global whitearrow */
Cursor	whitearrow;
/*e: global whitearrow */
/*s: global query */
Cursor	query;
/*e: global query */
/*s: global corners (windows/rio/dat.h) */
Cursor	*corners[9];
/*e: global corners (windows/rio/dat.h) */
/*s: global background */
Image	*background;
/*e: global background */
/*s: global lightgrey */
Image	*lightgrey;
/*e: global lightgrey */
/*s: global red (windows/rio/dat.h) */
Image	*red;
/*e: global red (windows/rio/dat.h) */
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
/*s: global winclosechan */
Channel*	winclosechan;
/*e: global winclosechan */
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
/*s: global srvpipe */
char		srvpipe[];
/*e: global srvpipe */
/*s: global srvwctl */
char		srvwctl[];
/*e: global srvwctl */
/*s: global errorshouldabort */
int		errorshouldabort;
/*e: global errorshouldabort */
/*s: global menuing */
int		menuing;		/* menu action is pending; waiting for window to be indicated */
/*e: global menuing */
/*s: global snarfversion */
int		snarfversion;	/* updated each time it is written */
/*e: global snarfversion */
/*s: global messagesize */
int		messagesize;		/* negotiated in 9P version setup */
/*e: global messagesize */

/*e: windows/rio/globals.c */
