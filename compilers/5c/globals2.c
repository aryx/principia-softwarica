/*s: 5c/globals2.c */
#include "gc.h"

/*s: global [[breakpc]] */
// option<pc>, None = -1
long	breakpc;
/*e: global [[breakpc]] */
/*s: global [[nbreak]] */
long nbreak;
/*e: global [[nbreak]] */
/*s: global [[cases]] */
Case*	cases;
/*x: global [[cases]] */
Case*	cases;
/*e: global [[cases]] */
/*s: global [[constnode]] */
Node	constnode;
/*e: global [[constnode]] */
/*s: global [[fconstnode]] */
Node	fconstnode;
/*e: global [[fconstnode]] */
/*s: global [[continpc]] */
// option<pc>, None = -1
long	continpc;
/*e: global [[continpc]] */

/*s: global [[curarg]] */
long	curarg;
/*e: global [[curarg]] */
/*s: global [[cursafe]] */
long	cursafe;
/*e: global [[cursafe]] */

/*s: global [[firstp]] */
// list<ref_own<Prog> (next = Prog.link, last = lastp)
Prog*	firstp;
/*e: global [[firstp]] */
/*s: global [[lastp]] */
// ref<Prog> (head = firstp)
Prog*	lastp;
/*e: global [[lastp]] */

/*s: global [[maxargsafe]] */
long	maxargsafe;
/*e: global [[maxargsafe]] */

/*s: global [[mnstring]] */
int	mnstring;
/*e: global [[mnstring]] */
/*s: global [[multab]](arm) */
Multab	multab[20];
/*e: global [[multab]](arm) */
/*s: global [[hintabsize]](arm) */
int	hintabsize;
/*e: global [[hintabsize]](arm) */
/*s: global [[nodrat]] */
Node*	nodrat;
/*e: global [[nodrat]] */
/*s: global [[nodret]] */
Node*	nodret;
/*e: global [[nodret]] */
/*s: global [[nodsafe]] */
Node*	nodsafe;
/*e: global [[nodsafe]] */
/*s: global [[nrathole]] */
long	nrathole;
/*e: global [[nrathole]] */
/*s: global [[nstring]] */
long	nstring;
/*e: global [[nstring]] */
/*s: global [[p]] */
Prog*	p;
/*e: global [[p]] */
/*s: global [[pc]] */
long	pc;
/*e: global [[pc]] */
/*s: global [[regnode]] */
Node	regnode;
/*e: global [[regnode]] */
/*s: global [[string]] */
char	string[NSNAME];
/*e: global [[string]] */
/*s: global [[symrathole]] */
Sym*	symrathole;
/*e: global [[symrathole]] */
/*s: global [[znode]] */
Node	znode;
/*e: global [[znode]] */
/*s: global [[zprog]] */
Prog	zprog;
/*e: global [[zprog]] */
/*s: global [[reg]](arm) */
// map<enum<registr>, int>
char	reg[NREG+NFREG];
/*e: global [[reg]](arm) */
/*s: global [[exregoffset]] */
long	exregoffset;
/*e: global [[exregoffset]] */
/*s: global [[exfregoffset]] */
long	exfregoffset;
/*e: global [[exfregoffset]] */
/*s: global [[suppress]] */
bool	suppress;
/*e: global [[suppress]] */

/*s: global [[region]] */
Rgn	region[NRGN];
/*e: global [[region]] */
/*s: global [[rgp]] */
Rgn*	rgp;
/*e: global [[rgp]] */
/*s: global [[nregion]] */
int	nregion;
/*e: global [[nregion]] */

/*s: global [[externs]] */
Bits	externs;
/*e: global [[externs]] */
/*s: global [[params]] */
Bits	params;
/*e: global [[params]] */
/*s: global [[consts]] */
Bits	consts;
/*e: global [[consts]] */
/*s: global [[addrs]] */
Bits	addrs;
/*e: global [[addrs]] */

/*s: global [[regbits]] */
long	regbits;
/*e: global [[regbits]] */
/*s: global [[exregbits]] */
long	exregbits;
/*e: global [[exregbits]] */

/*s: global [[change]] */
int	change;
/*e: global [[change]] */

/*s: global [[firstr]] */
Reg*	firstr;
/*e: global [[firstr]] */
/*s: global [[lastr]] */
Reg*	lastr;
/*e: global [[lastr]] */
/*s: global [[zreg]] */
Reg	zreg;
/*e: global [[zreg]] */
/*s: global [[freer]] */
Reg*	freer;
/*e: global [[freer]] */
/*s: global [[var]] */
Var	var[NVAR];
/*e: global [[var]] */
/*s: global [[nvar]] */
int	nvar;
/*e: global [[nvar]] */

/*s: global [[idom]] */
long*	idom;
/*e: global [[idom]] */
/*s: global [[rpo2r]] */
Reg**	rpo2r;
/*e: global [[rpo2r]] */
/*s: global [[maxnr]] */
long	maxnr;
/*e: global [[maxnr]] */
/*e: 5c/globals2.c */
