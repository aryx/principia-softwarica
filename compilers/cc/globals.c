/*s: cc/globals.c */
#include "cc.h"

/*s: global fi */
struct Fi fi;
/*e: global fi */
/*s: global hist */
Hist*	hist;
/*e: global hist */
/*s: global en */
struct En en;
/*e: global en */

/*s: global autobn */
int	autobn;
/*e: global autobn */
/*s: global autoffset */
long	autoffset;
/*e: global autoffset */
/*s: global blockno */
int	blockno;
/*e: global blockno */
/*s: global dclstack */
Decl*	dclstack;
/*e: global dclstack */
/*s: global debug */
char	debug[256];
/*e: global debug */
/*s: global ehist */
Hist*	ehist;
/*e: global ehist */
/*s: global firstbit */
long	firstbit;
/*e: global firstbit */
/*s: global firstarg */
Sym*	firstarg;
/*e: global firstarg */
/*s: global firstargtype */
Type*	firstargtype;
/*e: global firstargtype */
/*s: global firstdcl */
Decl*	firstdcl;
/*e: global firstdcl */
/*s: global fperror */
//int	fperror;
/*e: global fperror */
/*s: global hash */
Sym*	hash[NHASH];
/*e: global hash */
/*s: global hasdoubled */
int	hasdoubled;
/*e: global hasdoubled */
/*s: global hunk */
char*	hunk;
/*e: global hunk */
/*s: global include */
char**	include;
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
/*s: global lastbit */
long	lastbit;
/*e: global lastbit */
/*s: global lastclass */
//@Scheck: y.tab.c
char	lastclass;
/*e: global lastclass */
/*s: global lastdcl */
Type*	lastdcl;
/*e: global lastdcl */
/*s: global lastfield */
long	lastfield;
/*e: global lastfield */
/*s: global lasttype */
//@Scheck: y.tab.c
Type*	lasttype;
/*e: global lasttype */
/*s: global lineno */
long	lineno;
/*e: global lineno */
/*s: global nearln */
long	nearln;
/*e: global nearln */
/*s: global maxinclude */
int	maxinclude;
/*e: global maxinclude */
/*s: global nerrors */
int	nerrors;
/*e: global nerrors */
/*s: global newflag */
int	newflag;
/*e: global newflag */
/*s: global nhunk */
long	nhunk;
/*e: global nhunk */
/*s: global ninclude */
int	ninclude;
/*e: global ninclude */
/*s: global nodproto */
Node*	nodproto;
/*e: global nodproto */
/*s: global nodcast */
Node*	nodcast;
/*e: global nodcast */
/*s: global outbuf */
Biobuf	outbuf;
/*e: global outbuf */
/*s: global diagbuf */
Biobuf	diagbuf;
/*e: global diagbuf */
/*s: global outfile */
char*	outfile;
/*e: global outfile */
/*s: global pathname */
char*	pathname;
/*e: global pathname */
/*s: global peekc */
int	peekc;
/*e: global peekc */
/*s: global stkoff */
long	stkoff;
/*e: global stkoff */
/*s: global strf */
Type*	strf;
/*e: global strf */
/*s: global strl */
Type*	strl;
/*e: global strl */
/*s: global symb */
char	symb[NSYMB];
/*e: global symb */
/*s: global symstring */
Sym*	symstring;
/*e: global symstring */
/*s: global taggen */
//@Scheck: y.tab.c
int	taggen;
/*e: global taggen */
/*s: global tfield */
Type*	tfield;
/*e: global tfield */
/*s: global tufield */
Type*	tufield;
/*e: global tufield */
/*s: global thechar */
int	thechar;
/*e: global thechar */
/*s: global thestring */
char*	thestring;
/*e: global thestring */
/*s: global thisfn */
Type*	thisfn;
/*e: global thisfn */
/*s: global thunk */
long	thunk;
/*e: global thunk */
/*s: global types */
Type*	types[NTYPE];
/*e: global types */
/*s: global fntypes */
Type*	fntypes[NTYPE];
/*e: global fntypes */
/*s: global initlist */
Node*	initlist;
/*e: global initlist */
/*s: global term */
Term	term[NTERM];
/*e: global term */
/*s: global nterm */
int	nterm;
/*e: global nterm */
/*s: global packflg */
int	packflg;
/*e: global packflg */
/*s: global fproundflg */
int	fproundflg;
/*e: global fproundflg */
/*s: global profileflg */
int	profileflg;
/*e: global profileflg */
/*s: global ncontin */
int	ncontin;
/*e: global ncontin */
/*s: global newvlongcode */
int	newvlongcode;
/*e: global newvlongcode */
/*s: global canreach */
int	canreach;
/*e: global canreach */
/*s: global warnreach */
int	warnreach;
/*e: global warnreach */
/*s: global zbits */
Bits	zbits;
/*e: global zbits */

/*s: global typeswitch */
char*	typeswitch;
/*e: global typeswitch */
/*s: global typeword */
char*	typeword;
/*e: global typeword */
/*s: global typecmplx */
char*	typecmplx;
/*e: global typecmplx */
/*e: cc/globals.c */
