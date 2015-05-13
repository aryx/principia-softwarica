/*s: include/memlayer.h */
#pragma src "/sys/src/libmemlayer"
#pragma lib "libmemlayer.a"

typedef struct Memscreen Memscreen;
typedef void (*Refreshfn)(Memimage*, Rectangle, void*);

/*s: struct Memscreen */
struct Memscreen
{
    Memimage	*frontmost;	/* frontmost layer on screen */
    Memimage	*rearmost;	/* rearmost layer on screen */
    Memimage	*image;		/* upon which all layers are drawn */
    Memimage	*fill;			/* if non-zero, picture to use when repainting */
};
/*e: struct Memscreen */

/*s: struct Memlayer */
struct Memlayer
{
    Rectangle		screenr;	/* true position of layer on screen */
    Point			delta;	/* add delta to go from image coords to screen */
    Memscreen	*screen;	/* screen this layer belongs to */
    Memimage	*front;	/* window in front of this one */
    Memimage	*rear;	/* window behind this one*/
    int		clear;	/* layer is fully visible */
    Memimage	*save;	/* save area for obscured parts */
    Refreshfn	refreshfn;		/* function to call to refresh obscured parts if save==nil */
    void		*refreshptr;	/* argument to refreshfn */
};
/*e: struct Memlayer */

/*
 * These functions accept local coordinates
 */
int			memload(Memimage*, Rectangle, uchar*, int, int);
int			memunload(Memimage*, Rectangle, uchar*, int);

/*
 * All these functions accept screen coordinates, not local ones.
 */

Memimage*		memlalloc(Memscreen*, Rectangle, Refreshfn, void*, ulong);
void			memlfree(Memimage*);
void			memldelete(Memimage*);

int				memlorigin(Memimage*, Point, Point);

void			memltofront(Memimage*);
void			memltofrontn(Memimage**, int);
void			memltorear(Memimage*);
void			memltorearn(Memimage**, int);
void			memlhide(Memimage*, Rectangle);
void			memlexpose(Memimage*, Rectangle);

int				memlsetrefresh(Memimage*, Refreshfn, void*);
void			memlnorefresh(Memimage*, Rectangle, void*);

void			_memlayerop(void (*fn)(Memimage*, Rectangle, Rectangle, void*, int), Memimage*, Rectangle, Rectangle, void*);

void			_memltofrontfill(Memimage*, int);
void			_memlsetclear(Memscreen*);
/*e: include/memlayer.h */
