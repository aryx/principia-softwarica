/*s: generators/lex/lmain.c */
/* lex [-[dynvt]] [file] ... [file] */

/* Copyright 1976, Bell Telephone Laboratories, Inc.,
   written by Eric Schmidt, August 27, 1976   */

#include "ldefs.h"

static void	free1core(void);
static void	free2core(void);
static void	free3core(void);
static void	get1core(void);
static void	get2core(void);
static void	get3core(void);

#ifdef DEBUG
/*s: function SETYY */
#define SETYY() yydebug = TRUE
/*e: function SETYY */
#else
/*s: function SETYY (generators/lex/lmain.c) */
#define SETYY() nil
/*e: function SETYY (generators/lex/lmain.c) */
#endif

/*s: function main (generators/lex/lmain.c) */
void
main(int argc, char **argv)
{
    int i;

    ARGBEGIN {
        case 'd': debug++; break;
        case 'y': SETYY(); break;
        case 't': case 'T':
            Binit(&fout, 1, OWRITE);
            errorf= 2;
            foutopen = 1;
            break;
        case 'v': case 'V':
            report = 1;
            break;
        case 'n': case 'N':
            report = 0;
            break;
        case '9':
            nine = 1;
            break;
        default:
            warning("Unknown option %c", ARGC());
    } ARGEND
    sargc = argc;
    sargv = argv;
    if (argc > 0){
        yyfile = argv[fptr++];
        fin = Bopen(yyfile, OREAD);
        if(fin == 0)
            error ("%s - can't open file: %r", yyfile);
        sargc--;
        sargv++;
    }
    else {
        yyfile = "/fd/0";
        fin = myalloc(sizeof(Biobuf), 1);
        if(fin == 0)
            exits("core");
        Binit(fin, 0, OREAD);
    }
    if(Bgetc(fin) == Beof)		/* no input */
        exits(0);
    Bseek(fin, 0, 0);
    gch();
        /* may be gotten: def, subs, sname, stchar, ccl, dchar */
    get1core();
        /* may be gotten: name, left, right, nullstr, parent, ptr */
    strcpy((char*)sp, "INITIAL");
    sname[0] = sp;
    sp += strlen("INITIAL") + 1;
    sname[1] = 0;
    if(yyparse()) exits("error");	/* error return code */
        /* may be disposed of: def, subs, dchar */
    free1core();
        /* may be gotten: tmpstat, foll, positions, gotof, nexts, nchar, state, atable, sfall, cpackflg */
    get2core();
    ptail();
    mkmatch();
    if(debug) pccl();
    sect  = ENDSECTION;
    if(tptr>0)cfoll(tptr-1);
    if(debug)pfoll();
    cgoto();
    if(debug){
        print("Print %d states:\n",stnum+1);
        for(i=0;i<=stnum;i++)stprt(i);
        }
        /* may be disposed of: positions, tmpstat, foll, state, name, left, right, parent, ccl, stchar, sname */
        /* may be gotten: verify, advance, stoff */
    free2core();
    get3core();
    layout();
        /* may be disposed of: verify, advance, stoff, nexts, nchar,
            gotof, atable, ccpackflg, sfall */
    // was in #ifdef DEBUG before with no if(debug), so ok like this?
    if(debug) free3core();
    fother = Bopen(cname,OREAD);
    if(fother == 0)
        error("Lex driver missing, file %s: %r",cname);
    while ( (i=Bgetc(fother)) != Beof)
        Bputc(&fout, i);

    Bterm(fother);
    Bterm(&fout);
    if(debug ||	report == 1)
        statistics();
    if (fin)
        Bterm(fin);
    exits(0);	/* success return code */
}
/*e: function main (generators/lex/lmain.c) */

/*s: function get1core */
static void
get1core(void)
{
    ccptr =	ccl = myalloc(CCLSIZE,sizeof(*ccl));
    pcptr = pchar = myalloc(pchlen, sizeof(*pchar));
    def = myalloc(DEFSIZE,sizeof(*def));
    subs = myalloc(DEFSIZE,sizeof(*subs));
    dp = dchar = myalloc(DEFCHAR,sizeof(*dchar));
    sname = myalloc(STARTSIZE,sizeof(*sname));
    sp = stchar = myalloc(STARTCHAR,sizeof(*stchar));
    if(ccl == 0 || def == 0 || subs == 0 || dchar == 0 || sname == 0 || stchar == 0)
        error("Too little core to begin");
}
/*e: function get1core */

/*s: function free1core */
static void
free1core(void)
{
    free(def);
    free(subs);
    free(dchar);
}
/*e: function free1core */

/*s: function get2core */
static void
get2core(void)
{
    int i;

    gotof = myalloc(nstates,sizeof(*gotof));
    nexts = myalloc(ntrans,sizeof(*nexts));
    nchar = myalloc(ntrans,sizeof(*nchar));
    state = myalloc(nstates,sizeof(*state));
    atable = myalloc(nstates,sizeof(*atable));
    sfall = myalloc(nstates,sizeof(*sfall));
    cpackflg = myalloc(nstates,sizeof(*cpackflg));
    tmpstat = myalloc(tptr+1,sizeof(*tmpstat));
    foll = myalloc(tptr+1,sizeof(*foll));
    nxtpos = positions = myalloc(maxpos,sizeof(*positions));
    if(tmpstat == 0 || foll == 0 || positions == 0 ||
        gotof == 0 || nexts == 0 || nchar == 0 || state == 0 || atable == 0 || sfall == 0 || cpackflg == 0 )
        error("Too little core for state generation");
    for(i=0;i<=tptr;i++)foll[i] = 0;
}
/*e: function get2core */

/*s: function free2core */
static void
free2core(void)
{
    free(positions);
    free(tmpstat);
    free(foll);
    free(name);
    free(left);
    free(right);
    free(parent);
    free(nullstr);
    free(ptr);
    free(state);
    free(sname);
    free(stchar);
    free(ccl);
}
/*e: function free2core */

/*s: function get3core */
static void
get3core(void)
{
    verify = myalloc(outsize,sizeof(*verify));
    advance = myalloc(outsize,sizeof(*advance));
    stoff = myalloc(stnum+2,sizeof(*stoff));
    if(verify == 0 || advance == 0 || stoff == 0)
        error("Too little core for final packing");
}
/*e: function get3core */

/*s: function free3core */
static void
free3core(void){
    free(advance);
    free(verify);
    free(stoff);
    free(gotof);
    free(nexts);
    free(nchar);
    free(atable);
    free(sfall);
    free(cpackflg);
}
/*e: function free3core */

/*s: function myalloc */
void *
myalloc(int a, int b)
{
    void *i;
    i = calloc(a, b);
    if(i==0)
        warning("OOPS - calloc returns a 0");
    return(i);
}
/*e: function myalloc */

/*s: function yyerror */
void
yyerror(char *s)
{
    fprint(2, "%s:%d %s\n", yyfile, yyline, s);
}
/*e: function yyerror */
/*e: generators/lex/lmain.c */
