/*
 * bcm2385 framebuffer
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#include <draw.h>
#include <font.h>
#include <memdraw.h>
#include <cursor.h>

#include "../port/portscreen.h"

enum {
	Wid		= 1024,
	Ht		= 768,
	Depth		= 16,
};

static Memdata xgdata;

static Memimage xgscreen =
{
	.r = { 0, 0, Wid, Ht },
	.clipr = { 0, 0, Wid, Ht },
	.depth = Depth,
	.nchan = 3,
	.chan = RGB16,
	.cmap = nil,
	.data = &xgdata,
	.zero = 0,
	.width = 0, 			/* width in words of a single scan line */
	.layer = nil,
	.flags = 0,
};



/*
 * Software cursor. 
 */

// could factorize with swcursor_arrow
Cursor	arch_arrow = {
	{ -1, -1 },
	{ 0xFF, 0xFF, 0x80, 0x01, 0x80, 0x02, 0x80, 0x0C,
	  0x80, 0x10, 0x80, 0x10, 0x80, 0x08, 0x80, 0x04,
	  0x80, 0x02, 0x80, 0x01, 0x80, 0x02, 0x8C, 0x04,
	  0x92, 0x08, 0x91, 0x10, 0xA0, 0xA0, 0xC0, 0x40,
	},
	{ 0x00, 0x00, 0x7F, 0xFE, 0x7F, 0xFC, 0x7F, 0xF0,
	  0x7F, 0xE0, 0x7F, 0xE0, 0x7F, 0xF0, 0x7F, 0xF8,
	  0x7F, 0xFC, 0x7F, 0xFE, 0x7F, 0xFC, 0x73, 0xF8,
	  0x61, 0xF0, 0x60, 0xE0, 0x40, 0x40, 0x00, 0x00,
	},
};

bool
arch_cursoron(bool dolock)
{
	bool retry;

	if (dolock)
		lock(&cursor);
	if (canqlock(&drawlock)) {
		retry = false;
		swcursor_hide();
		swcursor_draw();
		qunlock(&drawlock);
	} else
		retry = true;
	if (dolock)
		unlock(&cursor);
	return retry;
}

void
arch_cursoroff(bool dolock)
{
	if (dolock)
		lock(&cursor);
	swcursor_hide();
	if (dolock)
		unlock(&cursor);
}


/* called from devmouse */
void
arch_ksetcursor(Cursor* curs)
{
	arch_cursoroff(false);
	swcursor_load(curs);
	arch_cursoron(false);
}

/*
 * Drawing
 */

int
hwdraw(Memdrawparam *par)
{
	Memimage *dst, *src, *mask;

	if((dst=par->dst) == nil || dst->data == nil)
		return 0;
	if((src=par->src) == nil || src->data == nil)
		return 0;
	if((mask=par->mask) == nil || mask->data == nil)
		return 0;

	if(dst->data->bdata == xgdata.bdata)
		swcursor_avoid(par->r);
	if(src->data->bdata == xgdata.bdata)
		swcursor_avoid(par->sr);
	if(mask->data->bdata == xgdata.bdata)
		swcursor_avoid(par->mr);

	return 0;
}

static bool
screensize(void)
{
	char *p;
	char *f[3];
	int width, height, depth;

	p = getconf("vgasize");
	if(p == nil || getfields(p, f, nelem(f), 0, "x") != nelem(f) ||
	    (width = atoi(f[0])) < 16 ||
	    (height = atoi(f[1])) <= 0 ||
	    (depth = atoi(f[2])) <= 0)
		return false;
	xgscreen.r.max = Pt(width, height);
	xgscreen.depth = depth;
	return true;
}

static void
screenwin(void)
{
	char *greet;
	Memimage *orange;
	Point p, q;
    int h;

	orange = allocmemimage(Rect(0, 0, 1, 1), RGB16);
	orange->flags |= Frepl;
	orange->clipr = gscreen->r;
	orange->data->bdata[0] = 0x40;		/* magic: colour? */
	orange->data->bdata[1] = 0xfd;		/* magic: colour? */

	h = swconsole_memdefont->height;

	memimagedraw(gscreen, 
                 Rect(swconsole_window.min.x, swconsole_window.min.y,
                      swconsole_window.max.x, swconsole_window.min.y + h + 5 + 6), 
                 orange, ZP, nil, ZP, S);
	freememimage(orange);

	swconsole_window = insetrect(swconsole_window, 5);

	greet = " Plan 9 Console ";
	p = addpt(swconsole_window.min, Pt(10, 0));
	q = memsubfontwidth(swconsole_memdefont, greet);
	memimagestring(gscreen, p, swconsole_conscol, ZP, swconsole_memdefont, 
                   greet);

	arch_flushmemscreen(gscreen->r); // was r before, but now in swconsole.c

	swconsole_window.min.y += h + 6;
	swconsole_curpos = swconsole_window.min;
	swconsole_window.max.y = 
      swconsole_window.min.y + 
      ((swconsole_window.max.y - swconsole_window.min.y) / h) * h;
}


void
arch_screeninit(void)
{
	uchar *fb;
	int set;
	ulong chan;

	set = screensize();
	fb = fbinit(set, &xgscreen.r.max.x, &xgscreen.r.max.y, &xgscreen.depth);
	if(fb == nil){
		print("can't initialise %dx%dx%d framebuffer \n",
			xgscreen.r.max.x, xgscreen.r.max.y, xgscreen.depth);
		return;
	}
	xgscreen.clipr = xgscreen.r;
	switch(xgscreen.depth){
	default:
		print("unsupported screen depth %d\n", xgscreen.depth);
		xgscreen.depth = 16;
		/* fall through */
	case 16:
		chan = RGB16;
		break;
	case 24:
		chan = BGR24;
		break;
	case 32:
		chan = ARGB32;
		break;
	}
	memsetchan(&xgscreen, chan);
	//conf.monitor = 1;
	xgdata.bdata = fb;
	xgdata.ref = 1;

    // set portscreen.c globals (used by draw.c, swcursor.c, swconsole
	gscreen = &xgscreen;
	gscreen->width = wordsperline(gscreen->r, gscreen->depth);

	memimageinit();
    swconsole_init();
	screenwin();

	screenputs = swconsole_screenputs;
}

uchar*
arch_attachscreen(Rectangle *r, ulong *chan, int* d, int *width, bool *softscreen)
{
	*r = gscreen->r;
	*d = gscreen->depth;
	*chan = gscreen->chan;
	*width = gscreen->width;
	*softscreen = false;

	return gscreen->data->bdata;
}

void
arch_blankscreen(bool blank)
{
	fbblank(blank);
}

void arch_flushmemscreen(Rectangle) { }
//old: #define ishwimage(i)	1		/* for ../port/devdraw.c */
bool arch_ishwimage(Memimage*) { return true; }
void arch_getcolor(ulong, ulong*, ulong*, ulong*) { }
int  arch_setcolor(ulong, ulong, ulong, ulong) { return 0; }
