/*s: windows/rio/fns.h */
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
void	killprocs(void);
int	shutdown(void*, char*);
void	iconinit(void);
void	*erealloc(void*, uint);
void *emalloc(uint);
char *estrdup(char*);
void	button3menu(void);
void	button2menu(Window*);
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
