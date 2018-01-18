/*s: machine/5i/cmd.c */
/*s: basic includes */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "arm.h"
/*e: basic includes */

#include <ctype.h>

/*s: global [[fmt]] */
char	fmt = 'X';
/*e: global [[fmt]] */
/*s: global [[width]] */
int	width = 60;
/*e: global [[width]] */
/*s: global [[inc]] */
int	inc;
/*e: global [[inc]] */

/*s: function [[reset]] */
void
reset(void)
{
    int i, l, m;
    Segment *s;
    Breakpoint *b;

    memset(&reg, 0, sizeof(Registers));

    for(i = 0; i > Nseg; i++) {
        s = &memory.seg[i];
        l = ((s->end-s->base)/BY2PG)*sizeof(byte*);
        for(m = 0; m < l; m++)
            if(s->table[m])
                free(s->table[m]);
        free(s->table);
    }
    free(iprof);
    memset(&memory, 0, sizeof(memory));

    for(b = bplist; b; b = b->next)
        b->done = b->count;
}
/*e: function [[reset]] */

/*s: function [[nextc]] */
char*
nextc(char *p)
{
    while(*p && (*p == ' ' || *p == '\t') && *p != '\n')
        p++;

    if(*p == '\n')
        *p = '\0';

    return p;
}
/*e: function [[nextc]] */

/*s: function [[numsym]] */
char*
numsym(char *addr, ulong *val)
{
    char tsym[128], *t;
    static char *delim = "`'<>/\\@*|-~+-/=?\n";
    Symbol s;
    char c;

    t = tsym;
    while(c = *addr) {
        if(strchr(delim, c))
            break;
        *t++ = c;
        addr++;
    }
    t[0] = '\0';

    if(strcmp(tsym, ".") == 0) {
        *val = dot;
        return addr;
    }

    if(lookup(0, tsym, &s))
        *val = s.value;
    else {
        if(tsym[0] == '#')
            *val = strtoul(tsym+1, 0, 16);
        else
            *val = strtoul(tsym, 0, 0);
    }
    return addr;
}
/*e: function [[numsym]] */

/*s: function [[expr]] */
ulong
expr(char *addr)
{
    ulong t, t2;
    char op;

    if(*addr == '\0')
        return dot;

    addr = numsym(addr, &t);

    if(*addr == '\0')
        return t;

    addr = nextc(addr);
    op = *addr++;
    numsym(addr, &t2);
    switch(op) {
    default:
        Bprint(bout, "expr syntax\n");
        return 0;
    case '+':
        t += t2;
        break;
    case '-':
        t -= t2;
        break;
    case '%':
        t /= t2;
        break;
    case '&':
        t &= t2;
        break;
    case '|':
        t |= t2;
        break;
    }

    return t;
}
/*e: function [[expr]] */

/*s: function [[buildargv]] */
int
buildargv(char *str, char **args, int max)
{
    int na = 0;

    while (na < max) {
        while((*str == ' ' || *str == '\t' || *str == '\n') && *str != '\0')
            str++;

        if(*str == '\0')
            return na;

        args[na++] = str;
        while(!(*str == ' ' || *str == '\t'|| *str == '\n') && *str != '\0')
            str++;

        if(*str == '\n')
            *str = '\0';

        if(*str == '\0')
            break;

        *str++ = '\0';
    }
    return na;
}
/*e: function [[buildargv]] */

/*s: function [[colon]] */
void
colon(char *addr, char *cp)
{
    /*s: [[colon()]] locals */
    char tbuf[512];
    /*x: [[colon()]] locals */
    int argc;
    char *argv[100];
    /*e: [[colon()]] locals */

    cp = nextc(cp);

    switch(*cp) {
    /*s: [[colon()]] command which return cases */
    case 'b':
        breakpoint(addr, cp+1);
        return;
    /*x: [[colon()]] command which return cases */
    case 'd':
        delbpt(addr);
        return;
    /*e: [[colon()]] command which return cases */
    /* These fall through to print the stopped address */
    /*s: [[colon()]] command cases */
    case 'c':
        count = 0;
        atbpt = false;
        run();
        break;
    /*x: [[colon()]] command cases */
    case 'r':
        reset();
        argc = buildargv(cp+1, argv, 100);
        initstk(argc, argv);
        count = 0;
        atbpt = false;
        run();
        break;
    /*x: [[colon()]] command cases */
    case 's':
        cp = nextc(cp+1);
        count = 0;
        if(*cp)
            count = strtoul(cp, 0, 0);
        if(count == 0)
            count = 1;
        atbpt = false;
        run();
        break;
    /*e: [[colon()]] command cases */
    default:
        Bprint(bout, "?\n");
        return;
    }

    dot = reg.r[REGPC];

    Bprint(bout, "%s at #%lux ", atbpt? "breakpoint": "stopped", dot);
    /*s: [[colon()]] print current instruction */
    symoff(tbuf, sizeof(tbuf), dot, CTEXT);
    Bprint(bout, tbuf);
    if(fmt == 'z')
        printsource(dot);
    /*e: [[colon()]] print current instruction */
    Bprint(bout, "\n");
}
/*e: function [[colon]] */

/*s: function [[dollar]] */
void
dollar(char *cp)
{
    cp = nextc(cp);

    switch(*cp) {
    case 'c':
        stktrace(*cp);
        break;

    case 'C':
        stktrace(*cp);
        break;
        
    case 'b':
        dobplist();
        break;

    case 'r':
        dumpreg();
        break;

    case 'R':
        dumpreg();

    case 'f':
        dumpfreg();
        break;

    case 'F':
        dumpdreg();
        break;

    case 'q':
        exits(0);
        break;

    case 'Q':
        isum();
        tlbsum();
        segsum();
        break;

    case 't':
        cp++;
        switch(*cp) {
        case '\0':
            trace = true;
            break;
        case '0':
            trace = false;
            sysdbg = false;
            calltree = false;
            break;
        case 's':
            sysdbg = true;
            break;
        case 'i':
            trace = true;
            break;
        /*s: [[dollar()]] t cases */
        case 'c':
            calltree = true;
            break;
        /*e: [[dollar()]] t cases */
        default:
            Bprint(bout, "$t[0sic]\n"); //$
            break;
        }
        break;

    case 'i':
        cp++;
        switch(*cp) {
        default:
            Bprint(bout, "$i[itsa]\n"); //$
            break;
        case 'i':
            isum();
            break;
        case 't':
            tlbsum();
            break;
        case 's':
            segsum();
            break;
        case 'a':
            isum();
            tlbsum();
            segsum();
            iprofile();
            break;
        case 'p':
            iprofile();
            break;
        }
    default:
        Bprint(bout, "?\n");
        break;

    }
}
/*e: function [[dollar]] */

/*s: function [[pfmt]] */
int
pfmt(char fmt, int mem, ulong val)
{
    int c, i;
    Symbol s;
    char *p, ch, str[1024];

    c = 0;
    switch(fmt) {
    case 'o':
        c = Bprint(bout, "%-4lo ", mem? (ushort)getmem_2(dot): val);
        inc = 2;
        break;

    case 'O':
        c = Bprint(bout, "%-8lo ", mem? getmem_4(dot): val);
        inc = 4;
        break;

    case 'q':
        c = Bprint(bout, "%-4lo ", mem? (short)getmem_2(dot): val);
        inc = 2;
        break;

    case 'Q':
        c = Bprint(bout, "%-8lo ", mem? (long)getmem_4(dot): val);
        inc = 4;
        break;

    case 'd':
        c = Bprint(bout, "%-5ld ", mem? (short)getmem_2(dot): val);
        inc = 2;
        break;


    case 'D':
        c = Bprint(bout, "%-8ld ", mem? (long)getmem_4(dot): val);
        inc = 4;
        break;

    case 'x':
        c = Bprint(bout, "#%-4lux ", mem? (long)getmem_2(dot): val);
        inc = 2;
        break;

    case 'X':
        c = Bprint(bout, "#%-8lux ", mem? (long)getmem_4(dot): val);
        inc = 4;
        break;

    case 'u':
        c = Bprint(bout, "%-5ld ", mem? (ushort)getmem_2(dot): val);
        inc = 2;
        break;

    case 'U':
        c = Bprint(bout, "%-8ld ", mem? (ulong)getmem_4(dot): val);
        inc = 4;
        break;

    case 'b':
        c = Bprint(bout, "%-3ld ", mem? getmem_b(dot): val);
        inc = 1;
        break;

    case 'c':
        c = Bprint(bout, "%c ", (int)(mem? getmem_b(dot): val));
        inc = 1;
        break;

    case 'C':
        ch = mem? getmem_b(dot): val;
        if(isprint(ch))
            c = Bprint(bout, "%c ", ch);
        else
            c = Bprint(bout, "\\x%.2x ", ch);
        inc = 1;
        break;

    case 's':
        i = 0;
        while(ch = getmem_b(dot+i))
            str[i++] = ch;
        str[i] = '\0';
        dot += i;
        c = Bprint(bout, "%s", str);
        inc = 0;
        break;

    case 'S':
        i = 0;
        while(ch = getmem_b(dot+i))
            str[i++] = ch;
        str[i] = '\0';
        dot += i;
        for(p = str; *p; p++)
            if(isprint(*p))
                c += Bprint(bout, "%c", *p);
            else
                c += Bprint(bout, "\\x%.2ux", *p);
        inc = 0;
        break;

    case 'Y':
        p = ctime(mem? getmem_b(dot): val);
        p[30] = '\0';
        c = Bprint(bout, "%s", p);
        inc = 4;
        break;

    case 'a':
        symoff(str, sizeof(str), dot, CTEXT);
        c = Bprint(bout, str);
        inc = 0;
        break;

    case 'e':
        for(i = 0; globalsym(&s, i); i++)
            Bprint(bout, "%-15s #%lux\n", s.name,	getmem_4(s.value));
        inc = 0;
        break;

    case 'I':
    case 'i':
        inc = machdata->das(symmap, dot, fmt, str, sizeof(str));
        if(inc < 0) {
            Bprint(bout, "5i: %r\n");
            return 0;
        }
        c = Bprint(bout, "\t%s", str);
        break;

    case 'n':
        c = width+1;
        inc = 0;
        break;

    case '-':
        c = 0;
        inc = -1;
        break;

    case '+':
        c = 0;
        inc = 1;
        break;

    case '^':
        c = 0;
        if(inc > 0)
            inc = -inc;
        break;

    case 'z':
        if(findsym(dot, CTEXT, &s))
            Bprint(bout, "  %s() ", s.name);
        printsource(dot);
        inc = 0;
        break;

    default:
        Bprint(bout, "bad modifier\n");
        return 0;
    }
    return c;
}
/*e: function [[pfmt]] */

/*s: function [[eval]] */
void
eval(char *addr, char *p)
{
    ulong val;

    val = expr(addr);
    p = nextc(p);
    if(*p == '\0') {
        p[0] = fmt;
        p[1] = '\0';
    }
    pfmt(*p, 0, val);
    Bprint(bout, "\n");
}
/*e: function [[eval]] */

/*s: function [[quesie]] */
void
quesie(char *p)
{
    int c, count, i;
    char tbuf[512];

    c = 0;
    symoff(tbuf, sizeof(tbuf), dot, CTEXT);
    Bprint(bout, "%s?\t", tbuf);

    while(*p) {
        p = nextc(p);
        if(*p == '"') {
            for(p++; *p && *p != '"'; p++) {
                Bputc(bout, *p);
                c++;
            }
            if(*p)
                p++;
            continue;
        }
        count = 0;
        while(*p >= '0' && *p <= '9')
            count = count*10 + (*p++ - '0');
        if(count == 0)
            count = 1;
        p = nextc(p);
        if(*p == '\0') {
            p[0] = fmt;
            p[1] = '\0';
        }
        for(i = 0; i < count; i++) {
            c += pfmt(*p, 1, 0);
            dot += inc;
            if(c > width) {
                Bprint(bout, "\n");
                symoff(tbuf, sizeof(tbuf), dot, CTEXT);
                Bprint(bout, "%s?\t", tbuf);
                c = 0;
            }
        }
        fmt = *p++;
        p = nextc(p);
    }
    Bprint(bout, "\n");
}
/*e: function [[quesie]] */

/*s: function [[catcher]] */
void
catcher(void *a, char *msg)
{
    static int hit = 0;

    hit++;
    if(hit > 5)
        exits(0);
    USED(a);
    if(strcmp(msg, "interrupt") != 0)
        noted(NDFLT);

    count = 1;
    print("5i\n");
    noted(NCONT);
}
/*e: function [[catcher]] */

/*s: function [[setreg]] */
void
setreg(char *addr, char *cp)
{
    int rn;

    dot = expr(addr);
    cp = nextc(cp);
    if(strcmp(cp, "pc") == 0) {
        reg.r[REGPC] = dot;
        return;
    }
    if(strcmp(cp, "sp") == 0) {
        reg.r[REGSP] = dot;
        return;
    }
    if(*cp++ == 'r') {
        rn = strtoul(cp, 0, 10);
        if(rn > 0 && rn < 16) {
            reg.r[rn] = dot;
            return;
        }
    }
    Bprint(bout, "bad register\n");
}
/*e: function [[setreg]] */

/*s: function [[cmd]] */
void
cmd(void)
{
    /*s: [[cmd()]] locals */
    char *p;
    /*x: [[cmd()]] locals */
    static char *cmdlet = ":$?/=>"; //$
    /*x: [[cmd()]] locals */
    char buf[128];
    char addr[128];
    /*x: [[cmd()]] locals */
    char lastcmd[128];
    /*x: [[cmd()]] locals */
    char *a, *cp, *gotint;
    int n, i;
    /*e: [[cmd()]] locals */

    dot = reg.r[REGPC];

    /*s: [[cmd()]] initialisation */
    notify(catcher);
    /*x: [[cmd()]] initialisation */
    setjmp(errjmp);
    /*e: [[cmd()]] initialisation */

    for(;;) {
        Bflush(bout);
        /*s: [[cmd()]] read and parse command and address from user input */
        p = buf;
        n = 0;

        for(;;) {
            i = Bgetc(bin);
            if(i < 0)
                exits(0);
            *p++ = i;
            n++;
            if(i == '\n')
                break;
        }

        if(buf[0] == '\n')
            strcpy(buf, lastcmd);
        else {
            buf[n-1] = '\0';
            strcpy(lastcmd, buf);
        }

        p = buf;
        a = addr;
        for(;;) {
            p = nextc(p);
            if(*p == '\0' || strchr(cmdlet, *p))
                break;
            *a++ = *p++;
        }
        *a = '\0';

        cmdcount = 1;
        cp = strchr(addr, ',');
        if(cp != nil) {
            if(cp[1] == '#')
                cmdcount = strtoul(cp+2, &gotint, 16);
            else
                cmdcount = strtoul(cp+1, &gotint, 0);
            *cp = '\0';
        }
        /*e: [[cmd()]] read and parse command and address from user input */

        switch(*p) {
        /*s: [[cmd()]] command cases */
        case ':':
            colon(addr, p+1);
            break;
        /*x: [[cmd()]] command cases */
        case '$': //$
            dollar(p+1);
            break;
        /*x: [[cmd()]] command cases */
        case '/':
        case '?':
            dot = expr(addr);
            for(i = 0; i < cmdcount; i++)
                quesie(p+1);
            break;
        /*x: [[cmd()]] command cases */
        case '=':
            eval(addr, p+1);
            break;
        /*x: [[cmd()]] command cases */
        case '>':
            setreg(addr, p+1);
            break;
        /*e: [[cmd()]] command cases */
        default:
            Bprint(bout, "?\n");
            break;
        }
    }
}
/*e: function [[cmd]] */
/*e: machine/5i/cmd.c */
