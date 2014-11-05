/*s: mk/fns.h */
// Utilities
void*	Malloc(int);
void*	Realloc(void*, int);
void	Exit(void);

// Constructors/destructors core data structures
// Bufblock
Bufblock* newbuf(void);
void	freebuf(Bufblock*);
void	growbuf(Bufblock *);
void	bufcpy(Bufblock *, char *, int);
char*	copyq(char*, Rune, Bufblock*);
void	insert(Bufblock *, int);
void	rinsert(Bufblock *, Rune);
// Word
Word*	newword(char*);
void	delword(Word*);
// Node
// ???
// Arc
Arc*	newarc(Node*, Rule*, char*, Resub*);
// Rule
void	addrule(char*, Word*, char*, Word*, int, int, char*);
void	addrules(Word*, Word*, char*, int, int, char*);
// Job
Job*	newjob(Rule*, Node*, char*, char**, Word*, Word*, Word*, Word*);

// Dumpers
void	dumpv(char*);
void	dumpw(char*, Word*);
void	dumpn(char*, Node*);
void	dumpa(char*, Arc*);
void	dumpr(char*, Rule*);
void	dumpj(char*, Job*, int);



// Symbol table
void	syminit(void);
//void	symdel(char*, int);
Symtab*	symlook(char*, int, void*);
void	symtraverse(int, void(*)(Symtab*));
void	symstat(void);

// File managment
void	touch(char*);

// Time managment
ulong	timeof(char*, int);
void	timeinit(char*);
ulong	mkmtime(char*, bool);
ulong	mtime(char*);
int	chgtime(char*);

// Archive managment
ulong	atimeof(int,char*);
void	atouch(char*);

// Env managment
void	initenv(void);
Envy*	buildenv(Job*, int);
void	readenv(void);
void	setvar(char*, void*);

// Matching and subst
void	subst(char*, char*, char*, int);
Word*	varsub(char**);
int	match(char*, char*, char*);

// Process managment
int	execsh(char*, char*, Bufblock*, Envy*);
void	killchildren(char*);
int	waitfor(char*);
int	waitup(int, int*);
int	pipecmd(char*, Envy*, int*);
void	nproc(void);

// Main functions
Node*	graph(char*);
void	parse(char*, int, int);
void	run(Job*);
void	prusage(void);
void	usage(void);
void	mk(char*);


// MISC
int	assline(Biobuf *, Bufblock *);
void	catchnotes(void);
char* 	charin(char *, char *);
void	delete(char*);
int	dorecipe(Node*);
int	escapetoken(Biobuf*, Bufblock*, int, int);
void	execinit(void);
char*	expandquote(char*, Rune, Bufblock*);
void	expunge(int, char*);
void	front(char*);
char	*maketmp(void);
int	nextrune(Biobuf*, int);
void	nrep(void);
int	outofdate(Node*, Arc*, int);
void	rcopy(char**, Resub*, int);
char*	rulecnt(void);
char*	shname(char*);
void	shprint(char*, Envy*, Bufblock*);
Word*	stow(char*);
void	update(int, Node*);
Word*	wdup(Word*);
char*	wtos(Word*, int);
/*e: mk/fns.h */
