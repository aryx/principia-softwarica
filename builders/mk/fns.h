/*s: mk/fns.h */
// Utilities
void*	Malloc(int);
void*	Realloc(void*, int);
void	Exit(void);
char*	maketmp(void);
void	delete(char*);

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
Word*	wdup(Word*);
char*	wtos(Word*, int);
// Node
// Node* graph(char* target)
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
void	dumpr(char*, Rule*);
void	dumpn(char*, Node*);
void	dumpa(char*, Arc*);
void	dumpj(char*, Job*, int);



// Symbol table
void	syminit(void);
Symtab*	symlook(char*, int, void*);
void	symtraverse(int, void(*)(Symtab*));
void	symstat(void);
//void	symdel(char*, int);

// File time managment
ulong	timeof(char*, int);
void	timeinit(char*);
ulong	mkmtime(char*, bool);
int	chgtime(char*);
void	touch(char*);

// Archive managment
ulong	atimeof(int,char*);
void	atouch(char*);

// Env managment
void	initenv(void);
Envy*	buildenv(Job*, int);
void	readenv(void);
void	setvar(char*, void*);

// Matching and subst
int	match(char*, char*, char*);
void	subst(char*, char*, char*, int);
Word*	varsub(char**);

// Process managment
int	execsh(char*, char*, Bufblock*, Envy*);
void	killchildren(char*);
int	waitfor(char*);
int	waitup(int, int*);
int	pipecmd(char*, Envy*, int*);
void	nproc(void);

// Parsing
int	assline(Biobuf *, Bufblock *);
Word*	stow(char*);

// Main functions
void	mk(char*);
void	parse(char*, int, int);
Node*	graph(char*);
int	outofdate(Node*, Arc*, int);
int	dorecipe(Node*);
void	run(Job*);
void	prusage(void);


// MISC
void	usage(void);
void	catchnotes(void);
char* 	charin(char *, char *);
void	execinit(void);
int	escapetoken(Biobuf*, Bufblock*, int, int);
char*	expandquote(char*, Rune, Bufblock*);
void	expunge(int, char*);
void	front(char*);
int	nextrune(Biobuf*, int);
void	nrep(void);
void	rcopy(char**, Resub*, int);
char*	rulecnt(void);
char*	shname(char*);
void	shprint(char*, Envy*, Bufblock*);
void	update(bool, Node*);
/*e: mk/fns.h */
