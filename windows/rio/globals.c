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

/*s: global mousectl */
Mousectl	*mousectl;
/*e: global mousectl */
/*s: global mouse */
// alias for &mousectl->Mouse
Mouse	*mouse;
/*e: global mouse */
/*s: global keyboardctl */
Keyboardctl	*keyboardctl;
/*e: global keyboardctl */

/*s: global wscreen */
Screen	*wscreen;
/*e: global wscreen */
/*s: global background */
Image	*background;
/*e: global background */
/*s: global red */
Image	*red;
/*e: global red */

/*s: global window */
// growing_array<option<ref_own<Window>>> (size = nwindow+1)
Window	**windows;
/*e: global window */
/*s: global wkeyboard */
Window	*wkeyboard;	/* window of simulated keyboard */
/*e: global wkeyboard */
/*s: global nwindow */
int	nwindow;
/*e: global nwindow */

/*s: global snarffd */
fdt		snarffd;
/*e: global snarffd */
/*s: global input */
//option<ref<Window>>, the window with the focus! the window to send input to
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
bool		scrolling;
/*e: global scrolling */
/*s: global maxtab */
int		maxtab = 0;
/*e: global maxtab */
/*s: global deletechan */
// chan<string> (listener = deletethread, sender = deletetimeoutproc)
Channel* deletechan;
/*e: global deletechan */
/*s: global startdir */
char		*startdir;
/*e: global startdir */
/*s: global sweeping */
bool	sweeping;
/*e: global sweeping */
/*s: global wctlfd */
int	wctlfd;
/*e: global wctlfd */
/*s: global menuing */
bool menuing;/* menu action is pending; waiting for window to be indicated */
/*e: global menuing */
/*s: global snarfversion */
int		snarfversion;	/* updated each time it is written */
/*e: global snarfversion */
/*s: global errorshouldabort */
bool errorshouldabort = false;
/*e: global errorshouldabort */
/*s: global winclosechan */
// chan<ref<Window>> (listener = winclosethread, sender = filsyswalk | filsysclunk )
Channel	*winclosechan; /* chan(Window*); */
/*e: global winclosechan */
/*s: global messagesize */
int	messagesize = 8192+IOHDRSZ;	/* good start */
/*e: global messagesize */
/*e: windows/rio/globals.c */
