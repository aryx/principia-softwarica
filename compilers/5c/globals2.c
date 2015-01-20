/*s: 5c/globals2.c */
#include "gc.h"

/*s: global breakpc(arm) */
long	breakpc;
/*e: global breakpc(arm) */
/*s: global nbreak(arm) */
long	nbreak;
/*e: global nbreak(arm) */
/*s: global cases(arm) */
Case*	cases;
/*e: global cases(arm) */
/*s: global constnode(arm) */
Node	constnode;
/*e: global constnode(arm) */
/*s: global fconstnode(arm) */
Node	fconstnode;
/*e: global fconstnode(arm) */
/*s: global continpc(arm) */
long	continpc;
/*e: global continpc(arm) */
/*s: global curarg(arm) */
long	curarg;
/*e: global curarg(arm) */
/*s: global cursafe(arm) */
long	cursafe;
/*e: global cursafe(arm) */
/*s: global firstp(arm) */
Prog*	firstp;
/*e: global firstp(arm) */
/*s: global lastp(arm) */
Prog*	lastp;
/*e: global lastp(arm) */
/*s: global maxargsafe(arm) */
long	maxargsafe;
/*e: global maxargsafe(arm) */
/*s: global mnstring(arm) */
int	mnstring;
/*e: global mnstring(arm) */
/*s: global multab(arm) */
Multab	multab[20];
/*e: global multab(arm) */
/*s: global hintabsize(arm) */
int	hintabsize;
/*e: global hintabsize(arm) */
/*s: global nodrat(arm) */
Node*	nodrat;
/*e: global nodrat(arm) */
/*s: global nodret(arm) */
Node*	nodret;
/*e: global nodret(arm) */
/*s: global nodsafe(arm) */
Node*	nodsafe;
/*e: global nodsafe(arm) */
/*s: global nrathole(arm) */
long	nrathole;
/*e: global nrathole(arm) */
/*s: global nstring(arm) */
long	nstring;
/*e: global nstring(arm) */
/*s: global p(arm) */
Prog*	p;
/*e: global p(arm) */
/*s: global pc(arm) */
long	pc;
/*e: global pc(arm) */
/*s: global regnode(arm) */
Node	regnode;
/*e: global regnode(arm) */
/*s: global string(arm) */
char	string[NSNAME];
/*e: global string(arm) */
/*s: global symrathole(arm) */
Sym*	symrathole;
/*e: global symrathole(arm) */
/*s: global znode(arm) */
Node	znode;
/*e: global znode(arm) */
/*s: global zprog(arm) */
Prog	zprog;
/*e: global zprog(arm) */
/*s: global reg(arm) */
char	reg[NREG+NFREG];
/*e: global reg(arm) */
/*s: global exregoffset(arm) */
long	exregoffset;
/*e: global exregoffset(arm) */
/*s: global exfregoffset(arm) */
long	exfregoffset;
/*e: global exfregoffset(arm) */
/*s: global suppress(arm) */
bool	suppress;
/*e: global suppress(arm) */

/*s: global region(arm) */
Rgn	region[NRGN];
/*e: global region(arm) */
/*s: global rgp(arm) */
Rgn*	rgp;
/*e: global rgp(arm) */
/*s: global nregion(arm) */
int	nregion;
/*e: global nregion(arm) */
/*s: global nvar(arm) */
int	nvar;
/*e: global nvar(arm) */

/*s: global externs(arm) */
Bits	externs;
/*e: global externs(arm) */
/*s: global params(arm) */
Bits	params;
/*e: global params(arm) */
/*s: global consts(arm) */
Bits	consts;
/*e: global consts(arm) */
/*s: global addrs(arm) */
Bits	addrs;
/*e: global addrs(arm) */

/*s: global regbits(arm) */
long	regbits;
/*e: global regbits(arm) */
/*s: global exregbits(arm) */
long	exregbits;
/*e: global exregbits(arm) */

/*s: global change(arm) */
int	change;
/*e: global change(arm) */

/*s: global firstr(arm) */
Reg*	firstr;
/*e: global firstr(arm) */
/*s: global lastr(arm) */
Reg*	lastr;
/*e: global lastr(arm) */
/*s: global zreg(arm) */
Reg	zreg;
/*e: global zreg(arm) */
/*s: global freer(arm) */
Reg*	freer;
/*e: global freer(arm) */
/*s: global var(arm) */
Var	var[NVAR];
/*e: global var(arm) */
/*s: global idom(arm) */
long*	idom;
/*e: global idom(arm) */
/*s: global rpo2r(arm) */
Reg**	rpo2r;
/*e: global rpo2r(arm) */
/*s: global maxnr(arm) */
long	maxnr;
/*e: global maxnr(arm) */
/*e: 5c/globals2.c */
