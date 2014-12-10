/*s: assemblers/aa/globals.c */
#include "aa.h"

/*s: global fi */
struct Fi fi;
/*e: global fi */
/*s: global h */
// array<ref<Sym>>
struct Htab h[NSYM];
/*e: global h */

/*s: global debug */
bool	debug[256];
/*e: global debug */
/*s: global hash */
// hash<string, ref_own?<Sym>>, (next in bucket = Sym.link)
Sym*	hash[NHASH];
/*e: global hash */
/*s: global Dlist */
char*	Dlist[30];
/*e: global Dlist */
/*s: global nDlist */
int	nDlist;
/*e: global nDlist */
/*s: global ehist */
// ref<Hist>, end of hist list?
Hist*	ehist;
/*e: global ehist */
/*s: global newflag */
bool	newflag;
/*e: global newflag */
/*s: global hist */
// list<ref_own<Hist>>, next = Hist.link
Hist*	hist;
/*e: global hist */
/*s: global hunk */
char*	hunk;
/*e: global hunk */
/*s: global include */
char*	include[NINCLUDE];
/*e: global include */
/*s: global iofree */
// list<IO>, next = IO.link
Io*	iofree = I;
/*e: global iofree */
/*s: global ionext */
Io*	ionext;
/*e: global ionext */
/*s: global iostack */
// list<Io> (next = Io.link)
Io*	iostack = I;
/*e: global iostack */
/*s: global lineno */
long	lineno;
/*e: global lineno */
/*s: global nerrors */
int	nerrors = 0;
/*e: global nerrors */
/*s: global nhunk */
long	nhunk = 0;
/*e: global nhunk */
/*s: global ninclude */
int	ninclude;
/*e: global ninclude */
/*s: global outfile */
char*	outfile = nil;
/*e: global outfile */
/*s: global pass */
// 1|2
int	pass;
/*e: global pass */
/*s: global pathname */
char*	pathname;
/*e: global pathname */
/*s: global pc */
long	pc;
/*e: global pc */
/*s: global peekc */
int	peekc = IGN;
/*e: global peekc */
/*s: global symcounter */
int	symcounter;
/*e: global symcounter */
/*s: global symb */
char	symb[NSYMB];
/*e: global symb */
/*s: global thechar */
int	thechar;
/*e: global thechar */
/*s: global thestring */
char*	thestring;
/*e: global thestring */
/*s: global thunk */
long	thunk;
/*e: global thunk */
/*s: global obuf */
Biobuf	obuf;
/*e: global obuf */
/*e: assemblers/aa/globals.c */
