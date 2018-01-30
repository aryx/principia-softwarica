/*s: rc/fns.h */

// for var.c, words.c, tree.c: see rc.h
// see also io.h and getflags.h

// input.c
int getnext(void);
int advance(void);
int nextc(void);
bool nextis(int c);
void	pprompt(void);
void	kinit(void);
tree *token(char*, int);
tree *klook(char*);

// lex.c
int	yylex(void);
//@Scheck: used in syn.y
void	skipnl(void);
int	idchr(int);

// syn.y
//@Scheck: defined in syn.y and y.tab.c
int	yyparse(void);

// executils.c
void	start(code*, int, var*);
void	pushlist(void);
void	poplist(void);
void	pushword(char*);
void	popword(void);
void	pushredir(int, int, int);

// status.c
char *getstatus(void);
void	setstatus(char*);
char* concstatus(char *s, char *t);
int	truestatus(void);

// path.c
word*	searchpath(char*);

// env.c
void	Updenv(void);
void	Vinit(void);

// processes.c
int	Waitfor(int, int);
void	Execute(word*,  word*);

// code.c
error0	compile(tree*);
code *codecopy(code*);
void	codefree(code*);
void	cleanhere(char*);

// trap.c
void	dotrap(void);
void	Trapinit(void);
int	Eintr(void);
void	Noerror(void);

// here.c
void	readhere(void);
tree *heredoc(tree*);

// glob.c
void	deglob(void*);
void	globlist(void);
int	match(void*, void*, int);

// utils.c
#define	new(type)	((type *)emalloc(sizeof(type)))
void *emalloc(long);
void efree(void *);
void	Memcpy(void*, void*, long);
long	Read(int, void*, long);
long	Write(int, void*, long);
long	Seek(int, long, long);
void	Unlink(char*);
int	Creat(char*);
int	Dup(int, int);
int	Opendir(char*);
int	Readdir(int, void*, int);
void	Closedir(int);
void	inttoascii(char*, long);

// error.c
void	panic(char*, int);
void	yyerror(char*);
void	Exit(char*);

/*e: rc/fns.h */
