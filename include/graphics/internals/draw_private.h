/*s: include/graphics/internals/draw_private.h */

extern int		drawlsetrefresh(ulong, int, void*, void*);

// dead?
extern	int		_cursorfd;

extern	bool	_drawdebug;	/* set to true to see errors from flushimage */

// used also by window.c
extern Image*	_allocimage(Image*, Display*, Rectangle, ulong, int, ulong, int, int);
extern int	    _freeimage1(Image*);

extern	void	_setdrawop(Display*, Drawop);

// used also by libmemdraw/
void _twiddlecompressed(uchar *buf, int n);

/* XXX backwards helps; should go */
extern	ulong	drawld2chan[];
extern	void	drawsetdebug(bool);

#include <marshal.h>

/*e: include/graphics/internals/draw_private.h */
