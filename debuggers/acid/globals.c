/*s: acid/globals.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>
#include "acid.h"

/*s: global kernel */
int	kernel;
/*e: global kernel */
/*s: global remote */
int	remote;
/*e: global remote */
/*s: global text */
int	text;
/*e: global text */
/*s: global silent */
int	silent;
/*e: global silent */
/*s: global fhdr (acid/globals.c) */
Fhdr	fhdr;
/*e: global fhdr (acid/globals.c) */
/*s: global line (acid/globals.c) */
int	line;
/*e: global line (acid/globals.c) */
/*s: global bout */
Biobuf*	bout;
/*e: global bout */
/*s: global io */
Biobuf*	io[32];
/*e: global io */
/*s: global iop */
int	iop;
/*e: global iop */
/*s: global symbol */
char	symbol[Strsize];
/*e: global symbol */
/*s: global interactive (acid/globals.c) */
int	interactive;
/*e: global interactive (acid/globals.c) */
/*s: global na */
int	na;
/*e: global na */
/*s: global wtflag (acid/globals.c) */
int	wtflag;
/*e: global wtflag (acid/globals.c) */
/*s: global cormap (acid/globals.c) */
Map*	cormap;
/*e: global cormap (acid/globals.c) */
/*s: global symmap (acid/globals.c) */
Map*	symmap;
/*e: global symmap (acid/globals.c) */
/*s: global hash */
Lsym*	hash[Hashsize];
/*e: global hash */
/*s: global dogc */
long	dogc;
/*e: global dogc */
/*s: global ret */
Rplace*	ret;
/*e: global ret */
/*s: global aout */
char*	aout;
/*e: global aout */
/*s: global gotint */
int	gotint;
/*e: global gotint */
/*s: global gcl */
Gc*	gcl;
/*e: global gcl */
/*s: global stacked */
int	stacked;
/*e: global stacked */
/*s: global err */
jmp_buf	err;
/*e: global err */
/*s: global prnt */
Node*	prnt;
/*e: global prnt */
/*s: global tracelist */
List*	tracelist;
/*e: global tracelist */
/*s: global initialising */
int	initialising;
/*e: global initialising */
/*s: global quiet */
int	quiet;
/*e: global quiet */

/*s: global ptab */
Ptab	ptab[Maxproc];
/*e: global ptab */
/*e: acid/globals.c */
