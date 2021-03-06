/*s: assemblers/aa/globals.c */
#include "aa.h"

/*s: global [[hash]] */
// hash<string, ref_own<Sym>> (next = Sym.link in bucket)
Sym*	hash[NHASH];
/*e: global [[hash]] */
/*s: global [[pc]] */
long	pc;
/*e: global [[pc]] */
/*s: global [[outfile]] */
char*	outfile = nil;
/*e: global [[outfile]] */
/*s: global [[obuf]] */
Biobuf	obuf;
/*e: global [[obuf]] */
/*s: global [[pass]] */
// 1|2
int	pass;
/*e: global [[pass]] */
/*s: global [[pathname]] */
char*	pathname;
/*e: global [[pathname]] */

/*s: global [[thechar]] */
int	thechar;
/*e: global [[thechar]] */
/*s: global [[thestring]] */
char*	thestring;
/*e: global [[thestring]] */

/*s: global [[iostack]] */
// list<ref_own<Io> (next = Io.link)
Io*	iostack = I;
/*e: global [[iostack]] */
/*s: global [[iofree]] */
// list<ref<Io>> (next = Io.link)
Io*	iofree = I;
/*e: global [[iofree]] */
/*s: global [[ionext]] */
// option<ref<Io>>
Io*	ionext;
/*e: global [[ionext]] */
/*s: global [[fi]] */
struct Fi fi;
/*e: global [[fi]] */

/*s: global [[symb]] */
char	symb[NSYMB];
/*e: global [[symb]] */
/*s: global [[peekc]] */
// option<char> (None = IGN)
int	peekc = IGN;
/*e: global [[peekc]] */

/*s: global [[include]] */
// array<option<string>> (size = ninclude)
char*	include[NINCLUDE];
/*e: global [[include]] */
/*s: global [[ninclude]] */
int	ninclude;
/*e: global [[ninclude]] */
/*s: global [[Dlist]] */
char*	Dlist[30];
/*e: global [[Dlist]] */
/*s: global [[nDlist]] */
int	nDlist;
/*e: global [[nDlist]] */

/*s: global [[h]] */
// array<Htab>
struct Htab h[NSYM];
/*e: global [[h]] */
/*s: global [[symcounter]] */
int	symcounter;
/*e: global [[symcounter]] */

/*s: global [[lineno]] */
long	lineno;
/*e: global [[lineno]] */
/*s: global [[hist]] */
// list<ref_own<Hist>> (next = Hist.link)
Hist*	hist;
/*e: global [[hist]] */
/*s: global [[ehist]] */
// ref<Hist> (end from = hist)
Hist*	ehist;
/*e: global [[ehist]] */

/*s: global [[debug]] */
bool	debug[256];
/*e: global [[debug]] */

/*s: global [[nerrors]] */
int	nerrors = 0;
/*e: global [[nerrors]] */

/*s: global [[hunk]] */
char*	hunk;
/*e: global [[hunk]] */
/*s: global [[nhunk]] */
long	nhunk = 0;
/*e: global [[nhunk]] */
/*s: global [[thunk]] */
long	thunk;
/*e: global [[thunk]] */

/*e: assemblers/aa/globals.c */
