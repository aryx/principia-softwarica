/*s: mk/fns.h */

// Constructors/destructors core data structures

// bufblock.c
Bufblock* newbuf(void);
void	freebuf(Bufblock*);
void	growbuf(Bufblock *);
void	bufcpy(Bufblock *, char *, int);
void	insert(Bufblock *, int);
void	rinsert(Bufblock *, Rune);

// words.c
Word*	newword(char*);
void	delword(Word*);
Word*	wdup(Word*);
char*	wtos(Word*, int);

// arc.c
Arc*	newarc(Node*, Rule*, char*, Resub*);

// rule.c
void	addrule(char*, Word*, char*, Word*, int, int, char*);
void	addrules(Word*, Word*, char*, int, int, char*);

// job.c
Job*	newjob(Rule*, Node*, char*, char**, Word*, Word*, Word*, Word*);


// symtab.c
void	syminit(void);
Symtab*	symlook(char*, int, void*);
void	symtraverse(int, void(*)(Symtab*));
void	symstat(void);
//void	symdel(char*, int);

// var.c
void	setvar(char*, void*);
char*	shname(char*);

// varsub.c
Word*	varsub(char**);

// file.c
ulong	timeof(char*, int);
void	timeinit(char*);
void	touch(char*);

// archive.c
ulong	atimeof(int,char*);
void	atouch(char*);

// env.c
Envy*	buildenv(Job*, int);


// lex.c
int		assline(Biobuf *, Bufblock *);

// parse.c
void	parse(char*, int, int);

// graph.c
Node*	graph(char*);

// match.c
int		match(char*, char*, char*);
void	subst(char*, char*, char*, int);

// mk.c
void	mk(char*);

// recipe.c
int		dorecipe(Node*);

// run.c
void	run(Job*);
int		waitup(int, int*);
void	killchildren(char*);
void	nproc(void);
void	prusage(void);
void	usage(void);

// shprint.c
void	shprint(char*, Envy*, Bufblock*);

// plan9.c
void	readenv(void);
int		execsh(char*, char*, Bufblock*, Envy*);
int		pipecmd(char*, Envy*, int*);
ulong	mkmtime(char*, bool);
int		chgtime(char*);
void	catchnotes(void);
int		waitfor(char*);

// rc.c
char* 	charin(char *, char *);


// utils.c
void*	Malloc(int);
void*	Realloc(void*, int);

// Dumpers
void	dumpv(char*);
void	dumpw(char*, Word*);
void	dumpr(char*, Rule*);
void	dumpn(char*, Node*);
void	dumpa(char*, Arc*);
void	dumpj(char*, Job*, int);


// MISC
void	Exit(void);
char*	maketmp(void);
void	delete(char*);
void	initenv(void);
int		outofdate(Node*, Arc*, int);
Word*	stow(char*);
char*	copyq(char*, Rune, Bufblock*);
void	execinit(void);
int		escapetoken(Biobuf*, Bufblock*, int, int);
char*	expandquote(char*, Rune, Bufblock*);
void	expunge(int, char*);
void	front(char*);
int		nextrune(Biobuf*, int);
void	nrep(void);
void	rcopy(char**, Resub*, int);
char*	rulecnt(void);
void	update(bool, Node*);
/*e: mk/fns.h */
