/*s: 5c/txt.c */
#include "gc.h"

/*s: global resvreg(arm) */
static	char	resvreg[nelem(reg)];
/*e: global resvreg(arm) */

/*s: function ginit(arm) */
void
ginit(void)
{
    Type *t;

    thechar = '5';
    thestring = "arm";
    exregoffset = REGEXT;
    exfregoffset = FREGEXT;
    listinit();
    nstring = 0;
    mnstring = 0;
    nrathole = 0;
    pc = 0;
    breakpc = -1;
    continpc = -1;
    cases = C;
    firstp = P;
    lastp = P;
    tfield = types[TLONG];

    zprog.link = P;
    zprog.as = AGOK;
    zprog.reg = NREG;
    zprog.from.type = D_NONE;
    zprog.from.name = D_NONE;
    zprog.from.reg = NREG;
    zprog.to = zprog.from;
    zprog.scond = 0xE;  

    regnode.op = OREGISTER;
    regnode.class = CEXREG;
    regnode.reg = REGTMP;
    regnode.complex = 0;
    regnode.addable = 11;
    regnode.type = types[TLONG];

    constnode.op = OCONST;
    constnode.class = CXXX;
    constnode.complex = 0;
    constnode.addable = 20;
    constnode.type = types[TLONG];

    fconstnode.op = OCONST;
    fconstnode.class = CXXX;
    fconstnode.complex = 0;
    fconstnode.addable = 20;
    fconstnode.type = types[TDOUBLE];

    nodsafe = new(ONAME, Z, Z);
    nodsafe->sym = slookup(".safe");
    nodsafe->type = types[TINT];
    nodsafe->etype = types[TINT]->etype;
    nodsafe->class = CAUTO;
    complex(nodsafe);

    t = typ(TARRAY, types[TCHAR]);
    symrathole = slookup(".rathole");
    symrathole->class = CGLOBL;
    symrathole->type = t;

    nodrat = new(ONAME, Z, Z);
    nodrat->sym = symrathole;
    nodrat->type = types[TIND];
    nodrat->etype = TVOID;
    nodrat->class = CGLOBL;
    complex(nodrat);
    nodrat->type = t;

    nodret = new(ONAME, Z, Z);
    nodret->sym = slookup(".ret");
    nodret->type = types[TIND];
    nodret->etype = TIND;
    nodret->class = CPARAM;
    nodret = new(OIND, nodret, Z);
    complex(nodret);

    com64init();

    memset(reg, 0, sizeof(reg));
    /* don't allocate */
    reg[REGTMP] = 1;
    reg[REGSB] = 1;
    reg[REGSP] = 1;
    reg[REGLINK] = 1;
    reg[REGPC] = 1;
    /* keep two external registers */
    reg[REGEXT] = 1;
    reg[REGEXT-1] = 1;
    memmove(resvreg, reg, sizeof(reg));
}
/*e: function ginit(arm) */

/*s: function gclean(arm) */
void
gclean(void)
{
    int i;
    Sym *s;

    for(i=0; i<NREG; i++)
        if(reg[i] && !resvreg[i])
            diag(Z, "reg %d left allocated", i);
    for(i=NREG; i<NREG+NFREG; i++)
        if(reg[i] && !resvreg[i])
            diag(Z, "freg %d left allocated", i-NREG);
    while(mnstring)
        outstring("", 1L);
    symstring->type->width = nstring;
    symrathole->type->width = nrathole;
    for(i=0; i<NHASH; i++)
    for(s = hash[i]; s != S; s = s->link) {
        if(s->type == T)
            continue;
        if(s->type->width == 0)
            continue;
        if(s->class != CGLOBL && s->class != CSTATIC)
            continue;
        if(s->type == types[TENUM])
            continue;
        gpseudo(AGLOBL, s, nodconst(s->type->width));
    }
    nextpc();
    p->as = AEND;
    outcode();
}
/*e: function gclean(arm) */

/*s: function nextpc */
void
nextpc(void)
{

    p = alloc(sizeof(Prog));
    *p = zprog;
    p->lineno = nearln;
    pc++;

    // add_end_list(p, firstp/lastp)
    if(firstp == P) {
        firstp = p;
        lastp = p;
        return;
    }
    lastp->link = p;
    lastp = p;
}
/*e: function nextpc */

/*s: function gargs */
void
gargs(Node *n, Node *tn1, Node *tn2)
{
    long regs;
    Node fnxargs[20], *fnxp;

    regs = cursafe;

    fnxp = fnxargs;
    garg1(n, tn1, tn2, 0, &fnxp);	/* compile fns to temps */

    curarg = 0;
    fnxp = fnxargs;
    garg1(n, tn1, tn2, 1, &fnxp);	/* compile normal args and temps */

    cursafe = regs;
}
/*e: function gargs */

/*s: function garg1(arm) */
void
garg1(Node *n, Node *tn1, Node *tn2, int f, Node **fnxp)
{
    Node nod;

    if(n == Z)
        return;
    if(n->op == OLIST) {
        garg1(n->left, tn1, tn2, f, fnxp);
        garg1(n->right, tn1, tn2, f, fnxp);
        return;
    }
    if(f == 0) {
        if(n->complex >= FNX) {
            regsalloc(*fnxp, n);
            nod = znode;
            nod.op = OAS;
            nod.left = *fnxp;
            nod.right = n;
            nod.type = n->type;
            cgen(&nod, Z);
            (*fnxp)++;
        }
        return;
    }
    if(typesuv[n->type->etype]) {
        regaalloc(tn2, n);
        if(n->complex >= FNX) {
            sugen(*fnxp, tn2, n->type->width);
            (*fnxp)++;
        } else
            sugen(n, tn2, n->type->width);
        return;
    }
    if(REGARG >= 0 && curarg == 0 && typechlp[n->type->etype]) {
        regaalloc1(tn1, n);
        if(n->complex >= FNX) {
            cgen(*fnxp, tn1);
            (*fnxp)++;
        } else
            cgen(n, tn1);
        return;
    }
    regalloc(tn1, n, Z);
    if(n->complex >= FNX) {
        cgen(*fnxp, tn1);
        (*fnxp)++;
    } else
        cgen(n, tn1);
    regaalloc(tn2, n);
    gopcode(OAS, tn1, Z, tn2);
    regfree(tn1);
}
/*e: function garg1(arm) */

/*s: function nodconst */
Node*
nodconst(long v)
{
    constnode.vconst = v;
    return &constnode;
}
/*e: function nodconst */

/*s: function nod32const(arm) */
Node*
nod32const(vlong v)
{
    constnode.vconst = v & MASK(32);
    return &constnode;
}
/*e: function nod32const(arm) */

/*s: function nodfconst */
Node*
nodfconst(double d)
{
    fconstnode.fconst = d;
    return &fconstnode;
}
/*e: function nodfconst */

/*s: function nodreg(arm) */
void
nodreg(Node *n, Node *nn, int reg)
{
    *n = regnode;
    n->reg = reg;
    n->type = nn->type;
    n->lineno = nn->lineno;
}
/*e: function nodreg(arm) */

/*s: function regret(arm) */
void
regret(Node *n, Node *nn)
{
    int r;

    r = REGRET;
    if(typefd[nn->type->etype])
        r = FREGRET+NREG;
    nodreg(n, nn, r);
    reg[r]++;
}
/*e: function regret(arm) */

/*s: function tmpreg(arm) */
int
tmpreg(void)
{
    int i;

    for(i=REGRET+1; i<NREG; i++)
        if(reg[i] == 0)
            return i;
    diag(Z, "out of fixed registers");
    return 0;
}
/*e: function tmpreg(arm) */

/*s: function regalloc(arm) */
void
regalloc(Node *n, Node *tn, Node *o)
{
    int i, j;
    static int lasti;

    switch(tn->type->etype) {
    case TCHAR:
    case TUCHAR:
    case TSHORT:
    case TUSHORT:
    case TINT:
    case TUINT:
    case TLONG:
    case TULONG:
    case TIND:
        if(o != Z && o->op == OREGISTER) {
            i = o->reg;
            if(i >= 0 && i < NREG)
                goto out;
        }
        j = lasti + REGRET+1;
        for(i=REGRET+1; i<NREG; i++) {
            if(j >= NREG)
                j = REGRET+1;
            if(reg[j] == 0 && resvreg[j] == 0) {
                i = j;
                goto out;
            }
            j++;
        }
        diag(tn, "out of fixed registers");
        goto err;

    case TFLOAT:
    case TDOUBLE:
    case TVLONG:
        if(o != Z && o->op == OREGISTER) {
            i = o->reg;
            if(i >= NREG && i < NREG+NFREG)
                goto out;
        }
        j = 0*2 + NREG;
        for(i=NREG; i<NREG+NFREG; i++) {
            if(j >= NREG+NFREG)
                j = NREG;
            if(reg[j] == 0) {
                i = j;
                goto out;
            }
            j++;
        }
        diag(tn, "out of float registers");
        goto err;
    }
    diag(tn, "unknown type in regalloc: %T", tn->type);
err:
    nodreg(n, tn, 0);
    return;
out:
    reg[i]++;
    lasti++;
    if(lasti >= 5)
        lasti = 0;
    nodreg(n, tn, i);
}
/*e: function regalloc(arm) */

/*s: function regialloc */
void
regialloc(Node *n, Node *tn, Node *o)
{
    Node nod;

    nod = *tn;
    nod.type = types[TIND];
    regalloc(n, &nod, o);
}
/*e: function regialloc */

/*s: function regfree(arm) */
void
regfree(Node *n)
{
    int i;

    i = 0;
    if(n->op != OREGISTER && n->op != OINDREG)
        goto err;
    i = n->reg;
    if(i < 0 || i >= sizeof(reg))
        goto err;
    if(reg[i] <= 0)
        goto err;
    reg[i]--;
    return;
err:
    diag(n, "error in regfree: %d", i);
}
/*e: function regfree(arm) */

/*s: function regsalloc */
void
regsalloc(Node *n, Node *nn)
{
    cursafe = align(cursafe, nn->type, Aaut3);
    maxargsafe = maxround(maxargsafe, cursafe+curarg);
    *n = *nodsafe;
    n->xoffset = -(stkoff + cursafe);
    n->type = nn->type;
    n->etype = nn->type->etype;
    n->lineno = nn->lineno;
}
/*e: function regsalloc */

/*s: function regaalloc1(arm) */
void
regaalloc1(Node *n, Node *nn)
{
    nodreg(n, nn, REGARG);
    reg[REGARG]++;
    curarg = align(curarg, nn->type, Aarg1);
    curarg = align(curarg, nn->type, Aarg2);
    maxargsafe = maxround(maxargsafe, cursafe+curarg);
}
/*e: function regaalloc1(arm) */

/*s: function regaalloc(arm) */
void
regaalloc(Node *n, Node *nn)
{
    curarg = align(curarg, nn->type, Aarg1);
    *n = *nn;
    n->op = OINDREG;
    n->reg = REGSP;
    n->xoffset = curarg + SZ_LONG;
    n->complex = 0;
    n->addable = 20;
    curarg = align(curarg, nn->type, Aarg2);
    maxargsafe = maxround(maxargsafe, cursafe+curarg);
}
/*e: function regaalloc(arm) */

/*s: function regind */
void
regind(Node *n, Node *nn)
{

    if(n->op != OREGISTER) {
        diag(n, "regind not OREGISTER");
        return;
    }
    n->op = OINDREG;
    n->type = nn->type;
}
/*e: function regind */

/*s: function raddr(arm) */
void
raddr(Node *n, Prog *p)
{
    Adr a;

    naddr(n, &a);
    if(R0ISZERO && a.type == D_CONST && a.offset == 0) {
        a.type = D_REG;
        a.reg = 0;
    }
    if(a.type != D_REG && a.type != D_FREG) {
        if(n)
            diag(n, "bad in raddr: %O", n->op);
        else
            diag(n, "bad in raddr: <null>");
        p->reg = NREG;
    } else
        p->reg = a.reg;
}
/*e: function raddr(arm) */

/*s: function naddr(arm) */
void
naddr(Node *n, Adr *a)
{
    long v;

    a->type = D_NONE;
    if(n == Z)
        return;

    switch(n->op) {
    case OREGISTER:
        a->type = D_REG;
        a->sym = S;
        a->reg = n->reg;
        if(a->reg >= NREG) {
            a->type = D_FREG;
            a->reg -= NREG;
        }
        break;

    case OIND:
        naddr(n->left, a);
        if(a->type == D_REG) {
            a->type = D_OREG;
            break;
        }
        if(a->type == D_CONST) {
            a->type = D_OREG;
            break;
        }
        goto bad;

    case OINDREG:
        a->type = D_OREG;
        a->sym = S;
        a->offset = n->xoffset;
        a->reg = n->reg;
        break;

    case ONAME:
        a->etype = n->etype;
        a->type = D_OREG;
        a->name = D_STATIC;
        a->sym = n->sym;
        a->offset = n->xoffset;
        if(n->class == CSTATIC)
            break;
        if(n->class == CEXTERN || n->class == CGLOBL) {
            a->name = D_EXTERN;
            break;
        }
        if(n->class == CAUTO) {
            a->name = D_AUTO;
            break;
        }
        if(n->class == CPARAM) {
            a->name = D_PARAM;
            break;
        }
        goto bad;

    case OCONST:
        a->sym = S;
        a->reg = NREG;
        if(typefd[n->type->etype]) {
            a->type = D_FCONST;
            a->dval = n->fconst;
        } else {
            a->type = D_CONST;
            a->offset = n->vconst;
        }
        break;

    case OADDR:
        naddr(n->left, a);
        if(a->type == D_OREG) {
            a->type = D_CONST;
            break;
        }
        goto bad;

    case OADD:
        if(n->left->op == OCONST) {
            naddr(n->left, a);
            v = a->offset;
            naddr(n->right, a);
        } else {
            naddr(n->right, a);
            v = a->offset;
            naddr(n->left, a);
        }
        a->offset += v;
        break;

    default:
    bad:
        diag(n, "bad in naddr: %O", n->op);
        break;

    }
}
/*e: function naddr(arm) */

/*s: function gmovm(arm) */
void
gmovm(Node *f, Node *t, int w)
{
    gins(AMOVM, f, t);
    p->scond |= C_UBIT;
    if(w)
        p->scond |= C_WBIT;
}
/*e: function gmovm(arm) */

/*s: function gmove(arm) */
void
gmove(Node *f, Node *t)
{
    int ft, tt, a;
    Node nod, nod1;
    Prog *p1;

    ft = f->type->etype;
    tt = t->type->etype;

    if(ft == TDOUBLE && f->op == OCONST) {
    }
    if(ft == TFLOAT && f->op == OCONST) {
    }

    /*
     * a load --
     * put it into a register then
     * worry what to do with it.
     */
    if(f->op == ONAME || f->op == OINDREG || f->op == OIND) {
        switch(ft) {
        case TFLOAT:
            a = AMOVF;
            break;
        case TDOUBLE:
            a = AMOVD;
            break;
        case TCHAR:
            a = AMOVB;
            break;
        case TUCHAR:
            a = AMOVBU;
            break;
        case TSHORT:
            a = AMOVH;
            break;
        case TUSHORT:
            a = AMOVHU;
            break;
        default:
            a = AMOVW;
            break;
        }
        if(typechlp[ft] && typeilp[tt])
            regalloc(&nod, t, t);
        else
            regalloc(&nod, f, t);
        gins(a, f, &nod);
        gmove(&nod, t);
        regfree(&nod);
        return;
    }

    /*
     * a store --
     * put it into a register then
     * store it.
     */
    if(t->op == ONAME || t->op == OINDREG || t->op == OIND) {
        switch(tt) {
        case TUCHAR:
            a = AMOVBU;
            break;
        case TCHAR:
            a = AMOVB;
            break;
        case TUSHORT:
            a = AMOVHU;
            break;
        case TSHORT:
            a = AMOVH;
            break;
        case TFLOAT:
            a = AMOVF;
            break;
        case TVLONG:
        case TDOUBLE:
            a = AMOVD;
            break;
        default:
            a = AMOVW;
            break;
        }
        if(ft == tt)
            regalloc(&nod, t, f);
        else
            regalloc(&nod, t, Z);
        gmove(f, &nod);
        gins(a, &nod, t);
        regfree(&nod);
        return;
    }

    /*
     * type x type cross table
     */
    a = AGOK;
    switch(ft) {
    case TDOUBLE:
    case TVLONG:
    case TFLOAT:
        switch(tt) {
        case TDOUBLE:
        case TVLONG:
            a = AMOVD;
            if(ft == TFLOAT)
                a = AMOVFD;
            break;
        case TFLOAT:
            a = AMOVDF;
            if(ft == TFLOAT)
                a = AMOVF;
            break;
        case TINT:
        case TUINT:
        case TLONG:
        case TULONG:
        case TIND:
            a = AMOVDW;
            if(ft == TFLOAT)
                a = AMOVFW;
            break;
        case TSHORT:
        case TUSHORT:
        case TCHAR:
        case TUCHAR:
            a = AMOVDW;
            if(ft == TFLOAT)
                a = AMOVFW;
            break;
        }
        break;
    case TUINT:
    case TULONG:
        if(tt == TFLOAT || tt == TDOUBLE) {
            // ugly and probably longer than necessary,
            // but vfp has a single instruction for this,
            // so hopefully it won't last long.
            //
            //	tmp = f
            //	tmp1 = tmp & 0x80000000
            //	tmp ^= tmp1
            //	t = float(int32(tmp))
            //	if(tmp1)
            //		t += 2147483648.
            //
            regalloc(&nod, f, Z);
            regalloc(&nod1, f, Z);
            gins(AMOVW, f, &nod);
            gins(AMOVW, &nod, &nod1);
            gins(AAND, nodconst(0x80000000), &nod1);
            gins(AEOR, &nod1, &nod);
            if(tt == TFLOAT)
                gins(AMOVWF, &nod, t);
            else
                gins(AMOVWD, &nod, t);
            gins(ACMP, nodconst(0), Z);
            raddr(&nod1, p);
            gins(ABEQ, Z, Z);
            regfree(&nod);
            regfree(&nod1);
            p1 = p;
            regalloc(&nod, t, Z);
            if(tt == TFLOAT) {
                gins(AMOVF, nodfconst(2147483648.), &nod);
                gins(AADDF, &nod, t);
            } else {
                gins(AMOVD, nodfconst(2147483648.), &nod);
                gins(AADDD, &nod, t);
            }
            regfree(&nod);
            patch(p1, pc);
            return;
        }
        // fall through

    case TINT:
    case TLONG:
    case TIND:
        switch(tt) {
        case TDOUBLE:
            gins(AMOVWD, f, t);
            return;
        case TFLOAT:
            gins(AMOVWF, f, t);
            return;
        case TINT:
        case TUINT:
        case TLONG:
        case TULONG:
        case TIND:
        case TSHORT:
        case TUSHORT:
        case TCHAR:
        case TUCHAR:
            a = AMOVW;
            break;
        }
        break;
    case TSHORT:
        switch(tt) {
        case TDOUBLE:
            regalloc(&nod, f, Z);
            gins(AMOVH, f, &nod);
            gins(AMOVWD, &nod, t);
            regfree(&nod);
            return;
        case TFLOAT:
            regalloc(&nod, f, Z);
            gins(AMOVH, f, &nod);
            gins(AMOVWF, &nod, t);
            regfree(&nod);
            return;
        case TUINT:
        case TINT:
        case TULONG:
        case TLONG:
        case TIND:
            a = AMOVH;
            break;
        case TSHORT:
        case TUSHORT:
        case TCHAR:
        case TUCHAR:
            a = AMOVW;
            break;
        }
        break;
    case TUSHORT:
        switch(tt) {
        case TDOUBLE:
            regalloc(&nod, f, Z);
            gins(AMOVHU, f, &nod);
            gins(AMOVWD, &nod, t);
            regfree(&nod);
            return;
        case TFLOAT:
            regalloc(&nod, f, Z);
            gins(AMOVHU, f, &nod);
            gins(AMOVWF, &nod, t);
            regfree(&nod);
            return;
        case TINT:
        case TUINT:
        case TLONG:
        case TULONG:
        case TIND:
            a = AMOVHU;
            break;
        case TSHORT:
        case TUSHORT:
        case TCHAR:
        case TUCHAR:
            a = AMOVW;
            break;
        }
        break;
    case TCHAR:
        switch(tt) {
        case TDOUBLE:
            regalloc(&nod, f, Z);
            gins(AMOVB, f, &nod);
            gins(AMOVWD, &nod, t);
            regfree(&nod);
            return;
        case TFLOAT:
            regalloc(&nod, f, Z);
            gins(AMOVB, f, &nod);
            gins(AMOVWF, &nod, t);
            regfree(&nod);
            return;
        case TINT:
        case TUINT:
        case TLONG:
        case TULONG:
        case TIND:
        case TSHORT:
        case TUSHORT:
            a = AMOVB;
            break;
        case TCHAR:
        case TUCHAR:
            a = AMOVW;
            break;
        }
        break;
    case TUCHAR:
        switch(tt) {
        case TDOUBLE:
            regalloc(&nod, f, Z);
            gins(AMOVBU, f, &nod);
            gins(AMOVWD, &nod, t);
            regfree(&nod);
            return;
        case TFLOAT:
            regalloc(&nod, f, Z);
            gins(AMOVBU, f, &nod);
            gins(AMOVWF, &nod, t);
            regfree(&nod);
            return;
        case TINT:
        case TUINT:
        case TLONG:
        case TULONG:
        case TIND:
        case TSHORT:
        case TUSHORT:
            a = AMOVBU;
            break;
        case TCHAR:
        case TUCHAR:
            a = AMOVW;
            break;
        }
        break;
    }
    if(a == AGOK)
        diag(Z, "bad opcode in gmove %T -> %T", f->type, t->type);
    if(a == AMOVW || a == AMOVF || a == AMOVD)
    if(samaddr(f, t))
        return;
    gins(a, f, t);
}
/*e: function gmove(arm) */

/*s: function gmover(arm) */
void
gmover(Node *f, Node *t)
{
    int ft, tt, a;

    ft = f->type->etype;
    tt = t->type->etype;
    a = AGOK;
    if(typechlp[ft] && typechlp[tt] && ewidth[ft] >= ewidth[tt]){
        switch(tt){
        case TSHORT:
            a = AMOVH;
            break;
        case TUSHORT:
            a = AMOVHU;
            break;
        case TCHAR:
            a = AMOVB;
            break;
        case TUCHAR:
            a = AMOVBU;
            break;
        }
    }
    if(a == AGOK)
        gmove(f, t);
    else
        gins(a, f, t);
}
/*e: function gmover(arm) */

/*s: function gins(arm) */
void
gins(int a, Node *f, Node *t)
{

    nextpc();
    p->as = a;
    if(f != Z)
        naddr(f, &p->from);
    if(t != Z)
        naddr(t, &p->to);
    if(debug['g'])
        print("%P\n", p);
}
/*e: function gins(arm) */

/*s: function gopcode(arm) */
void
gopcode(int o, Node *f1, Node *f2, Node *t)
{
    int a, et, true;
    Adr ta;

    et = TLONG;
    if(f1 != Z && f1->type != T)
        et = f1->type->etype;
    true = o & BTRUE;
    o &= ~BTRUE;
    a = AGOK;

    switch(o) {
    case OAS:
        gmove(f1, t);
        return;

    case OASADD:
    case OADD:
        a = AADD;
        if(et == TFLOAT)
            a = AADDF;
        else
        if(et == TDOUBLE || et == TVLONG)
            a = AADDD;
        break;

    case OASSUB:
    case OSUB:
        if(f2 && f2->op == OCONST) {
            Node *t = f1;
            f1 = f2;
            f2 = t;
            a = ARSB;
        } else
            a = ASUB;
        if(et == TFLOAT)
            a = ASUBF;
        else
        if(et == TDOUBLE || et == TVLONG)
            a = ASUBD;
        break;

    case OASOR:
    case OOR:
        a = AORR;
        break;

    case OASAND:
    case OAND:
        a = AAND;
        break;

    case OASXOR:
    case OXOR:
        a = AEOR;
        break;

    case OASLSHR:
    case OLSHR:
        a = ASRL;
        break;

    case OASASHR:
    case OASHR:
        a = ASRA;
        break;

    case OASASHL:
    case OASHL:
        a = ASLL;
        break;

    case OFUNC:
        a = ABL;
        break;

    case OASMUL:
    case OMUL:
        a = AMUL;
        if(et == TFLOAT)
            a = AMULF;
        else
        if(et == TDOUBLE || et == TVLONG)
            a = AMULD;
        break;

    case OASDIV:
    case ODIV:
        a = ADIV;
        if(et == TFLOAT)
            a = ADIVF;
        else
        if(et == TDOUBLE || et == TVLONG)
            a = ADIVD;
        break;

    case OASMOD:
    case OMOD:
        a = AMOD;
        break;

    case OASLMUL:
    case OLMUL:
        a = AMULU;
        break;

    case OASLMOD:
    case OLMOD:
        a = AMODU;
        break;

    case OASLDIV:
    case OLDIV:
        a = ADIVU;
        break;

    case OCASE:
    case OEQ:
    case ONE:
    case OLT:
    case OLE:
    case OGE:
    case OGT:
    case OLO:
    case OLS:
    case OHS:
    case OHI:
        a = ACMP;
        if(et == TFLOAT)
            a = ACMPF;
        else
        if(et == TDOUBLE || et == TVLONG)
            a = ACMPD;
        nextpc();
        p->as = a;
        naddr(f1, &p->from);
        if(a == ACMP && f1->op == OCONST && p->from.offset < 0 &&
            p->from.offset != 0x80000000) {
            p->as = ACMN;
            p->from.offset = -p->from.offset;
        }
        raddr(f2, p);
        switch(o) {
        case OEQ:
            a = ABEQ;
            break;
        case ONE:
            a = ABNE;
            break;
        case OLT:
            a = ABLT;
            /* ensure NaN comparison is always false */
            if(typefd[et] && !true)
                a = ABMI;
            break;
        case OLE:
            a = ABLE;
            if(typefd[et] && !true)
                a = ABLS;
            break;
        case OGE:
            a = ABGE;
            if(typefd[et] && true)
                a = ABPL;
            break;
        case OGT:
            a = ABGT;
            if(typefd[et] && true)
                a = ABHI;
            break;
        case OLO:
            a = ABLO;
            break;
        case OLS:
            a = ABLS;
            break;
        case OHS:
            a = ABHS;
            break;
        case OHI:
            a = ABHI;
            break;
        case OCASE:
            nextpc();
            p->as = ACASE;
            p->scond = 0x9;
            naddr(f2, &p->from);
            a = ABHI;
            break;
        }
        f1 = Z;
        f2 = Z;
        break;
    }

    if(a == AGOK)
        diag(Z, "bad in gopcode %O", o);

    nextpc();
    p->as = a;
    if(f1 != Z)
        naddr(f1, &p->from);
    if(f2 != Z) {
        naddr(f2, &ta);
        p->reg = ta.reg;
    }
    if(t != Z)
        naddr(t, &p->to);

    if(debug['g'])
        print("%P\n", p);
}
/*e: function gopcode(arm) */

/*s: function samaddr */
bool
samaddr(Node *f, Node *t)
{

    if(f->op != t->op)
        return false;
    switch(f->op) {

    case OREGISTER:
        if(f->reg != t->reg)
            break;
        return true;
    }
    return false;
}
/*e: function samaddr */

/*s: function gbranch(arm) */
void
gbranch(int o)
{
    int a;

    a = AGOK;

    switch(o) {
    case ORETURN:
        a = ARET;
        break;
    case OGOTO:
        a = AB;
        break;
    }
    nextpc();
    if(a == AGOK) {
        diag(Z, "bad in gbranch %O",  o);
        nextpc();
    }
    p->as = a;
}
/*e: function gbranch(arm) */

/*s: function patch */
void
patch(Prog *op, long pc)
{

    op->to.offset = pc;
    op->to.type = D_BRANCH;
}
/*e: function patch */

/*s: function gpseudo(arm) */
void
gpseudo(int a, Sym *s, Node *n)
{

    nextpc();
    p->as = a;
    p->from.type = D_OREG;
    p->from.sym = s;
    p->from.name = (s->class == CSTATIC) ? D_STATIC : D_EXTERN;
    if(a == ATEXT)
        p->reg = (profileflg ? 0 : NOPROF);
    naddr(n, &p->to);
    if(a == ADATA || a == AGLOBL)
        pc--;
}
/*e: function gpseudo(arm) */

/*s: function sconst(arm) */
bool
sconst(Node *n)
{
    vlong vv;

    if(n->op == OCONST) {
        if(!typefd[n->type->etype]) {
            vv = n->vconst;
            if(vv >= (vlong)(-32766) && vv < (vlong)32766)
                return true;
            /*
             * should be specialised for constant values which will
             * fit in different instructionsl; for now, let 5l
             * sort it out
             */
            return true;
        }
    }
    return false;
}
/*e: function sconst(arm) */

/*s: function sval(arm) */
int
sval(long v)
{
    int i;

    for(i=0; i<16; i++) {
        if((v & ~0xff) == 0)
            return 1;
        if((~v & ~0xff) == 0)
            return 1;
        v = (v<<2) | ((ulong)v>>30);
    }
    return 0;
}
/*e: function sval(arm) */

/*s: function exreg(arm) */
long
exreg(Type *t)
{
    long o;

    if(typechlp[t->etype]) {
        if(exregoffset <= REGEXT-2)
            return 0;
        o = exregoffset;
        if(reg[o] && !resvreg[o])
            return 0;
        resvreg[o] = reg[o] = 1;
        exregoffset--;
        return o;
    }
    if(typefd[t->etype]) {
        if(exfregoffset <= NFREG-1)
            return 0;
        o = exfregoffset + NREG;
        if(reg[o] && !resvreg[o])
            return 0;
        resvreg[o] = reg[o] = 1;
        exfregoffset--;
        return o;
    }
    return 0;
}
/*e: function exreg(arm) */

/*s: global ewidth */
schar	ewidth[NTYPE] =
{
    [TXXX] = -1,		
    [TCHAR] = SZ_CHAR,	
    [TUCHAR] = SZ_CHAR,	
    [TSHORT] = SZ_SHORT,	
    [TUSHORT] = SZ_SHORT,	
    [TINT] = SZ_INT,		
    [TUINT] = SZ_INT,		
    [TLONG] = SZ_LONG,	
    [TULONG] = SZ_LONG,	
    [TVLONG] = SZ_VLONG,	
    [TUVLONG] = SZ_VLONG,	
    [TFLOAT] = SZ_FLOAT,	
    [TDOUBLE] = SZ_DOUBLE,	
    [TIND] = SZ_IND,		
    [TFUNC] = 0,		
    [TARRAY] = -1,		
    [TVOID] = 0,		
    [TSTRUCT] = -1,		
    [TUNION] = -1,		
    [TENUM] = SZ_INT,		
};
/*e: global ewidth */

/*s: global ncast */
long	ncast[NTYPE] =
{
    [TXXX] = 0,				
    [TCHAR] = BCHAR|BUCHAR,			
    [TUCHAR] = BCHAR|BUCHAR,			
    [TSHORT] = BSHORT|BUSHORT,			
    [TUSHORT] = BSHORT|BUSHORT,			
    [TINT] = BINT|BUINT|BLONG|BULONG|BIND,	
    [TUINT] = BINT|BUINT|BLONG|BULONG|BIND,	
    [TLONG] = BINT|BUINT|BLONG|BULONG|BIND,	
    [TULONG] = BINT|BUINT|BLONG|BULONG|BIND,	
    [TVLONG] = BVLONG|BUVLONG,			
    [TUVLONG] = BVLONG|BUVLONG,			
    [TFLOAT] = BFLOAT,				
    [TDOUBLE] = BDOUBLE,			
    [TIND] = BLONG|BULONG|BIND,		
    [TFUNC] = 0,				
    [TARRAY] = 0,				
    [TVOID] = 0,				
    [TSTRUCT] = BSTRUCT,			
    [TUNION] = BUNION,				
    [TENUM] = 0,				
};
/*e: global ncast */
/*e: 5c/txt.c */
