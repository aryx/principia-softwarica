#include "l.h"

union Buf buf;

long	HEADR;			/* length of header */
int	HEADTYPE;		/* type of header */
vlong	INITDAT;		/* data location */
vlong	INITRND;		/* data round above text location */
vlong	INITTEXT;		/* text location */
vlong	INITTEXTP;		/* text location (physical) */
char*	INITENTRY;		/* entry point */
long	autosize;
Biobuf	bso;
long	bsssize;
int	cbc;
uchar*	cbp;
int	cout;
Auto*	curauto;
Auto*	curhist;
Prog*	curp;
Prog*	curtext;
Prog*	datap;
long	datsize;
char	debug[128];
Prog*	etextp;
Prog*	firstp;
char	fnuxi4[4];	/* for 3l [sic] */
char	fnuxi8[8];
char*	noname;
Sym*	hash[NHASH];
Sym*	histfrog[MAXHIST];
int	histfrogp;
int	histgen;
char*	library[50];
char*	libraryobj[50];
int	libraryp;
int	xrefresolv;
char*	hunk;
char	inuxi1[1];
char	inuxi2[2];
char	inuxi4[4];
Prog*	lastp;
long	lcsize;
char	literal[32];
int	nerrors;
long	nhunk;
long	instoffset;
Opcross	opcross[10];
Oprang	oprange[ALAST];
char*	outfile;
vlong	pc;
uchar	repop[ALAST];
long	symsize;
Prog*	textp;
vlong	textsize;
long	thunk;
int	version;
char	xcmp[32][32];
Prog	zprg;
int	dtype;
int	little;

struct Nop nop;
