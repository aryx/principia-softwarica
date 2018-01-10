/*s: linkers/5l/globals.c */
#include	"l.h"
#include	"m.h"

/*s: global [[thechar]] */
char	thechar;
/*e: global [[thechar]] */
/*s: global [[thestring]] */
char*	thestring;
/*e: global [[thestring]] */

/*s: global [[HEADR]] */
long	HEADR; /* length of header */
/*e: global [[HEADR]] */
/*s: global [[HEADTYPE]] */
// option<enum<Headtype>>, None = -1
short	HEADTYPE = -1; /* type of header */
/*e: global [[HEADTYPE]] */
/*s: global [[INITDAT]] */
long	INITDAT = -1; /* data location */
/*e: global [[INITDAT]] */
/*s: global [[INITRND]] */
long	INITRND = -1; /* data round above text location */
/*e: global [[INITRND]] */
/*s: global [[INITTEXT]] */
long	INITTEXT = -1; /* text location */
/*e: global [[INITTEXT]] */
/*s: global [[INITTEXTP]] */
long	INITTEXTP = -1; /* text location (physical) */
/*e: global [[INITTEXTP]] */
/*s: global [[INITENTRY]] */
char*	INITENTRY = nil;		/* entry point */
/*e: global [[INITENTRY]] */

/*s: global [[outfile]] */
char*	outfile;
/*e: global [[outfile]] */
/*s: global [[cout]] */
fdt	cout = -1;
/*e: global [[cout]] */
/*s: global [[bso]] */
Biobuf	bso;
/*e: global [[bso]] */

/*s: global [[curauto]] */
// list<ref<Auto>> (next = Auto.link)
Auto*	curauto;
/*e: global [[curauto]] */
/*s: global [[curhist]] */
Auto*	curhist;
/*e: global [[curhist]] */
/*s: global [[curp]] */
// option<ref<Prog>>
Prog*	curp;
/*e: global [[curp]] */
/*s: global [[curtext]] */
//option<ref<Prog>> where Prog.as == ATEXT
Prog*	curtext = P;
/*e: global [[curtext]] */

/*s: global [[autosize]](arm) */
long	autosize;
/*e: global [[autosize]](arm) */
/*s: global [[instoffset]](arm) */
long	instoffset;
/*e: global [[instoffset]](arm) */


/*s: global [[datap]] */
// list<ref_own<Prog>> (next = Prog.link)
Prog*	datap = P;
/*e: global [[datap]] */
/*s: global [[etextp]] */
// ref<Prog> (end from = textp)
Prog*	etextp = P;
/*e: global [[etextp]] */
/*s: global [[firstp]] */
// list<ref_own<Prog>> (next = Prog.link)
Prog*	firstp;
/*e: global [[firstp]] */
/*s: global [[lastp]] */
// ref<Prog> (end from = firstp)
Prog*	lastp;
/*e: global [[lastp]] */
/*s: global [[textp]] */
// list<ref<Prog>> (next = Prog.cond)
Prog*	textp = P;
/*e: global [[textp]] */


/*s: global [[debug]] */
bool	debug[128];
/*e: global [[debug]] */


/*s: global [[textsize]] */
long	textsize;
/*e: global [[textsize]] */
/*s: global [[datsize]] */
long	datsize;
/*e: global [[datsize]] */
/*s: global [[bsssize]] */
long	bsssize;
/*e: global [[bsssize]] */
/*s: global [[symsize]] */
long	symsize;
/*e: global [[symsize]] */
/*s: global [[lcsize]] */
long	lcsize;
/*e: global [[lcsize]] */

/*s: global hash linker */
// hash<Sym.name * Sym.version, ref_own<Sym>> (next = Sym.link in bucket)
Sym*	hash[NHASH];
/*e: global hash linker */
/*s: global [[pc]] */
long	pc = 0;
/*e: global [[pc]] */
/*s: global [[zprg]] */
Prog	zprg;
/*e: global [[zprg]] */

/*s: global [[histfrog]] */
Sym*	histfrog[MAXHIST];
/*e: global [[histfrog]] */
/*s: global [[histfrogp]] */
int	histfrogp;
/*e: global [[histfrogp]] */
/*s: global [[histgen]] */
int	histgen = 0;
/*e: global [[histgen]] */

/*s: global [[xrefresolv]] */
bool	xrefresolv;
/*e: global [[xrefresolv]] */

/*s: global [[thunk]] */
long	thunk;
/*e: global [[thunk]] */
/*s: global nsymbol linker */
long	nsymbol;
/*e: global nsymbol linker */


/*s: global [[armv4]](arm) */
bool	armv4;
/*e: global [[armv4]](arm) */
/*s: global [[vfp]](arm) */
bool vfp;
/*e: global [[vfp]](arm) */

/*s: global [[doexp]] */
// do export table, -x
bool	doexp;
/*e: global [[doexp]] */
/*s: global [[dlm]] */
bool dlm;
/*e: global [[dlm]] */

/*s: global [[EXPTAB]] */
char*	EXPTAB;
/*e: global [[EXPTAB]] */
/*s: global [[undefp]] */
//@Scheck: not dead, used by UP
Prog	undefp;
/*e: global [[undefp]] */

/*e: linkers/5l/globals.c */
