#include "aa.h"

struct Fi fi;
struct Htab h[NSYM];

char	debug[256];
Sym*	hash[NHASH];
char*	Dlist[30];
int	nDlist;
Hist*	ehist;
int	newflag;
Hist*	hist;
char*	hunk;
char*	include[NINCLUDE];
Io*	iofree;
Io*	ionext;
Io*	iostack;
long	lineno;
int	nerrors;
long	nhunk;
int	ninclude;
char*	outfile;
int	pass;
char*	pathname;
long	pc;
int	peekc;
int	sym;
char	symb[NSYMB];
int	thechar;
char*	thestring;
long	thunk;
Biobuf	obuf;
