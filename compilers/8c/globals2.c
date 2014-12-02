/*s: 8c/globals2.c */
#include "gc.h"

/*s: global idx(x86) */
struct Idx idx;
/*e: global idx(x86) */

/*s: global breakpc(x86) */
long	breakpc;
/*e: global breakpc(x86) */
/*s: global nbreak(x86) */
long	nbreak;
/*e: global nbreak(x86) */
/*s: global cases(x86) */
Case*	cases;
/*e: global cases(x86) */
/*s: global constnode(x86) */
Node	constnode;
/*e: global constnode(x86) */
/*s: global fconstnode(x86) */
Node	fconstnode;
/*e: global fconstnode(x86) */
/*s: global continpc(x86) */
long	continpc;
/*e: global continpc(x86) */
/*s: global curarg(x86) */
long	curarg;
/*e: global curarg(x86) */
/*s: global cursafe(x86) */
long	cursafe;
/*e: global cursafe(x86) */
/*s: global firstp(x86) */
Prog*	firstp;
/*e: global firstp(x86) */
/*s: global lastp(x86) */
Prog*	lastp;
/*e: global lastp(x86) */
/*s: global maxargsafe(x86) */
long	maxargsafe;
/*e: global maxargsafe(x86) */
/*s: global mnstring(x86) */
int	mnstring;
/*e: global mnstring(x86) */
/*s: global nodrat(x86) */
Node*	nodrat;
/*e: global nodrat(x86) */
/*s: global nodret(x86) */
Node*	nodret;
/*e: global nodret(x86) */
/*s: global nodsafe(x86) */
Node*	nodsafe;
/*e: global nodsafe(x86) */
/*s: global nrathole(x86) */
long	nrathole;
/*e: global nrathole(x86) */
/*s: global nstring(x86) */
long	nstring;
/*e: global nstring(x86) */
/*s: global p(x86) */
Prog*	p;
/*e: global p(x86) */
/*s: global pc(x86) */
long	pc;
/*e: global pc(x86) */
/*s: global regnode(x86) */
Node	regnode;
/*e: global regnode(x86) */
/*s: global fregnode0(x86) */
Node	fregnode0;
/*e: global fregnode0(x86) */
/*s: global fregnode1(x86) */
Node	fregnode1;
/*e: global fregnode1(x86) */
/*s: global string(x86) */
char	string[NSNAME];
/*e: global string(x86) */
/*s: global symrathole(x86) */
Sym*	symrathole;
/*e: global symrathole(x86) */
/*s: global znode(x86) */
Node	znode;
/*e: global znode(x86) */
/*s: global zprog(x86) */
Prog	zprog;
/*e: global zprog(x86) */
/*s: global reg(x86) */
int	reg[D_NONE];
/*e: global reg(x86) */
/*s: global exregoffset(x86) */
long	exregoffset;
/*e: global exregoffset(x86) */
/*s: global exfregoffset(x86) */
long	exfregoffset;
/*e: global exfregoffset(x86) */

/*s: global region(x86) */
Rgn	region[NRGN];
/*e: global region(x86) */
/*s: global rgp(x86) */
Rgn*	rgp;
/*e: global rgp(x86) */
/*s: global nregion(x86) */
int	nregion;
/*e: global nregion(x86) */
/*s: global nvar(x86) */
int	nvar;
/*e: global nvar(x86) */

/*s: global externs(x86) */
Bits	externs;
/*e: global externs(x86) */
/*s: global params(x86) */
Bits	params;
/*e: global params(x86) */
/*s: global consts(x86) */
Bits	consts;
/*e: global consts(x86) */
/*s: global addrs(x86) */
Bits	addrs;
/*e: global addrs(x86) */

/*s: global regbits(x86) */
long	regbits;
/*e: global regbits(x86) */
/*s: global exregbits(x86) */
//long	exregbits;
/*e: global exregbits(x86) */

/*s: global change(x86) */
int	change;
/*e: global change(x86) */
/*s: global suppress(x86) */
int	suppress;
/*e: global suppress(x86) */

/*s: global firstr(x86) */
Reg*	firstr;
/*e: global firstr(x86) */
/*s: global lastr(x86) */
Reg*	lastr;
/*e: global lastr(x86) */
/*s: global zreg(x86) */
Reg	zreg;
/*e: global zreg(x86) */
/*s: global freer(x86) */
Reg*	freer;
/*e: global freer(x86) */
/*s: global var(x86) */
Var	var[NVAR];
/*e: global var(x86) */
/*s: global idom(x86) */
long*	idom;
/*e: global idom(x86) */
/*s: global rpo2r(x86) */
Reg**	rpo2r;
/*e: global rpo2r(x86) */
/*s: global maxnr(x86) */
long	maxnr;
/*e: global maxnr(x86) */
/*e: 8c/globals2.c */
