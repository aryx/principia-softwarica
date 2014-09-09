#include "ldefs.h"

Biobuf	fout;
int	foutopen;
int	errorf = 1;
int	sect = DEFSECTION;
int	prev = '\n';	/* previous input character */
int	pres = '\n';	/* present input character */
int	peek = '\n';	/* next input character */
uchar	*pushptr = pushc;
uchar	*slptr = slist;

char	*cname = "/sys/lib/lex/ncform";

int nine;
int ccount = 1;
int casecount = 1;
int aptr = 1;
int nstates = NSTATES, maxpos = MAXPOS;
int treesize = TREESIZE, ntrans = NTRANS;
int yytop;
int outsize = NOUTPUT;
int sptr = 1;
int report = 2;
int debug = 0;		/* 1 = on */
int charc;
int sargc;
char **sargv;
uchar buf[520];
int yyline;		/* line number of file */
char *yyfile;		/* filename for error messages */
int eof;
int lgatflg;
int divflg;
int funcflag;
int pflag;
int chset;	/* 1 = char set modified */
Biobuf *fin = 0, *fother;
int fptr;
int *name;
int *left;
int *right;
int *parent;
uchar *nullstr;
uchar **ptr;
int tptr;
uchar pushc[TOKENSIZE];
uchar slist[STARTSIZE];
uchar **def, **subs, *dchar;
uchar **sname, *stchar;
uchar *ccl;
uchar *ccptr;
uchar *dp, *sp;
int dptr;
uchar *bptr;		/* store input position */
uchar *tmpstat;
int count;
int **foll;
int *nxtpos;
int *positions;
int *gotof;
int *nexts;
uchar *nchar;
int **state;
int *sfall;		/* fallback state num */
uchar *cpackflg;		/* true if state has been character packed */
int *atable;
int nptr;
uchar symbol[NCH];
uchar cindex[NCH];
int xstate;
int stnum;
uchar match[NCH];
uchar extra[NACTIONS];
uchar *pchar, *pcptr;
int pchlen = TOKENSIZE;
 long rcount;
int *verify, *advance, *stoff;
int scon;
uchar *psave;
