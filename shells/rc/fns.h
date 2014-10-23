/*s: rc/fns.h */
void	Abort(void);
void	Closedir(int);
int	Creat(char*);
int	Dup(int, int);
int	Dup1(int);
int	Eintr(void);
int	Executable(char*);
void	Execute(word*,  word*);
void	Exit(char*);
int	Globsize(char*);
int	Isatty(int);
void	Memcpy(void*, void*, long);
void	Noerror(void);
int	Opendir(char*);
long	Read(int, void*, long);
int	Readdir(int, void*, int);
long	Seek(int, long, long);
void	Trapinit(void);
void	Unlink(char*);
void	Updenv(void);
void	Vinit(void);
int	Waitfor(int, int);
long	Write(int, void*, long);

void	addwaitpid(int);
int	advance(void);
void	cleanhere(char*);
void	codefree(code*);
int	compile(tree*);
int	count(word*);
void	deglob(void*);
void	dotrap(void);
void	freenodes(void);
void	freewords(word*);
void	globlist(void);
int	idchr(int);
void	inttoascii(char*, long);
void	kinit(void);
int	match(void*, void*, int);
void	clearwaitpids(void);
void	panic(char*, int);
void	poplist(void);
void	popword(void);
void	pprompt(void);
void	pushlist(void);
void	pushredir(int, int, int);
void	pushword(char*);
void	readhere(void);

void	setstatus(char*);
void	setvar(char*, word*);
//@Scheck: used in syn.y
void	skipnl(void);
void	start(code*, int, var*);
int	truestatus(void);
void	usage(char*);
void	yyerror(char*);
int	yylex(void);
//@Scheck: defined in syn.y and y.tab.c
int	yyparse(void);
/*e: rc/fns.h */
