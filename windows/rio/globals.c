/*s: rio/globals.c */
#include <u.h>
#include <libc.h>
/*s: rio includes */
#include <draw.h>
#include <cursor.h>
#include <mouse.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <thread.h>

#include "dat.h"
#include "fns.h"
/*e: rio includes */

/*s: global [[mousectl]] */
Mousectl	*mousectl;
/*e: global [[mousectl]] */
/*s: global [[mouse]] */
// alias for &mousectl->m
Mouse	*mouse;
/*e: global [[mouse]] */
/*s: global [[keyboardctl]] */
Keyboardctl	*keyboardctl;
/*e: global [[keyboardctl]] */

/*s: global [[desktop]] */
Screen	*desktop;
/*e: global [[desktop]] */
/*s: global [[background]] */
Image	*background;
/*e: global [[background]] */
/*s: global [[red]] */
Image	*red;
/*e: global [[red]] */

/*s: global [[windows]] */
// growing_array<ref_own<Window>> (allocated = nwindow+1)
Window	**windows;
/*e: global [[windows]] */
/*s: global [[wkeyboard]] */
Window	*wkeyboard;	/* window of simulated keyboard */
/*e: global [[wkeyboard]] */
/*s: global [[nwindow]] */
int	nwindow;
/*e: global [[nwindow]] */

/*s: global [[exitchan]] */
// chan<unit> (listener = threadmain, sender = mousethread(Exit) | ?)
Channel	*exitchan;	/* chan(int) */
/*e: global [[exitchan]] */

/*s: global [[winclosechan]] */
// chan<ref<Window>> (listener = winclosethread, sender = filsyswalk | filsysclunk )
Channel	*winclosechan; /* chan(Window*); */
/*e: global [[winclosechan]] */
/*s: global [[deletechan]] */
// chan<string> (listener = deletethread, sender = deletetimeoutproc)
Channel* deletechan;
/*e: global [[deletechan]] */

/*s: global [[input]] */
//option<ref<Window>>, the window with the focus! the window to send input to
Window	*input;
/*e: global [[input]] */
/*s: global [[all]] */
QLock	all;			/* BUG */
/*e: global [[all]] */
/*s: global [[filsys]] */
Filsys	*filsys;
/*e: global [[filsys]] */
/*s: global [[hidden]] */
// array<ref<Window>> (size valid = nhidden)
Window	*hidden[100];
/*e: global [[hidden]] */
/*s: global [[nhidden]] */
int		nhidden;
/*e: global [[nhidden]] */
/*s: global [[scrolling]] */
bool		scrolling;
/*e: global [[scrolling]] */

/*s: global [[startdir]] */
char		*startdir;
/*e: global [[startdir]] */
/*s: global [[sweeping]] */
bool	sweeping;
/*e: global [[sweeping]] */
/*s: global [[wctlfd]] */
int	wctlfd;
/*e: global [[wctlfd]] */
/*s: global [[menuing]] */
bool menuing;/* menu action is pending; waiting for window to be indicated */
/*e: global [[menuing]] */
/*s: global [[snarfversion]] */
int		snarfversion;	/* updated each time it is written */
/*e: global [[snarfversion]] */

/*s: global [[maxtab]] */
int		maxtab = 0;
/*e: global [[maxtab]] */
/*e: rio/globals.c */
