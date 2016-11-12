/*s: mk/fns.h */

// Constructors/destructors for core data structures

// bufblock.c
Bufblock* newbuf(void);
void	freebuf(Bufblock*);
void	growbuf(Bufblock *);
void	bufcpy(Bufblock *, char *, int);
void	insert(Bufblock *, int);
void	rinsert(Bufblock *, Rune);
/*s: macro isempty */
#define isempty(buf) (buf->current == buf->start)
/*e: macro isempty */
/*s: macro resetbuf */
#define resetbuf(buf) do { buf->current = buf->start; } while(0)
/*e: macro resetbuf */
/*s: macro bufcontent */
#define bufcontent(buf) buf->start
/*e: macro bufcontent */

// words.c
Word*	newword(char*);
void	freewords(Word*);
Word*	wdup(Word*);
char*	wtos(Word*, int);
void	addw(Word*, char*);

// symtab.c
Symtab*	symlook(char*, int, void*);
void	symtraverse(int, void(*)(Symtab*));
void	symstat(void);

// var.c
void	setvar(char*, void*);
char*	shname(char*);

// rule.c
void	addrule(char*, Word*, char*, Word*, int, int, char*);
void	addrules(Word*, Word*, char*, int, int, char*);
char*	rulecnt(void);



// env.c
void	inithash(void);
void	initshellenv(void);
ShellEnvVar*	buildenv(Job*, int);



// lex.c
int		assline(Biobuf *, Bufblock *);
int		nextrune(Biobuf*, int);

// parse.c
void	parse(char*, int, int);
Word*	stow(char*);

// varsub.c
Word*	varsub(char**);



// graph.c
Node*	graph(char*);
void	nrep(void);

// file.c
ulong	timeof(char*, int);
void	timeinit(char*);
void	touch(char*);

// match.c
int		match(char*, char*, char*);
void	subst(char*, char*, char*, int);



// mk.c
void	mk(char*);
int		outofdate(Node*, Arc*, int);
void	update(Node*, bool);

// recipe.c
void		dorecipe(Node*, bool*);

// run.c
void	run(Job*);
int		waitup(int, int*);
void	killchildren(char*);
void	nproc(void);

void	prusage(void);
void	usage(void);

// shprint.c
void	shprint(char*, ShellEnvVar*, Bufblock*);
void	front(char*);



// plan9.c
int		execsh(char*, char*, Bufblock*, ShellEnvVar*);
int		pipecmd(char*, ShellEnvVar*, int*);
int		waitfor(char*);
void	catchnotes(void);
void	readenv(void);
ulong	mkmtime(char*, bool);
int		chgtime(char*);
void	Exit(void);
char*	maketmp(void);
void	delete(char*);
void	expunge(int, char*);
void	rcopy(char**, Resub*, int);

// rc.c
char* 	charin(char *, char *);
char*	copyq(char*, Rune, Bufblock*);
int		escapetoken(Biobuf*, Bufblock*, int, int);
char*	expandquote(char*, Rune, Bufblock*);


// archive.c
ulong	atimeof(int,char*);
void	atouch(char*);


// utils.c
void*	Malloc(int);
void*	Realloc(void*, int);

// Dumpers
void	dumpv(char*);
void	dumpw(char*, Word*);
void	dumpr(char*, Rule*);
void	dumpn(char*, Node*);
void	dumpj(char*, Job*, int);

/*e: mk/fns.h */
