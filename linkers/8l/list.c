/*s: linkers/8l/list.c */
#include	"l.h"

int	Aconv(Fmt*);
int	Dconv(Fmt*);
int	Pconv(Fmt*);
int	Rconv(Fmt*);
int	Sconv(Fmt*);

/*s: function [[listinit]](x86) */
void
listinit(void)
{

    fmtinstall('A', Aconv);
    fmtinstall('R', Rconv);
    fmtinstall('D', Dconv);
    fmtinstall('P', Pconv);
    fmtinstall('S', Sconv);
}
/*e: function [[listinit]](x86) */

/*s: global [[bigP]](x86) */
static	Prog	*bigP;
/*e: global [[bigP]](x86) */

/*s: function [[Pconv]](x86) */
// Prog -> string
int
Pconv(Fmt *fp)
{
    char str[STRINGSZ];
    Prog *p;

    p = va_arg(fp->args, Prog*);
    bigP = p;
    switch(p->as) {
    case ATEXT:
        // when this happens?
        if(p->from.scale) {
            snprint(str, sizeof(str), "(%ld)	%A	%D,%d,%D",
                p->line, p->as, &p->from, p->from.scale, &p->to);
        } else {
            snprint(str, sizeof(str), "(%ld)	%A	%D,%D",
                p->line, p->as, &p->from, &p->to);
        }
        break;
    case ADATA:
    case AINIT:
    case ADYNT:
        snprint(str, sizeof(str), "(%ld)	%A	%D/%d,%D",
            p->line, p->as, &p->from, p->from.scale, &p->to);
        break;
    default:
        snprint(str, sizeof(str), "(%ld)	%A	%D,%D",
            p->line, p->as, &p->from, &p->to);
        break;
    }
    bigP = P;
    return fmtstrcpy(fp, str);
}
/*e: function [[Pconv]](x86) */

/*s: function [[Aconv]](x86) */
// enum<opcode> -> string
int
Aconv(Fmt *fp)
{
    int i;

    i = va_arg(fp->args, int);
    return fmtstrcpy(fp, anames[i]);
}
/*e: function [[Aconv]](x86) */

/*s: function [[Dconv]](x86) */
// Adr -> string
int
Dconv(Fmt *fp)
{
    char str[STRINGSZ+40], s[20];
    Adr *a;
    //enum<operand_kind>
    int i;

    a = va_arg(fp->args, Adr*);
    i = a->type;

    /*s: [[Dconv()]] if i >= D_INDIR(x86) */
    if(i >= D_INDIR) {
        if(a->offset)
            snprint(str, sizeof(str), "%ld(%R)", a->offset, i-D_INDIR);
        else
            snprint(str, sizeof(str), "(%R)", i-D_INDIR);
        goto brk;
    }
    /*e: [[Dconv()]] if i >= D_INDIR(x86) */

    switch(i) {
    case D_NONE:
        str[0] = '\0';
        break;


    case D_BRANCH:
        if(bigP != P && bigP->pcond != P)
            if(a->sym != S)
                snprint(str, sizeof(str), "%lux+%s", bigP->pcond->pc,
                    a->sym->name);
            else
                snprint(str, sizeof(str), "%lux", bigP->pcond->pc);
        else
            snprint(str, sizeof(str), "%ld(PC)", a->offset);
        break;

    case D_EXTERN:
        snprint(str, sizeof(str), "%s+%ld(SB)", a->sym->name, a->offset);
        break;
    case D_STATIC:
        snprint(str, sizeof(str), "%s<%d>+%ld(SB)", a->sym->name,
            a->sym->version, a->offset);
        break;
    case D_AUTO:
        snprint(str, sizeof(str), "%s+%ld(SP)", a->sym->name, a->offset);
        break;
    case D_PARAM:
        if(a->sym)
            snprint(str, sizeof(str), "%s+%ld(FP)", a->sym->name, a->offset);
        else
            snprint(str, sizeof(str), "%ld(FP)", a->offset);
        break;

    case D_CONST:
        snprint(str, sizeof(str), "$%ld", a->offset);
        break;
    case D_FCONST:
        snprint(str, sizeof(str), "$(%.8lux,%.8lux)", a->ieee.h, a->ieee.l);
        break;
    case D_SCONST:
        snprint(str, sizeof(str), "$\"%S\"", a->scon);
        break;

    case D_ADDR:
        a->type = a->index;
        a->index = D_NONE;
        snprint(str, sizeof(str), "$%D", a);
        a->index = a->type;
        a->type = D_ADDR;
        goto conv;

    default:
        snprint(str, sizeof(str), "%R", i);
        break;

    }
brk:
    if(a->index != D_NONE) {
        snprint(s, sizeof(s), "(%R*%d)", a->index, a->scale);
        strcat(str, s);
    }
conv:
    return fmtstrcpy(fp, str);
}
/*e: function [[Dconv]](x86) */

/*s: global [[regstr]](x86) */
// coupling with enum regs in 8.out.h
char*	regstr[] =
{
    "AL",		/* [D_AL] */
    "CL",
    "DL",
    "BL",
    "AH",
    "CH",
    "DH",
    "BH",

    "AX",		/* [D_AX] */
    "CX",
    "DX",
    "BX",
    "SP",
    "BP",
    "SI",
    "DI",

    "F0",		/* [D_F0] */
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",

    "CS",		/* [D_CS] */
    "SS",
    "DS",
    "ES",
    "FS",
    "GS",

    "GDTR",		/* [D_GDTR] */
    "IDTR",		/* [D_IDTR] */
    "LDTR",		/* [D_LDTR] */
    "MSW",		/* [D_MSW] */
    "TASK",		/* [D_TASK] */

    "CR0",		/* [D_CR] */
    "CR1",
    "CR2",
    "CR3",
    "CR4",
    "CR5",
    "CR6",
    "CR7",

    "DR0",		/* [D_DR] */
    "DR1",
    "DR2",
    "DR3",
    "DR4",
    "DR5",
    "DR6",
    "DR7",

    "TR0",		/* [D_TR] */
    "TR1",
    "TR2",
    "TR3",
    "TR4",
    "TR5",
    "TR6",
    "TR7",

    "NONE",		/* [D_NONE] */
};
/*e: global [[regstr]](x86) */

/*s: function [[Rconv]](x86) */
// enum<operand_kind(register-only)> -> string
int
Rconv(Fmt *fp)
{
    char str[20];
    int r;

    r = va_arg(fp->args, int);
    if(r >= D_AL && r <= D_NONE)
        snprint(str, sizeof(str), "%s", regstr[r-D_AL]);
    else
        snprint(str, sizeof(str), "gok(%d)", r);

    return fmtstrcpy(fp, str);
}
/*e: function [[Rconv]](x86) */

/*s: function [[Sconv]](x86) */
// ?? -> string
int
Sconv(Fmt *fp)
{
    int i, c;
    char str[30], *p, *a;

    a = va_arg(fp->args, char*);
    p = str;
    for(i=0; i<sizeof(double); i++) {
        c = a[i] & 0xff;
        if(c >= 'a' && c <= 'z' ||
           c >= 'A' && c <= 'Z' ||
           c >= '0' && c <= '9') {
            *p++ = c;
            continue;
        }
        *p++ = '\\';

        switch(c) {
        default:
            if(c < 040 || c >= 0177)
                break;	/* not portable */
            p[-1] = c;
            continue;
        case 0:
            *p++ = 'z';
            continue;
        case '\\':
        case '"':
            *p++ = c;
            continue;
        case '\n':
            *p++ = 'n';
            continue;
        case '\t':
            *p++ = 't';
            continue;
        }
        *p++ = (c>>6) + '0';
        *p++ = ((c>>3) & 7) + '0';
        *p++ = (c & 7) + '0';
    }
    *p = 0;
    return fmtstrcpy(fp, str);
}
/*e: function [[Sconv]](x86) */

/*s: function [[diag]] */
void
diag(char *fmt, ...)
{
    char buf[STRINGSZ];
    char *tn;
    va_list arg;

    tn = "??none??";
    if(curtext != P && curtext->from.sym != S)
        tn = curtext->from.sym->name;
    va_start(arg, fmt);
    vseprint(buf, buf+sizeof(buf), fmt, arg);
    va_end(arg);
    print("%s: %s\n", tn, buf);

    nerrors++;
    if(nerrors > 20 && !debug['A']) {
        print("too many errors\n");
        errorexit();
    }
}
/*e: function [[diag]] */
/*e: linkers/8l/list.c */
