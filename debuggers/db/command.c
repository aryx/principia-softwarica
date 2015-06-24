/*s: db/command.c */
/*
 *
 *	debugger
 *
 */

#include "defs.h"
#include "fns.h"

/*s: global BADEQ */
char	BADEQ[] = "unexpected `='";
/*e: global BADEQ */

/*s: global executing */
bool	executing;
/*e: global executing */
extern	Rune	*lp;

/*s: global eqformat */
char	eqformat[ARB] = "z";
/*e: global eqformat */
/*s: global stformat */
char	stformat[ARB] = "zMi";
/*e: global stformat */


/*s: global loopcnt */
WORD loopcnt;
/*e: global loopcnt */

/*s: function command */
/* command decoding */
int
command(char *buf, int defcom)
{
    /*s: [[command()]] locals (db) */
    static char lastcom = '=';
    /*x: [[command()]] locals (db) */
    Rune*   savlp = lp;
    char	savlc = lastc;
    char	savpc = peekc;
    /*x: [[command()]] locals (db) */
    char	*reg;
    char	savc;
    static char savecom = '=';
    /*e: [[command()]] locals (db) */

    /*s: [[command()]] initializations (db) */
    if (defcom == 0)
        defcom = lastcom;
    if (buf) {
        if (*buf==EOR)
            return FALSE;
        clrinp();
        lp=(Rune*)buf;
    }
    /*e: [[command()]] initializations (db) */

    do {
        /*s: [[command()]] parse possibly first address, set dot */
        adrflg=expr(0);		/* first address */
        if (adrflg){
            dot=expv;
            ditto=expv;
        }
        adrval=dot;
        /*e: [[command()]] parse possibly first address, set dot */
        /*s: [[command()]] parse possibly count, set cntval */
        if (rdc()==',' && expr(0)) {	/* count */
            cntflg=TRUE;
            cntval=expv;
        } else {
            cntflg=FALSE;
            cntval=1;
            reread();
        }
        /*e: [[command()]] parse possibly count, set cntval */

        /*s: [[command()]] parse command, set lastcom */
        if (!eol(rdc()))
            lastcom=lastc;		/* command */
        else {
            if (adrflg==false)
                dot=inkdot(dotinc);
            reread();
            lastcom=defcom;
        }
        /*e: [[command()]] parse command, set lastcom */
        switch(lastcom) {
        /*s: [[command()]] switch lastcom cases */
        case '?':
        case '/':
        case '=':
            savecom = lastcom;
            acommand(lastcom);
            break;
        /*x: [[command()]] switch lastcom cases */
        case '$':
            lastcom=savecom;
            printtrace(nextchar()); 
            break;
        /*x: [[command()]] switch lastcom cases */
        case ':':
            if (!executing) { 
                executing=TRUE;
                subpcs(nextchar());
                executing=FALSE;
                lastcom=savecom;
            }
            break;
        /*x: [[command()]] switch lastcom cases */
        case '>':
            lastcom = savecom; 
            savc=rdc();
            if (reg=regname(savc))
                rput(cormap, reg, dot);
            else	
                error("bad variable");
            break;
        /*x: [[command()]] switch lastcom cases */
        case '!':
            lastcom=savecom;
            shell(); 
            break;
        /*x: [[command()]] switch lastcom cases */
        case '\0':
            prints(DBNAME);
            break;

        /*e: [[command()]] switch lastcom cases */
        default: 
            error("bad command");
        }
        flushbuf();
    } while (rdc()==';');

    /*s: [[command()]] finalizations (db) */
    if (buf == nil)
        reread();
    else {
        clrinp();
        lp=savlp;
        lastc = savlc;
        peekc = savpc;
    }

    if(adrflg)
        return dot;
    return 1;
    /*e: [[command()]] finalizations (db) */
}
/*e: function command */

/*s: function acommand */
/*
 * [/?][wml]
 */
void
acommand(int pc)
{
    bool eqcom;
    Map *map;
    char *fmt;
    char buf[512];

    if (pc == '=') {
        eqcom = true;
        fmt = eqformat;
        map = dotmap;
    } else {
        eqcom = false;
        fmt = stformat;
        if (pc == '/')
            map = cormap;
        else
            map = symmap;
    }
    if (!map) {
        snprint(buf, sizeof(buf), "no map for %c", pc);
        error(buf);
    }

    switch (rdc()) {
    /*s: [[acommand()]] switch optional command suffix character */
    case 'm':
        if (eqcom)
            error(BADEQ); 
        cmdmap(map);
        break;
    /*x: [[acommand()]] switch optional command suffix character */
    case 'L':
    case 'l':
        if (eqcom)
            error(BADEQ); 
        cmdsrc(lastc, map);
        break;

    /*x: [[acommand()]] switch optional command suffix character */
    case 'W':
    case 'w':
        if (eqcom)
            error(BADEQ); 
        cmdwrite(lastc, map);
        break;
    /*e: [[acommand()]] switch optional command suffix character */
    default:
        reread();
        getformat(fmt);
        scanform(cntval, !eqcom, fmt, map, eqcom);
    }
}
/*e: function acommand */

/*s: function cmdsrc */
void
cmdsrc(int c, Map *map)
{
    ulong w;
    long locval, locmsk;
    ADDR savdot;
    ushort sh;
    char buf[512];
    int ret;

    if (c == 'L')
        dotinc = 4;
    else
        dotinc = 2;
    savdot=dot;
    expr(1); 
    locval=expv;
    if (expr(0))
        locmsk=expv; 
    else
        locmsk = ~0;
    if (c == 'L')
        while ((ret = get4(map, dot, &w)) > 0 &&  (w&locmsk) != locval)
            dot = inkdot(dotinc);
    else
        while ((ret = get2(map, dot, &sh)) > 0 && (sh&locmsk) != locval)
            dot = inkdot(dotinc);
    if (ret < 0) { 
        dot=savdot; 
        error("%r");
    }
    symoff(buf, 512, dot, CANY);
    dprint(buf);
}
/*e: function cmdsrc */

/*s: global badwrite */
static char badwrite[] = "can't write process memory or text image";
/*e: global badwrite */

/*s: function cmdwrite */
void
cmdwrite(int wcom, Map *map)
{
    ADDR savdot;
    char *format;
    int pass;

    if (wcom == 'w')
        format = "x";
    else
        format = "X";
    expr(1);
    pass = 0;
    do {
        pass++;  
        savdot=dot;
        exform(1, 1, format, map, 0, pass);
        dot=savdot;
        if (wcom == 'W') {
            if (put4(map, dot, expv) <= 0)
                error(badwrite);
        } else {
            if (put2(map, dot, expv) <= 0)
                error(badwrite);
        }
        savdot=dot;
        dprint("=%8t"); 
        exform(1, 0, format, map, 0, pass);
        newline();
    } while (expr(0));
    dot=savdot;
}
/*e: function cmdwrite */

/*s: function regname */
/*
 * collect a register name; return register offset
 * this is not what i'd call a good division of labour
 */

char *
regname(int regnam)
{
    static char buf[64];
    char *p;
    int c;

    p = buf;
    *p++ = regnam;
    while (isalnum(c = readchar())) {
        if (p >= buf+sizeof(buf)-1)
            error("register name too long");
        *p++ = c;
    }
    *p = 0;
    reread();
    return (buf);
}
/*e: function regname */

/*s: function shell */
/*
 * shell escape
 */

void
shell(void)
{
    int	rc, unixpid;
    char *argp = (char*)lp;

    while (lastc!=EOR)
        rdc();
    if ((unixpid=fork())==0) {
        *lp=0;
        execl("/bin/rc", "rc", "-c", argp, nil);
        exits("execl");				/* botch */
    } else if (unixpid == -1) {
        error("cannot fork");
    } else {
        mkfault = 0;
        while ((rc = waitpid()) != unixpid){
            if(rc == -1 && mkfault){
                mkfault = 0;
                continue;
            }
            break;
        }
        prints("!"); 
        reread();
    }
}
/*e: function shell */
/*e: db/command.c */
