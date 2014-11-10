/*s: windows/rio/fns.h */

// Window stuff
int		winborder(Window*, Point);
void		winctl(void*);
void		winshell(void*);
Window*	wlookid(int);
Window*	wmk(Image*, Mousectl*, Channel*, Channel*, int);
Window*	wpointto(Point);
Window*	wtop(Point);
void		wtopme(Window*);
void		wbottomme(Window*);
char*	wcontents(Window*, int*);
int		wclose(Window*);

uint		wbacknl(Window*, uint, uint);
uint		winsert(Window*, Rune*, int, uint);
void		waddraw(Window*, Rune*, int);
void		wcurrent(Window*);
void		wcut(Window*);
void		wmovemouse(Window*, Point);
void		wpaste(Window*);
void		wplumb(Window*);

void		wscrdraw(Window*);
void		wscroll(Window*, int);
void		wsendctlmesg(Window*, int, Rectangle, Image*);
void		wsetcursor(Window*, int);
void		wsetname(Window*);
void		wsetorigin(Window*, uint, int);
void		wsetpid(Window*, int, int);
void		wsetselect(Window*, uint, uint);
void		wshow(Window*, uint);
void		wsnarf(Window*);
void 		wscrsleep(Window*, uint);


// XFid stuff
Channel*	xfidinit(void);
void		xfidflush(Xfid*);
void		xfidattach(Xfid*);
void		xfidopen(Xfid*);
void		xfidclose(Xfid*);
void		xfidread(Xfid*);
void		xfidwrite(Xfid*);

// Filsys stuff
Filsys*	filsysinit(Channel*);
int		filsysmount(Filsys*, int);
Xfid*		filsysrespond(Filsys*, Xfid*, Fcall*, char*);
void		filsyscancel(Xfid*);

void		wctlproc(void*);
void		wctlthread(void*);

void		deletetimeoutproc(void*);



// Misc stuff
void	keyboardsend(char*, int);
int	whide(Window*);
int	wunhide(int);
void	freescrtemps(void);
int	parsewctl(char**, Rectangle, Rectangle*, int*, int*, int*, int*, char**, char*, char*);
int	writewctl(Xfid*, char*);
Window *new(Image*, int, int, int, char*, char*, char**);
void	riosetcursor(Cursor*, int);
int	min(int, int);
int	max(int, int);
Rune*	strrune(Rune*, Rune);
int	isalnum(Rune);
void	timerstop(Timer*);
void	timercancel(Timer*);
Timer*	timerstart(int);
void	error(char*);
void	iconinit(void);
void	*erealloc(void*, uint);
void *emalloc(uint);
char *estrdup(char*);
void	cvttorunes(char*, int, Rune*, int*, int*, int*);
/* was (byte*,int)	runetobyte(Rune*, int); */
char* runetobyte(Rune*, int, int*);
void	putsnarf(void);
void	getsnarf(void);
void	timerinit(void);
int	goodrect(Rectangle);


/*s: function runemalloc */
#define	runemalloc(n)		malloc((n)*sizeof(Rune))
/*e: function runemalloc */
/*s: function runerealloc */
#define	runerealloc(a, n)	realloc(a, (n)*sizeof(Rune))
/*e: function runerealloc */
/*s: function runemove */
#define	runemove(a, b, n)	memmove(a, b, (n)*sizeof(Rune))
/*e: function runemove */
/*e: windows/rio/fns.h */
