#include "gc.h"

long	breakpc;
long	nbreak;
Case*	cases;
Node	constnode;
Node	fconstnode;
long	continpc;
long	curarg;
long	cursafe;
Prog*	firstp;
Prog*	lastp;
long	maxargsafe;
int	mnstring;
Multab	multab[20];
int	hintabsize;
Node*	nodrat;
Node*	nodret;
Node*	nodsafe;
long	nrathole;
long	nstring;
Prog*	p;
long	pc;
Node	regnode;
char	string[NSNAME];
Sym*	symrathole;
Node	znode;
Prog	zprog;
char	reg[NREG+NFREG];
long	exregoffset;
long	exfregoffset;
int	suppress;

Rgn	region[NRGN];
Rgn*	rgp;
int	nregion;
int	nvar;

Bits	externs;
Bits	params;
Bits	consts;
Bits	addrs;

long	regbits;
long	exregbits;

int	change;

Reg*	firstr;
Reg*	lastr;
Reg	zreg;
Reg*	freer;
Var	var[NVAR];
long*	idom;
Reg**	rpo2r;
long	maxnr;
