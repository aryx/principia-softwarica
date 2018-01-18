/*s: libregexp/regcomp.c */
#include <u.h>
#include <libc.h>
#include "regexp.h"
#include "regcomp.h"

/*s: constant [[TRUE]] */
#define	TRUE	1
/*e: constant [[TRUE]] */
/*s: constant [[FALSE]] */
#define	FALSE	0
/*e: constant [[FALSE]] */

/*s: struct [[Node]] */
/*
 * Parser Information
 */
typedef
struct Node
{
    Reinst*	first;
    Reinst*	last;
}Node;
/*e: struct [[Node]] */

/*s: global [[reprog]] */
/* max character classes per program is nelem(reprog->class) */
static Reprog	*reprog;
/*e: global [[reprog]] */

/*s: constant [[NCCRUNE]] */
/* max rune ranges per character class is nelem(classp->spans)/2 */
#define NCCRUNE	nelem(classp->spans)
/*e: constant [[NCCRUNE]] */

/*s: constant [[NSTACK]] */
#define	NSTACK	20
/*e: constant [[NSTACK]] */
/*s: global [[andstack]] */
static	Node	andstack[NSTACK];
/*e: global [[andstack]] */
/*s: global [[andp]] */
static	Node	*andp;
/*e: global [[andp]] */
/*s: global [[atorstack]] */
static	int	atorstack[NSTACK];
/*e: global [[atorstack]] */
/*s: global [[atorp]] */
static	int*	atorp;
/*e: global [[atorp]] */
/*s: global [[cursubid]] */
static	int	cursubid;		/* id of current subexpression */
/*e: global [[cursubid]] */
/*s: global [[subidstack]] */
static	int	subidstack[NSTACK];	/* parallel to atorstack */
/*e: global [[subidstack]] */
/*s: global [[subidp]] */
static	int*	subidp;
/*e: global [[subidp]] */
/*s: global [[lastwasand]] */
static	int	lastwasand;	/* Last token was operand */
/*e: global [[lastwasand]] */
/*s: global [[nbra]] */
static	int	nbra;
/*e: global [[nbra]] */
/*s: global [[exprp]] */
static	char*	exprp;		/* pointer to next character in source expression */
/*e: global [[exprp]] */
/*s: global [[lexdone]] */
static	int	lexdone;
/*e: global [[lexdone]] */
/*s: global [[nclass]] */
static	int	nclass;
/*e: global [[nclass]] */
/*s: global [[classp]] */
static	Reclass*classp;
/*e: global [[classp]] */
/*s: global [[freep]] */
static	Reinst*	freep;
/*e: global [[freep]] */
/*s: global [[errors]] */
static	int	errors;
/*e: global [[errors]] */
/*s: global [[yyrune]] */
static	Rune	yyrune;		/* last lex'd rune */
/*e: global [[yyrune]] */
/*s: global [[yyclassp]] */
static	Reclass*yyclassp;	/* last lex'd class */
/*e: global [[yyclassp]] */

/* predeclared crap */
static	void	operator(int);
static	void	pushand(Reinst*, Reinst*);
static	void	pushator(int);
static	void	evaluntil(int);
static	int	bldcclass(void);

/*s: global [[regkaboom]] */
static jmp_buf regkaboom;
/*e: global [[regkaboom]] */

/*s: function [[rcerror]] */
static	void
rcerror(char *s)
{
    errors++;
    regerror(s);
    longjmp(regkaboom, 1);
}
/*e: function [[rcerror]] */

/*s: function [[newinst]] */
static	Reinst*
newinst(int t)
{
    freep->type = t;
    freep->left = 0;
    freep->right = 0;
    return freep++;
}
/*e: function [[newinst]] */

/*s: function [[operand]] */
static	void
operand(int t)
{
    Reinst *i;

    if(lastwasand)
        operator(CAT);	/* catenate is implicit */
    i = newinst(t);

    if(t == CCLASS || t == NCCLASS)
        i->cp = yyclassp;
    if(t == RUNE)
        i->r = yyrune;

    pushand(i, i);
    lastwasand = TRUE;
}
/*e: function [[operand]] */

/*s: function [[operator]] */
static	void
operator(int t)
{
    if(t==RBRA && --nbra<0)
        rcerror("unmatched right paren");
    if(t==LBRA){
        if(++cursubid >= NSUBEXP)
            rcerror ("too many subexpressions");
        nbra++;
        if(lastwasand)
            operator(CAT);
    } else
        evaluntil(t);
    if(t != RBRA)
        pushator(t);
    lastwasand = FALSE;
    if(t==STAR || t==QUEST || t==PLUS || t==RBRA)
        lastwasand = TRUE;	/* these look like operands */
}
/*e: function [[operator]] */

/*s: function [[regerr2]] */
static	void
regerr2(char *s, int c)
{
    char buf[100];
    char *cp = buf;
    while(*s)
        *cp++ = *s++;
    *cp++ = c;
    *cp = '\0'; 
    rcerror(buf);
}
/*e: function [[regerr2]] */

/*s: function [[cant]] */
static	void
cant(char *s)
{
    char buf[100];
    strcpy(buf, "can't happen: ");
    strcat(buf, s);
    rcerror(buf);
}
/*e: function [[cant]] */

/*s: function [[pushand]] */
static	void
pushand(Reinst *f, Reinst *l)
{
    if(andp >= &andstack[NSTACK])
        cant("operand stack overflow");
    andp->first = f;
    andp->last = l;
    andp++;
}
/*e: function [[pushand]] */

/*s: function [[pushator]] */
static	void
pushator(int t)
{
    if(atorp >= &atorstack[NSTACK])
        cant("operator stack overflow");
    *atorp++ = t;
    *subidp++ = cursubid;
}
/*e: function [[pushator]] */

/*s: function [[popand]] */
static	Node*
popand(int op)
{
    Reinst *inst;

    if(andp <= &andstack[0]){
        regerr2("missing operand for ", op);
        inst = newinst(NOP);
        pushand(inst,inst);
    }
    return --andp;
}
/*e: function [[popand]] */

/*s: function [[popator]] */
static	int
popator(void)
{
    if(atorp <= &atorstack[0])
        cant("operator stack underflow");
    --subidp;
    return *--atorp;
}
/*e: function [[popator]] */

/*s: function [[evaluntil]] */
static	void
evaluntil(int pri)
{
    Node *op1, *op2;
    Reinst *inst1, *inst2;

    while(pri==RBRA || atorp[-1]>=pri){
        switch(popator()){
        default:
            rcerror("unknown operator in evaluntil");
            break;
        case LBRA:		/* must have been RBRA */
            op1 = popand('(');
            inst2 = newinst(RBRA);
            inst2->subid = *subidp;
            op1->last->next = inst2;
            inst1 = newinst(LBRA);
            inst1->subid = *subidp;
            inst1->next = op1->first;
            pushand(inst1, inst2);
            return;
        case OR:
            op2 = popand('|');
            op1 = popand('|');
            inst2 = newinst(NOP);
            op2->last->next = inst2;
            op1->last->next = inst2;
            inst1 = newinst(OR);
            inst1->right = op1->first;
            inst1->left = op2->first;
            pushand(inst1, inst2);
            break;
        case CAT:
            op2 = popand(0);
            op1 = popand(0);
            op1->last->next = op2->first;
            pushand(op1->first, op2->last);
            break;
        case STAR:
            op2 = popand('*');
            inst1 = newinst(OR);
            op2->last->next = inst1;
            inst1->right = op2->first;
            pushand(inst1, inst1);
            break;
        case PLUS:
            op2 = popand('+');
            inst1 = newinst(OR);
            op2->last->next = inst1;
            inst1->right = op2->first;
            pushand(op2->first, inst1);
            break;
        case QUEST:
            op2 = popand('?');
            inst1 = newinst(OR);
            inst2 = newinst(NOP);
            inst1->left = inst2;
            inst1->right = op2->first;
            op2->last->next = inst2;
            pushand(inst1, inst2);
            break;
        }
    }
}
/*e: function [[evaluntil]] */

/*s: function [[optimize]] */
static	Reprog*
optimize(Reprog *pp)
{
    Reinst *inst, *target;
    int size;
    Reprog *npp;
    Reclass *cl;
    int diff;

    /*
     *  get rid of NOOP chains
     */
    for(inst=pp->firstinst; inst->type!=END; inst++){
        target = inst->next;
        while(target->type == NOP)
            target = target->next;
        inst->next = target;
    }

    /*
     *  The original allocation is for an area larger than
     *  necessary.  Reallocate to the actual space used
     *  and then relocate the code.
     */
    size = sizeof(Reprog) + (freep - pp->firstinst)*sizeof(Reinst);
    npp = realloc(pp, size);
    if(npp==0 || npp==pp)
        return pp;
    diff = (char *)npp - (char *)pp;
    freep = (Reinst *)((char *)freep + diff);
    for(inst=npp->firstinst; inst<freep; inst++){
        switch(inst->type){
        case OR:
        case STAR:
        case PLUS:
        case QUEST:
            *(char **)&inst->right += diff;
            break;
        case CCLASS:
        case NCCLASS:
            *(char **)&inst->right += diff;
            cl = inst->cp;
            *(char **)&cl->end += diff;
            break;
        }
        *(char **)&inst->left += diff;
    }
    *(char **)&npp->startinst += diff;
    return npp;
}
/*e: function [[optimize]] */

#ifdef	DEBUG
/*s: function [[dumpstack]] */
static	void
dumpstack(void){
    Node *stk;
    int *ip;

    print("operators\n");
    for(ip=atorstack; ip<atorp; ip++)
        print("0%o\n", *ip);
    print("operands\n");
    for(stk=andstack; stk<andp; stk++)
        print("0%o\t0%o\n", stk->first->type, stk->last->type);
}
/*e: function [[dumpstack]] */

/*s: function [[dump]] */
static	void
dump(Reprog *pp)
{
    Reinst *l;
    Rune *p;

    l = pp->firstinst;
    do{
        print("%d:\t0%o\t%d\t%d", l-pp->firstinst, l->type,
            l->left-pp->firstinst, l->right-pp->firstinst);
        if(l->type == RUNE)
            print("\t%C\n", l->r);
        else if(l->type == CCLASS || l->type == NCCLASS){
            print("\t[");
            if(l->type == NCCLASS)
                print("^");
            for(p = l->cp->spans; p < l->cp->end; p += 2)
                if(p[0] == p[1])
                    print("%C", p[0]);
                else
                    print("%C-%C", p[0], p[1]);
            print("]\n");
        } else
            print("\n");
    }while(l++->type);
}
/*e: function [[dump]] */
#endif

/*s: function [[newclass]] */
static	Reclass*
newclass(void)
{
    if(nclass >= nelem(reprog->class))
        rcerror("too many character classes; increase Reprog.class size");
    return &(classp[nclass++]);
}
/*e: function [[newclass]] */

/*s: function [[nextc]] */
static	int
nextc(Rune *rp)
{
    if(lexdone){
        *rp = 0;
        return 1;
    }
    exprp += chartorune(rp, exprp);
    if(*rp == L'\\'){
        exprp += chartorune(rp, exprp);
        return 1;
    }
    if(*rp == 0)
        lexdone = 1;
    return 0;
}
/*e: function [[nextc]] */

/*s: function [[lex]] */
static	int
lex(int literal, int dot_type)
{
    int quoted;

    quoted = nextc(&yyrune);
    if(literal || quoted){
        if(yyrune == 0)
            return END;
        return RUNE;
    }

    switch(yyrune){
    case 0:
        return END;
    case L'*':
        return STAR;
    case L'?':
        return QUEST;
    case L'+':
        return PLUS;
    case L'|':
        return OR;
    case L'.':
        return dot_type;
    case L'(':
        return LBRA;
    case L')':
        return RBRA;
    case L'^':
        return BOL;
    case L'$':
        return EOL;
    case L'[':
        return bldcclass();
    }
    return RUNE;
}
/*e: function [[lex]] */

/*s: function [[bldcclass]] */
static int
bldcclass(void)
{
    int type;
    Rune r[NCCRUNE];
    Rune *p, *ep, *np;
    Rune rune;
    int quoted;

    /* we have already seen the '[' */
    type = CCLASS;
    yyclassp = newclass();

    /* look ahead for negation */
    /* SPECIAL CASE!!! negated classes don't match \n */
    ep = r;
    quoted = nextc(&rune);
    if(!quoted && rune == L'^'){
        type = NCCLASS;
        quoted = nextc(&rune);
        *ep++ = L'\n';
        *ep++ = L'\n';
    }

    /* parse class into a set of spans */
    while(ep < &r[NCCRUNE-1]){
        if(rune == 0){
            rcerror("malformed '[]'");
            return 0;
        }
        if(!quoted && rune == L']')
            break;
        if(!quoted && rune == L'-'){
            if(ep == r){
                rcerror("malformed '[]'");
                return 0;
            }
            quoted = nextc(&rune);
            if((!quoted && rune == L']') || rune == 0){
                rcerror("malformed '[]'");
                return 0;
            }
            *(ep-1) = rune;
        } else {
            *ep++ = rune;
            *ep++ = rune;
        }
        quoted = nextc(&rune);
    }
    if(ep >= &r[NCCRUNE-1]) {
        rcerror("char class too large; increase Reclass.spans size");
        return 0;
    }

    /* sort on span start */
    for(p = r; p < ep; p += 2){
        for(np = p; np < ep; np += 2)
            if(*np < *p){
                rune = np[0];
                np[0] = p[0];
                p[0] = rune;
                rune = np[1];
                np[1] = p[1];
                p[1] = rune;
            }
    }

    /* merge spans */
    np = yyclassp->spans;
    p = r;
    if(r == ep)
        yyclassp->end = np;
    else {
        np[0] = *p++;
        np[1] = *p++;
        for(; p < ep; p += 2)
            /* overlapping or adjacent ranges? */
            if(p[0] <= np[1] + 1){
                if(p[1] >= np[1])
                    np[1] = p[1];	/* coalesce */
            } else {
                np += 2;
                np[0] = p[0];
                np[1] = p[1];
            }
        yyclassp->end = np+2;
    }

    return type;
}
/*e: function [[bldcclass]] */

/*s: function [[regcomp1]] */
static	Reprog*
regcomp1(char *s, int literal, int dot_type)
{
    int token;
    Reprog *pp;

    /* get memory for the program */
    pp = malloc(sizeof(Reprog) + 6*sizeof(Reinst)*strlen(s));
    if(pp == 0){
        regerror("out of memory");
        return 0;
    }
    freep = pp->firstinst;
    classp = pp->class;
    errors = 0;

    if(setjmp(regkaboom))
        goto out;

    /* go compile the sucker */
    lexdone = 0;
    exprp = s;
    nclass = 0;
    nbra = 0;
    atorp = atorstack;
    andp = andstack;
    subidp = subidstack;
    lastwasand = FALSE;
    cursubid = 0;

    /* Start with a low priority operator to prime parser */
    pushator(START-1);
    while((token = lex(literal, dot_type)) != END){
        if((token&0300) == OPERATOR)
            operator(token);
        else
            operand(token);
    }

    /* Close with a low priority operator */
    evaluntil(START);

    /* Force END */
    operand(END);
    evaluntil(START);
#ifdef DEBUG
    dumpstack();
#endif
    if(nbra)
        rcerror("unmatched left paren");
    --andp;	/* points to first and only operand */
    pp->startinst = andp->first;
#ifdef DEBUG
    dump(pp);
#endif
    pp = optimize(pp);
#ifdef DEBUG
    print("start: %d\n", andp->first-pp->firstinst);
    dump(pp);
#endif
out:
    if(errors){
        free(pp);
        pp = 0;
    }
    return pp;
}
/*e: function [[regcomp1]] */

/*s: function [[regcomp]] */
extern	Reprog*
regcomp(char *s)
{
    return regcomp1(s, 0, ANY);
}
/*e: function [[regcomp]] */

/*s: function [[regcomplit]] */
extern	Reprog*
regcomplit(char *s)
{
    return regcomp1(s, 1, ANY);
}
/*e: function [[regcomplit]] */

/*s: function [[regcompnl]] */
extern	Reprog*
regcompnl(char *s)
{
    return regcomp1(s, 0, ANYNL);
}
/*e: function [[regcompnl]] */
/*e: libregexp/regcomp.c */
