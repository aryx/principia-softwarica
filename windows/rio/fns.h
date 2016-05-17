/*s: windows/rio/fns.h */

// thread_keyboard.c (for rio.c)
void	keyboardthread(void*);
// thread_mouse.c    (for rio.c)
void	mousethread(void*);
// threads_misc.c    (for rio.c)
void 	winclosethread(void*);
void 	deletethread(void*);
// threads_window.c
void    winctl(void*);
void    wsendctlmesg(Window*, int, Rectangle, Image*);

// data.c (for rio.c)
void	iconinit(void);
// timer.c (for rio.c and scrl.c)
void	timerinit(void);
void	timerstop(Timer*);
void	timercancel(Timer*);
Timer*	timerstart(int);
// xfid.c  (for rio.c and fsys.c)
Channel*xfidinit(void);
void	xfidattach(Xfid*);
void	xfidopen(Xfid*);
void	xfidclose(Xfid*);
void	xfidread(Xfid*);
void	xfidwrite(Xfid*);
void	xfidflush(Xfid*);
// fsys.c (for rio.c and xfid.c and process_winshell.c)
Filsys*	filsysinit(Channel*);
Xfid*	filsysrespond(Filsys*, Xfid*, Fcall*, char*);
void	filsyscancel(Xfid*);
int		filsysmount(Filsys*, int);

// cursor.c TODO
void	riosetcursor(Cursor*, int);

// TODO
int	    goodrect(Rectangle);


// wm.c (for thread_mouse.c)
void    cornercursor(Window *w, Point p, bool force);
Image	*bandsize(Window*);
Image*	drag(Window*, Rectangle*);
void    button3menu(void);
Window *new(Image*, int, int, int, char*, char*, char**); // for wkeyboard
int	whide(Window*); // for wctl
int	wunhide(int);   // for wctl

// wind.c
Window*	wpointto(Point);
void	wresize(Window*, Image*, int);
int		wclose(Window*);
void	wclosewin(Window*);
void	wrefresh(Window*, Rectangle);
void	wrepaint(Window*);
int		winborder(Window*, Point);
Window*	wtop(Point);
void	wtopme(Window*);
void	wbottomme(Window*);
void	wsetcursor(Window*, bool);
void	wfill(Window*);
void	wcurrent(Window*);
void	wmovemouse(Window*, Point);
Window*	wmk(Image*, Mousectl*, Channel*, Channel*, int);
void	wsetname(Window*);
void	wsetpid(Window*, int, int);
Window*	wlookid(int);

// processes_winshell TODO
void	winshell(void*);

// graphical_window.c TODO
void	waddraw(Window*, Rune*, int);

// terminal.c
void    button2menu(Window *w);
void	wkeyctl(Window*, Rune);
void	wmousectl(Window*);
void	wdelete(Window*, uint, uint);
uint	winsert(Window*, Rune*, int, uint);
void	wshow(Window*, uint);
void	wsetselect(Window*, uint, uint);
void	wsetorigin(Window*, uint, int);
uint	wbacknl(Window*, uint, uint);
char*	wcontents(Window*, int*);

// scroll.c
void	freescrtemps(void);
void	wscrdraw(Window*);
void 	wscrsleep(Window*, uint);
void	wscroll(Window*, int);

// snarf.c
void	putsnarf(void);
void	getsnarf(void);

// wctl.c (for fsys.c and xfid.c)
void	wctlproc(void*);
void	wctlthread(void*);
int	    parsewctl(char**, Rectangle, Rectangle*, int*, int*, int*, int*, char**, char*, char*);
int	    writewctl(Xfid*, char*);

// util.c
int	    min(int, int);
int	    max(int, int);
Rune*	strrune(Rune*, Rune);
int	    isalnum(Rune);
void	cvttorunes(char*, int, Rune*, int*, int*, int*);
/* was (byte*,int)	runetobyte(Rune*, int); */
char* 	runetobyte(Rune*, int, int*);
void*	erealloc(void*, uint);
void* 	emalloc(uint);
char* 	estrdup(char*);

/*s: function runemalloc */
#define	runemalloc(n)		malloc((n)*sizeof(Rune))
/*e: function runemalloc */
/*s: function runerealloc */
#define	runerealloc(a, n)	realloc(a, (n)*sizeof(Rune))
/*e: function runerealloc */
/*s: function runemove */
#define	runemove(a, b, n)	memmove(a, b, (n)*sizeof(Rune))
/*e: function runemove */

// error.c
void    derror(Display*, char *); // for main.c
void	error(char*);

/*e: windows/rio/fns.h */
