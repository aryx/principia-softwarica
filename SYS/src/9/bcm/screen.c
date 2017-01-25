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


extern Point     curpos;
extern Rectangle window;
extern Memsubfont *memdefont;
extern Memimage *conscol;

extern void swconsole_init(void);
extern void swconsole_screenputs(char *s, int n);


/*
 * Software cursor. 
 */

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

static int
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
		return -1;
	xgscreen.r.max = Pt(width, height);
	xgscreen.depth = depth;
	return 0;
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

	h = memdefont->height;

	memimagedraw(gscreen, Rect(window.min.x, window.min.y,
		window.max.x, window.min.y + h + 5 + 6), orange, ZP, nil, ZP, S);
	freememimage(orange);

	window = insetrect(window, 5);

	greet = " Plan 9 Console ";
	p = addpt(window.min, Pt(10, 0));
	q = memsubfontwidth(memdefont, greet);
	memimagestring(gscreen, p, conscol, ZP, memdefont, greet);

	arch_flushmemscreen(gscreen->r); // was r before, but now in swconsole.c

	window.min.y += h + 6;
	curpos = window.min;
	window.max.y = window.min.y + ((window.max.y - window.min.y) / h) * h;
}


void
arch_screeninit(void)
{
	uchar *fb;
	int set;
	ulong chan;

	set = (screensize() == 0);
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

	gscreen = &xgscreen;
	gscreen->width = wordsperline(gscreen->r, gscreen->depth);

	memimageinit();
    swconsole_init();
	screenwin();

	screenputs = swconsole_screenputs;
}

void arch_flushmemscreen(Rectangle) { }

uchar*
arch_attachscreen(Rectangle *r, ulong *chan, int* d, int *width, int *softscreen)
{
	*r = gscreen->r;
	*d = gscreen->depth;
	*chan = gscreen->chan;
	*width = gscreen->width;
	*softscreen = 0;

	return gscreen->data->bdata;
}

void arch_getcolor(ulong p, ulong *pr, ulong *pg, ulong *pb)
{
	USED(p, pr, pg, pb);
}

int arch_setcolor(ulong p, ulong r, ulong g, ulong b)
{
	USED(p, r, g, b);
	return 0;
}

void
arch_blankscreen(int blank)
{
	fbblank(blank);
}


//old: #define ishwimage(i)	1		/* for ../port/devdraw.c */
bool arch_ishwimage(Memimage* i) { USED(i); return true; }
