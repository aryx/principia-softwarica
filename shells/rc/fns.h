/*s: rc/fns.h */

//
void	Abort(void);
void	Closedir(int);
int	Eintr(void);
int	Executable(char*);
void	Execute(word*,  word*);
void	Exit(char*);
int	Globsize(char*);
int	Isatty(int);
void	Noerror(void);
int	Opendir(char*);
int	Readdir(int, void*, int);
void	Trapinit(void);
void	Updenv(void);
void	Vinit(void);
int	Waitfor(int, int);

void	addwaitpid(int);
void	cleanhere(char*);
void	codefree(code*);
error0	compile(tree*);
void	deglob(void*);
void	dotrap(void);
void	freenodes(void);
void	globlist(void);
int	idchr(int);
void	inttoascii(char*, long);
void	kinit(void);
int	match(void*, void*, int);
void	clearwaitpids(void);
void	poplist(void);
void	popword(void);
void	pprompt(void);
void	pushlist(void);
void	pushredir(int, int, int);
void	pushword(char*);
void	readhere(void);
//@Scheck: used in syn.y
void	skipnl(void);
void	start(code*, int, var*);
void	usage(char*);

// utils.c
void	Memcpy(void*, void*, long);
long	Read(int, void*, long);
long	Write(int, void*, long);
long	Seek(int, long, long);
void	Unlink(char*);
int	Creat(char*);
int	Dup(int, int);

// words.c
word* copynwords(word *a, word *tail, int n);
void freelist(word *w);
void	freewords(word*);
int	count(word*);

// var.c
void	setvar(char*, word*);

// status.c
void	setstatus(char*);
char* concstatus(char *s, char *t);
int	truestatus(void);

// error.c
void	panic(char*, int);
void	yyerror(char*);

// path.c
word*	searchpath(char*);

// lex.c
int	yylex(void);

// syn.y
//@Scheck: defined in syn.y and y.tab.c
int	yyparse(void);

/*e: rc/fns.h */
