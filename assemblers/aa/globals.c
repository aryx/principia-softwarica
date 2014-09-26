/*s: assemblers/aa/globals.c */
#include "aa.h"

/*s: global fi */
struct Fi fi;
/*e: global fi */
/*s: global h */
struct Htab h[NSYM];
/*e: global h */

/*s: global debug */
bool	debug[256];
/*e: global debug */
/*s: global hash */
Sym*	hash[NHASH];
/*e: global hash */
/*s: global Dlist */
char*	Dlist[30];
/*e: global Dlist */
/*s: global nDlist */
int	nDlist;
/*e: global nDlist */
/*s: global ehist */
Hist*	ehist;
/*e: global ehist */
/*s: global newflag */
int	newflag;
/*e: global newflag */
/*s: global hist */
Hist*	hist;
/*e: global hist */
/*s: global hunk */
char*	hunk;
/*e: global hunk */
/*s: global include */
char*	include[NINCLUDE];
/*e: global include */
/*s: global iofree */
Io*	iofree;
/*e: global iofree */
/*s: global ionext */
Io*	ionext;
/*e: global ionext */
/*s: global iostack */
Io*	iostack;
/*e: global iostack */
/*s: global lineno */
long	lineno;
/*e: global lineno */
/*s: global nerrors */
int	nerrors;
/*e: global nerrors */
/*s: global nhunk */
long	nhunk;
/*e: global nhunk */
/*s: global ninclude */
int	ninclude;
/*e: global ninclude */
/*s: global outfile */
char*	outfile;
/*e: global outfile */
/*s: global pass */
int	pass;
/*e: global pass */
/*s: global pathname */
char*	pathname;
/*e: global pathname */
/*s: global pc */
long	pc;
/*e: global pc */
/*s: global peekc */
int	peekc;
/*e: global peekc */
/*s: global sym */
// bool?
int	sym;
/*e: global sym */
/*s: global symb */
char	symb[NSYMB];
/*e: global symb */
/*s: global thechar */
int	thechar = '8';
/*e: global thechar */
/*s: global thestring */
char*	thestring = "386";
/*e: global thestring */
/*s: global thunk */
long	thunk;
/*e: global thunk */
/*s: global obuf */
Biobuf	obuf;
/*e: global obuf */
/*e: assemblers/aa/globals.c */
