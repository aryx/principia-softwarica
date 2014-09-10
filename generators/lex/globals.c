/*s: generators/lex/globals.c */
#include "ldefs.h"

/*s: global fout */
Biobuf	fout;
/*e: global fout */
/*s: global foutopen */
int	foutopen;
/*e: global foutopen */
/*s: global errorf */
int	errorf = 1;
/*e: global errorf */
/*s: global sect */
int	sect = DEFSECTION;
/*e: global sect */
/*s: global prev */
int	prev = '\n';	/* previous input character */
/*e: global prev */
/*s: global pres */
int	pres = '\n';	/* present input character */
/*e: global pres */
/*s: global peek */
int	peek = '\n';	/* next input character */
/*e: global peek */
/*s: global pushptr */
uchar	*pushptr = pushc;
/*e: global pushptr */
/*s: global slptr */
uchar	*slptr = slist;
/*e: global slptr */

/*s: global cname */
char	*cname = "/sys/lib/lex/ncform";
/*e: global cname */

/*s: global nine */
int nine;
/*e: global nine */
/*s: global ccount */
int ccount = 1;
/*e: global ccount */
/*s: global casecount */
int casecount = 1;
/*e: global casecount */
/*s: global aptr */
int aptr = 1;
/*e: global aptr */
int nstates = NSTATES, maxpos = MAXPOS;
int treesize = TREESIZE, ntrans = NTRANS;
/*s: global yytop */
int yytop;
/*e: global yytop */
/*s: global outsize */
int outsize = NOUTPUT;
/*e: global outsize */
/*s: global sptr */
int sptr = 1;
/*e: global sptr */
/*s: global report */
int report = 2;
/*e: global report */
/*s: global debug */
int debug = 0;		/* 1 = on */
/*e: global debug */
/*s: global charc */
int charc;
/*e: global charc */
/*s: global sargc */
int sargc;
/*e: global sargc */
/*s: global sargv */
char **sargv;
/*e: global sargv */
/*s: global buf */
uchar buf[520];
/*e: global buf */
/*s: global yyline */
int yyline;		/* line number of file */
/*e: global yyline */
/*s: global yyfile */
char *yyfile;		/* filename for error messages */
/*e: global yyfile */
/*s: global eof */
int eof;
/*e: global eof */
/*s: global lgatflg */
int lgatflg;
/*e: global lgatflg */
/*s: global divflg */
int divflg;
/*e: global divflg */
/*s: global funcflag */
int funcflag;
/*e: global funcflag */
/*s: global pflag */
int pflag;
/*e: global pflag */
/*s: global chset */
int chset;	/* 1 = char set modified */
/*e: global chset */
Biobuf *fin = 0, *fother;
/*s: global fptr */
int fptr;
/*e: global fptr */
/*s: global name */
int *name;
/*e: global name */
/*s: global left */
int *left;
/*e: global left */
/*s: global right */
int *right;
/*e: global right */
/*s: global parent */
int *parent;
/*e: global parent */
/*s: global nullstr */
uchar *nullstr;
/*e: global nullstr */
/*s: global ptr */
uchar **ptr;
/*e: global ptr */
/*s: global tptr */
int tptr;
/*e: global tptr */
/*s: global pushc */
uchar pushc[TOKENSIZE];
/*e: global pushc */
/*s: global slist */
uchar slist[STARTSIZE];
/*e: global slist */
uchar **def, **subs, *dchar;
uchar **sname, *stchar;
/*s: global ccl */
uchar *ccl;
/*e: global ccl */
/*s: global ccptr */
uchar *ccptr;
/*e: global ccptr */
uchar *dp, *sp;
/*s: global dptr */
int dptr;
/*e: global dptr */
/*s: global bptr */
uchar *bptr;		/* store input position */
/*e: global bptr */
/*s: global tmpstat */
uchar *tmpstat;
/*e: global tmpstat */
/*s: global count */
int count;
/*e: global count */
/*s: global foll */
int **foll;
/*e: global foll */
/*s: global nxtpos */
int *nxtpos;
/*e: global nxtpos */
/*s: global positions */
int *positions;
/*e: global positions */
/*s: global gotof */
int *gotof;
/*e: global gotof */
/*s: global nexts */
int *nexts;
/*e: global nexts */
/*s: global nchar */
uchar *nchar;
/*e: global nchar */
/*s: global state */
int **state;
/*e: global state */
/*s: global sfall */
int *sfall;		/* fallback state num */
/*e: global sfall */
/*s: global cpackflg */
uchar *cpackflg;		/* true if state has been character packed */
/*e: global cpackflg */
/*s: global atable */
int *atable;
/*e: global atable */
/*s: global nptr */
int nptr;
/*e: global nptr */
/*s: global symbol */
uchar symbol[NCH];
/*e: global symbol */
/*s: global cindex */
uchar cindex[NCH];
/*e: global cindex */
/*s: global xstate */
int xstate;
/*e: global xstate */
/*s: global stnum */
int stnum;
/*e: global stnum */
/*s: global match */
uchar match[NCH];
/*e: global match */
/*s: global extra */
uchar extra[NACTIONS];
/*e: global extra */
uchar *pchar, *pcptr;
/*s: global pchlen */
int pchlen = TOKENSIZE;
/*e: global pchlen */
/*s: global rcount */
 long rcount;
/*e: global rcount */
int *verify, *advance, *stoff;
/*s: global scon */
int scon;
/*e: global scon */
/*s: global psave */
uchar *psave;
/*e: global psave */
/*e: generators/lex/globals.c */
