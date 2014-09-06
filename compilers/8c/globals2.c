#include "gc.h"

struct Idx idx;

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
Node*	nodrat;
Node*	nodret;
Node*	nodsafe;
long	nrathole;
long	nstring;
Prog*	p;
long	pc;
Node	regnode;
Node	fregnode0;
Node	fregnode1;
char	string[NSNAME];
Sym*	symrathole;
Node	znode;
Prog	zprog;
int	reg[D_NONE];
long	exregoffset;
long	exfregoffset;

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
int	suppress;

Reg*	firstr;
Reg*	lastr;
Reg	zreg;
Reg*	freer;
Var	var[NVAR];
long*	idom;
Reg**	rpo2r;
long	maxnr;
