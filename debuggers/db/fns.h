/*s: db/fns.h */
void		acommand(int);
void		attachprocess(void);
void		bkput(BKPT*, bool);
void		bpwait(void);
int		charpos(void);
void		chkerr(void);
void		clrinp(void);
void		cmdmap(Map*);
void		cmdsrc(int, Map*);
void		cmdwrite(int, Map*);
int		command(char*, int);
int		convdig(int);
void		ctrace(int);
WORD		defval(WORD);
void		delbp(void);
void		done(void);
int		dprint(char*, ...);
Map*		dumbmap(int);
void		endline(void);
void		endpcs(void);
int		eol(int);
void		error(char*);
void		errors(char*, char*);
void		execbkpt(BKPT*, int);
char*		exform(int, int, char*, Map*, int, int);
int		expr(int);
void		flush(void);
void		flushbuf(void);
char*		getfname(void);
void		getformat(char*);
int		getnum(int (*)(void));
void		grab(void);
void		iclose(int, int);
ADDR		inkdot(int);
int		isfileref(void);
int		item(int);
void		killpcs(void);
void		kmsys(void);
void		main(int, char**);
int		mapimage(void);
void		newline(void);
int		nextchar(void);
void		notes(void);
void		oclose(void);
void		outputinit(void);
void		printc(int);
void		printesc(int);
void		printlocals(Symbol *, ADDR);
void		printmap(char*, Map*);
void		printparams(Symbol *, ADDR);
void		printpc(void);
void		printregs(int);
void		prints(char*);
void		printsource(ADDR);
void		printsym(void);
void		printsyscall(void);
void		printtrace(int);
int		quotchar(void);
int		rdc(void);
int		readchar(void);
void		readsym(char*);
void		redirin(int, char*);
void		redirout(char*);
void		readfname(char *);
void		reread(void);
char*		regname(int);
uvlong		rget(Map*, char*);
Reglist*	rname(char*);
void		rput(Map*, char*, vlong);
int		runpcs(int, int);
void		runrun(int);
void		runstep(uvlong, int);
BKPT*		scanbkpt(ADDR adr);
void		scanform(long, int, char*, Map*, int);
void		setbp(void);
void		setcor(void);
void		setsym(void);
void		setup(void);
void		setvec(void);
void		shell(void);
void		startpcs(void);
void		subpcs(int);
int		symchar(int);
int		term(int);
void		ungrab(void);
int		valpr(long, int);

#pragma	varargck	argpos	dprint	1
#pragma	varargck	type	"t"	void
/*e: db/fns.h */
