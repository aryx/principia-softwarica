/*s: 5c/txt.c */
#include "gc.h"

/*s: global [[resvreg]](arm) */
static	char	resvreg[nelem(reg)];
/*e: global [[resvreg]](arm) */

/*s: function [[ginit]](arm) */
void
ginit(void)
{
    /*s: [[ginit()]] locals */
    Type *t;
    /*e: [[ginit()]] locals */

    /*s: [[ginit()]] thexxx initialisation */
    thechar = '5';
    thestring = "arm";
    /*e: [[ginit()]] thexxx initialisation */
    /*s: [[ginit()]] pc initialisation */
    pc = 0;
    /*e: [[ginit()]] pc initialisation */
    /*s: [[ginit()]] misc initialisations */
    firstp = P;
    lastp = P;
    /*x: [[ginit()]] misc initialisations */
    breakpc = -1;
    continpc = -1;
    /*x: [[ginit()]] misc initialisations */
    nstring = 0;
    mnstring = 0;
    /*x: [[ginit()]] misc initialisations */
    nrathole = 0;
    /*x: [[ginit()]] misc initialisations */
    cases = C;
    /*x: [[ginit()]] misc initialisations */
    tfield = types[TLONG];
    /*x: [[ginit()]] misc initialisations */
    exregoffset = REGEXT;
    exfregoffset = FREGEXT;
    /*x: [[ginit()]] misc initialisations */
    listinit();
    /*e: [[ginit()]] misc initialisations */
    /*s: [[ginit()]] zprog initialisation */
    zprog.link = P;
    zprog.as = AGOK;
    zprog.reg = R_NONE;
    zprog.from.type = D_NONE;
    zprog.from.symkind = N_NONE;
    zprog.from.reg = R_NONE;
    zprog.to = zprog.from;
    zprog.scond = COND_ALWAYS;  
    /*e: [[ginit()]] zprog initialisation */
    /*s: [[ginit()]] special nodes initialisation */
    constnode.op = OCONST;
    constnode.class = CXXX;
    constnode.complex = 0;
    constnode.addable = 20;
    constnode.type = types[TLONG];
    /*x: [[ginit()]] special nodes initialisation */
    regnode.op = OREGISTER;
    regnode.class = CEXREG;
    regnode.reg = REGTMP;
    regnode.complex = 0;
    regnode.addable = 11;
    regnode.type = types[TLONG];
    /*x: [[ginit()]] special nodes initialisation */
    nodsafe = new(ONAME, Z, Z);
    nodsafe->sym = slookup(".safe");
    nodsafe->type = types[TINT];
    nodsafe->etype = types[TINT]->etype; // TINT
    nodsafe->class = CAUTO;
    complex(nodsafe);
    /*x: [[ginit()]] special nodes initialisation */
    t = typ(TARRAY, types[TCHAR]);
    symrathole = slookup(".rathole");
    symrathole->class = CGLOBL;
    symrathole->type = t;

    nodrat = new(ONAME, Z, Z);
    nodrat->sym = symrathole;
    nodrat->type = types[TIND];
    nodrat->etype = TVOID; // not TIND?
    nodrat->class = CGLOBL;
    complex(nodrat);
    nodrat->type = t;
    /*x: [[ginit()]] special nodes initialisation */
    fconstnode.op = OCONST;
    fconstnode.class = CXXX;
    fconstnode.complex = 0;
    fconstnode.addable = 20;
    fconstnode.type = types[TDOUBLE];
    /*x: [[ginit()]] special nodes initialisation */
    nodret = new(ONAME, Z, Z);
    nodret->sym = slookup(".ret");
    nodret->type = types[TIND];
    nodret->etype = TIND;
    nodret->class = CPARAM;
    nodret = new(OIND, nodret, Z);
    complex(nodret);
    /*e: [[ginit()]] special nodes initialisation */
    /*s: [[ginit()]] com64init initialisation */
    com64init();
    /*e: [[ginit()]] com64init initialisation */
    /*s: [[ginit()]] reg and resvreg initialisation */
    memset(reg, 0, sizeof(reg));

    /* don't allocate */
    reg[REGTMP] = 1;
    reg[REGSB] = 1;
    reg[REGSP] = 1;
    reg[REGLINK] = 1;
    reg[REGPC] = 1;
    /*x: [[ginit()]] reg and resvreg initialisation */
    memmove(resvreg, reg, sizeof(reg));
    /*x: [[ginit()]] reg and resvreg initialisation */
    /* keep two external registers */
    reg[REGEXT] = 1; // R10
    reg[REGEXT-1] = 1; // R9
    /*e: [[ginit()]] reg and resvreg initialisation */
}
/*e: function [[ginit]](arm) */

/*s: function [[gclean]](arm) */
void
gclean(void)
{
    /*s: [[gclean()]] locals */
    int i;
    /*x: [[gclean()]] locals */
    Sym *s;
    /*e: [[gclean()]] locals */

    /*s: [[gclean()]] sanity check reg */
    for(i=0; i<NREG; i++)
        if(reg[i] && !resvreg[i])
            diag(Z, "reg %d left allocated", i);
    /*x: [[gclean()]] sanity check reg */
    for(i=NREG; i<NREG+NFREG; i++)
        if(reg[i] && !resvreg[i])
            diag(Z, "freg %d left allocated", i-NREG);
    /*e: [[gclean()]] sanity check reg */

    /*s: [[gclean()]] adjust symstring width */
    while(mnstring)
        outstring("", 1L);
    symstring->type->width = nstring;
    /*e: [[gclean()]] adjust symstring width */
    /*s: [[gclean()]] adjust symrathole width */
    symrathole->type->width = nrathole;
    /*e: [[gclean()]] adjust symrathole width */

    /*s: [[gclean()]] generate all AGLOBL pseudo opcodes */
    for(i=0; i<NHASH; i++)
     for(s = hash[i]; s != S; s = s->link) {
        /*s: [[gclean()]] when generate all AGLOBL, filter and sanity check symbol */
        if(s->type == T)
            continue;
        if(s->type->width == 0)
            continue;
        if(s->class != CGLOBL && s->class != CSTATIC)
            continue;
        if(s->type == types[TENUM])
            continue;
        /*e: [[gclean()]] when generate all AGLOBL, filter and sanity check symbol */
        // else
        gpseudo(AGLOBL, s, nodconst(s->type->width));
    }
    /*e: [[gclean()]] generate all AGLOBL pseudo opcodes */
    /*s: [[gclean()]] generate last opcode, AEND */
    nextpc();
    p->as = AEND;
    /*e: [[gclean()]] generate last opcode, AEND */

    // generate the whole output file using outbuf global
    outcode();
}
/*e: function [[gclean]](arm) */

/*s: function [[nextpc]] */
void
nextpc(void)
{

    p = alloc(sizeof(Prog));
    *p = zprog;
    p->lineno = nearln; // for origin tracking in db (from assembly to C)
    pc++;

    // add_tail(p, firstp/lastp)
    if(firstp == P) {
        firstp = p;
        lastp = p;
        return;
    }
    lastp->link = p;
    lastp = p;
}
/*e: function [[nextpc]] */

/*s: function [[gargs]] */
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
/*e: function [[gargs]] */

/*s: function [[garg1]](arm) */
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
    } 
    else
    /*s: [[garg1()]] if complex argument type */
    if(typesuv[n->type->etype]) {
        regaalloc(tn2, n);
        if(n->complex >= FNX) {
            sugen(*fnxp, tn2, n->type->width);
            (*fnxp)++;
        } else
            sugen(n, tn2, n->type->width);
    }
    /*e: [[garg1()]] if complex argument type */
    else
    /*s: [[garg1()]] if use REGARG and curarg is zero and simple type */
    if(REGARG >= 0 && curarg == 0 && typechlp[n->type->etype]) {
        regaalloc1(tn1, n);
        if(n->complex >= FNX) {
            cgen(*fnxp, tn1);
            (*fnxp)++;
        } else
            cgen(n, tn1);
    }
    /*e: [[garg1()]] if use REGARG and curarg is zero and simple type */
    else {
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
}
/*e: function [[garg1]](arm) */

/*s: function [[nodconst]] */
Node*
nodconst(long v)
{
    constnode.vconst = v;
    return &constnode;
}
/*e: function [[nodconst]] */

/*s: function [[nod32const]](arm) */
Node*
nod32const(vlong v)
{
    constnode.vconst = v & MASK(32);
    return &constnode;
}
/*e: function [[nod32const]](arm) */

/*s: function [[nodfconst]] */
Node*
nodfconst(double d)
{
    fconstnode.fconst = d;
    return &fconstnode;
}
/*e: function [[nodfconst]] */

/*s: function [[nodreg]](arm) */
void
nodreg(Node *n, Node *nn, int reg)
{
    *n = regnode;
    n->reg = reg;
    n->type = nn->type;
    n->lineno = nn->lineno;
}
/*e: function [[nodreg]](arm) */

/*s: function [[regret]](arm) */
void
regret(Node *n/*OUT*/, Node *nn/*IN*/)
{
    int r;

    r = REGRET;
    /*s: [[regret()]] if float expression, adjust return register */
    if(typefd[nn->type->etype])
        r = FREGRET+NREG;
    /*e: [[regret()]] if float expression, adjust return register */
    nodreg(n, nn, r);
    reg[r]++;
}
/*e: function [[regret]](arm) */

/*s: function [[tmpreg]](arm) */
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
/*e: function [[tmpreg]](arm) */

/*s: function [[regalloc]](arm) */
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
        /*s: [[regalloc()]] if integer type tn and OREGISTER o */
        if(o != Z && o->op == OREGISTER) {
            i = o->reg;
            if(i >= 0 && i < NREG)
                goto out;
        }
        /*e: [[regalloc()]] if integer type tn and OREGISTER o */

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
        /*s: [[regalloc()]] if reach here, out of fixed registers */
        diag(tn, "out of fixed registers");
        goto err;
        /*e: [[regalloc()]] if reach here, out of fixed registers */

    /*s: [[regalloc()]] switch tn type, float or vlong case */
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
    /*e: [[regalloc()]] switch tn type, float or vlong case */
    }
    /*s: [[regalloc()]] if reach here, unknown type */

    diag(tn, "unknown type in regalloc: %T", tn->type);
    // fallthrough
    /*e: [[regalloc()]] if reach here, unknown type */
    /*s: [[regalloc()]] error management */
    err:
        nodreg(n, tn, 0);
        return;
    /*e: [[regalloc()]] error management */

out:
    reg[i]++;
    nodreg(n, tn, i);

    lasti++;
    if(lasti >= 5)
        lasti = 0;

}
/*e: function [[regalloc]](arm) */

/*s: function [[regialloc]] */
void
regialloc(Node *n, Node *tn, Node *o)
{
    Node nod;

    nod = *tn;
    nod.type = types[TIND];
    regalloc(n, &nod, o);
}
/*e: function [[regialloc]] */

/*s: function [[regfree]](arm) */
void
regfree(Node *n)
{
    int i;

    /*s: [[regfree()]] sanity checks */
    i = 0;
    if(n->op != OREGISTER && n->op != OINDREG)
        goto err;
    i = n->reg;
    if(i < 0 || i >= sizeof(reg))
        goto err;
    if(reg[i] <= 0)
        goto err;
    /*e: [[regfree()]] sanity checks */

    reg[n->reg]--;
    return;

err:
    diag(n, "error in regfree: %d", i);
}
/*e: function [[regfree]](arm) */

/*s: function [[regsalloc]] */
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
/*e: function [[regsalloc]] */

/*s: function [[regaalloc1]](arm) */
void
regaalloc1(Node *n, Node *nn)
{
    nodreg(n, nn, REGARG);
    reg[REGARG]++;
    curarg = align(curarg, nn->type, Aarg1);
    curarg = align(curarg, nn->type, Aarg2);
    maxargsafe = maxround(maxargsafe, cursafe+curarg);
}
/*e: function [[regaalloc1]](arm) */

/*s: function [[regaalloc]](arm) */
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
/*e: function [[regaalloc]](arm) */

/*s: function [[regind]] */
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
/*e: function [[regind]] */

/*s: function [[raddr]](arm) */
void
raddr(Node *n, Prog *p)
{
    Adr a;

    naddr(n, &a);
    if(a.type != D_REG && a.type != D_FREG) {
        if(n)
            diag(n, "bad in raddr: %O", n->op);
        else
            diag(n, "bad in raddr: <null>");
        p->reg = R_NONE;
    } else
        p->reg = a.reg;
}
/*e: function [[raddr]](arm) */

/*s: function [[naddr]](arm) */
void
naddr(Node *n, Adr *a)
{
    /*s: [[naddr()]] locals */
    long v;
    /*e: [[naddr()]] locals */

    a->type = D_NONE;
    if(n == Z)
        return;

    switch(n->op) {
    /*s: [[naddr()]] switch node kind cases */
    case OCONST:
        a->sym = S;
        a->reg = R_NONE;
        /*s: [[naddr()]] if float type */
        if(typefd[n->type->etype]) {
            a->type = D_FCONST;
            a->dval = n->fconst;
        } 
        /*e: [[naddr()]] if float type */
        else {
            a->type = D_CONST;
            a->offset = n->vconst;
        }
        break;
    /*x: [[naddr()]] switch node kind cases */
    case OREGISTER:
        a->type = D_REG;
        a->sym = S;
        a->reg = n->reg;
        /*s: [[naddr()]] if float register, adjust a */
        if(a->reg >= NREG) {
            a->type = D_FREG;
            a->reg -= NREG;
        }
        /*e: [[naddr()]] if float register, adjust a */
        break;
    /*x: [[naddr()]] switch node kind cases */
    case ONAME:
        a->type = D_OREG;
        a->offset = n->xoffset;
        a->sym = n->sym;
        /*s: [[naddr()]] when ONAME, save etype */
        a->etype = n->etype;
        /*e: [[naddr()]] when ONAME, save etype */

        a->symkind = N_INTERN;
        switch(n->class) {
        /*s: [[naddr()]] when ONAME, switch class cases */
        case CPARAM:
            a->symkind = N_PARAM;
            break;
        /*x: [[naddr()]] when ONAME, switch class cases */
        case CAUTO:
            a->symkind = N_LOCAL;
            break;
        /*x: [[naddr()]] when ONAME, switch class cases */
        case CEXTERN: case CGLOBL:
            a->symkind = N_EXTERN;
            break;
        /*x: [[naddr()]] when ONAME, switch class cases */
        case CSTATIC: 
            a->symkind = N_INTERN;
            break;
        /*e: [[naddr()]] when ONAME, switch class cases */
        default:
            goto bad;
        }
        break;
    /*x: [[naddr()]] switch node kind cases */
    case OIND:
        naddr(n->left, a);
        if(a->type == D_REG || a->type == D_CONST) {
            a->type = D_OREG;
            break;
        }
        goto bad;
    /*x: [[naddr()]] switch node kind cases */
    case OADDR:
        naddr(n->left, a);
        if(a->type == D_OREG) {
            a->type = D_CONST;
            break;
        }
        // else
        goto bad;
    /*x: [[naddr()]] switch node kind cases */
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
    /*x: [[naddr()]] switch node kind cases */
    case OINDREG:
        a->type = D_OREG;
        a->sym = S;
        a->offset = n->xoffset;
        a->reg = n->reg;
        break;
    /*e: [[naddr()]] switch node kind cases */
    default:
    bad:
        diag(n, "bad in naddr: %O", n->op);
        break;

    }
}
/*e: function [[naddr]](arm) */

/*s: function [[gmovm]](arm) */
void
gmovm(Node *f, Node *t, int w)
{
    gins(AMOVM, f, t);
    p->scond |= C_UBIT;
    if(w)
        p->scond |= C_WBIT;
}
/*e: function [[gmovm]](arm) */

/*s: function [[gmove]](arm) */
void
gmove(Node *f, Node *t)
{
    // enum<Type_kind> // Txxx
    int ft, tt;
    // enum<Opcode_kind> // Axxx
    int a;
    /*s: [[gmove()]] locals */
    Node nod;
    /*x: [[gmove()]] locals */
    Node nod1;
    Prog *p1;
    /*e: [[gmove()]] locals */

    ft = f->type->etype;
    tt = t->type->etype;

    /*
     * a load --
     * put it into a register then
     * worry what to do with it.
     */
    /*s: [[gmove()]] if from is an indirect, registerize and return */
    if(f->op == ONAME || f->op == OIND || f->op == OINDREG) {
        switch(ft) {
        /*s: [[gmove()]] when indirect from, set MOVxx for [[a]] depending on type */
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
        case TFLOAT:
            a = AMOVF;
            break;
        case TDOUBLE:
            a = AMOVD;
            break;
        /*e: [[gmove()]] when indirect from, set MOVxx for [[a]] depending on type */
        default:
            a = AMOVW;
            break;
        }
        /*s: [[gmove()]] when indirect from, if chlp ft and ilp tt */
        if(typechlp[ft] && typeilp[tt])
            regalloc(&nod, t, t);
        /*e: [[gmove()]] when indirect from, if chlp ft and ilp tt */
        else
            regalloc(&nod, f, t);
        gins(a, f, &nod);
        gmove(&nod, t);
        regfree(&nod);
        return;
    }
    /*e: [[gmove()]] if from is an indirect, registerize and return */
    /*
     * a store --
     * put it into a register then
     * store it.
     */
    /*s: [[gmove()]] if to is an indirect, registerize and return */
    if(t->op == ONAME || t->op == OIND || t->op == OINDREG) {
        switch(tt) {
        /*s: [[gmove()]] when indirect to, set MOVxx for [[a]] depending on type */
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
        case TFLOAT:
            a = AMOVF;
            break;
        case TVLONG: // addon?
        case TDOUBLE:
            a = AMOVD;
            break;
        /*e: [[gmove()]] when indirect to, set MOVxx for [[a]] depending on type */
        default:
            a = AMOVW;
            break;
        }
        /*s: [[gmove()]] when indirect to, if ft equal tt */
        if(ft == tt)
            regalloc(&nod, t, f);
        /*e: [[gmove()]] when indirect to, if ft equal tt */
        else
            regalloc(&nod, t, Z);
        gmove(f, &nod);
        gins(a, &nod, t);
        regfree(&nod);
        return;
    }
    /*e: [[gmove()]] if to is an indirect, registerize and return */

    // at this point f and t should be simpler nodes with 
    // registers or constants

    /*
     * type x type cross table
     */
    a = AGOK;
    switch(ft) {
    /*s: [[gmove()]] switch from type cases */
    case TUINT:
    case TULONG:
        /*s: [[gmove()]] switch from type cases, TUINT/TULONG case, if float target */
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
        /*e: [[gmove()]] switch from type cases, TUINT/TULONG case, if float target */
        // fall through
    case TINT:
    case TLONG:
    case TIND:
        switch(tt) {
        /*s: [[gmove()]] switch from type cases, TINT/TLONG case, if float target */
        case TDOUBLE:
            gins(AMOVWD, f, t);
            return;
        case TFLOAT:
            gins(AMOVWF, f, t);
            return;
        /*e: [[gmove()]] switch from type cases, TINT/TLONG case, if float target */
        case TCHAR:
        case TUCHAR:
        case TSHORT:
        case TUSHORT:
        case TINT:
        case TUINT:
        case TLONG:
        case TULONG:

        case TIND:
            a = AMOVW;
            break;
        }
        break;
    /*x: [[gmove()]] switch from type cases */
    case TSHORT:
        switch(tt) {
        /*s: [[gmove()]] switch from type cases, TSHORT case, if float target */
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
        /*e: [[gmove()]] switch from type cases, TSHORT case, if float target */
        case TUINT:
        case TINT:
        case TULONG:
        case TLONG:
        case TIND:
            a = AMOVH;
            break;
        case TCHAR:
        case TUCHAR:
        case TSHORT:
        case TUSHORT:
            a = AMOVW;
            break;
        }
        break;
    /*x: [[gmove()]] switch from type cases */
    case TUSHORT:
        switch(tt) {
        /*s: [[gmove()]] switch from type cases, TUSHORT case, if float target */
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
        /*e: [[gmove()]] switch from type cases, TUSHORT case, if float target */
        case TINT:
        case TUINT:
        case TLONG:
        case TULONG:
        case TIND:
            a = AMOVHU;
            break;
        case TCHAR:
        case TUCHAR:
        case TSHORT:
        case TUSHORT:
            a = AMOVW;
            break;
        }
        break;
    /*x: [[gmove()]] switch from type cases */
    case TCHAR:
        switch(tt) {
        /*s: [[gmove()]] switch from type cases, TCHAR case, if float target */
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
        /*e: [[gmove()]] switch from type cases, TCHAR case, if float target */
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
    /*x: [[gmove()]] switch from type cases */
    case TUCHAR:
        switch(tt) {
        /*s: [[gmove()]] switch from type cases, TUCHAR case, if float target */
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
        /*e: [[gmove()]] switch from type cases, TUCHAR case, if float target */
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
    /*x: [[gmove()]] switch from type cases */
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
    /*e: [[gmove()]] switch from type cases */
    }
    /*s: [[gmove()]] sanity check a */
    if(a == AGOK)
        diag(Z, "bad opcode in gmove %T -> %T", f->type, t->type);
    /*e: [[gmove()]] sanity check a */
    /*s: [[gmove()]] if samaddr return */
    if(a == AMOVW || a == AMOVF || a == AMOVD)
     if(samaddr(f, t))
        return;
    /*e: [[gmove()]] if samaddr return */
    // else

    gins(a, f, t);
}
/*e: function [[gmove]](arm) */

/*s: function [[gmover]](arm) */
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
/*e: function [[gmover]](arm) */

/*s: function [[gins]](arm) */
void
gins(int a, Node *f, Node *t)
{

    nextpc();
    p->as = a;

    if(f != Z)
        naddr(f, &p->from);
    if(t != Z)
        naddr(t, &p->to);

    /*s: [[gins()]] debug */
    if(debug['g'])
        print("%P\n", p);
    /*e: [[gins()]] debug */
}
/*e: function [[gins]](arm) */

/*s: function [[gopcode]](arm) */
void
gopcode(int o, Node *f1, Node *f2, Node *t)
{
    // enum<opcode_kind> AXXX
    int a;
    // enum<type_kind> TXXX
    int et;
    Adr ta;
    /*s: [[gopcode()]] locals */
    bool btrue;
    /*e: [[gopcode()]] locals */

    et = (f1 != Z && f1->type != T)? f1->type->etype : TLONG;
    /*s: [[gopcode()]] initialisation */
    btrue = o & BTRUE;
    o &= ~BTRUE;
    /*e: [[gopcode()]] initialisation */

    a = AGOK;
    switch(o) {
    /*s: [[gopcode()]] switch opcode cases */
    case OADD:
    case OASADD:
        a = AADD;
        /*s: [[gopcode()]] when OADD, adjust a if float type */
        if(et == TFLOAT)
            a = AADDF;
        else
        if(et == TDOUBLE || et == TVLONG)
            a = AADDD;
        /*e: [[gopcode()]] when OADD, adjust a if float type */
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OSUB:
    case OASSUB:
        a = ASUB;
        /*s: [[gopcode()]] when OSUB, ARSB opportunity */
        if(f2 && f2->op == OCONST) {
            Node *t = f1;
            f1 = f2;
            f2 = t;
            a = ARSB;
        }
        /*e: [[gopcode()]] when OSUB, ARSB opportunity */
        /*s: [[gopcode()]] when OSUB, adjust a if float type */
        if(et == TFLOAT)
            a = ASUBF;
        else
        if(et == TDOUBLE || et == TVLONG)
            a = ASUBD;
        /*e: [[gopcode()]] when OSUB, adjust a if float type */
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OOR:
    case OASOR:
        a = AORR;
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OAND:
    case OASAND:
        a = AAND;
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OXOR:
    case OASXOR:
        a = AEOR;
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OASHL:
    case OASASHL:
        a = ASLL;
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OASHR:
    case OASASHR:
        a = ASRA;
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OMUL:
    case OASMUL:
        a = AMUL;
        /*s: [[gopcode()]] when OMUL, adjust a if float type */
        if(et == TFLOAT)
            a = AMULF;
        else
        if(et == TDOUBLE || et == TVLONG)
            a = AMULD;
        /*e: [[gopcode()]] when OMUL, adjust a if float type */
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case ODIV:
    case OASDIV:
        a = ADIV;
        /*s: [[gopcode()]] when ODIV, adjust a if float type */
        if(et == TFLOAT)
            a = ADIVF;
        else
        if(et == TDOUBLE || et == TVLONG)
            a = ADIVD;
        /*e: [[gopcode()]] when ODIV, adjust a if float type */
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OMOD:
    case OASMOD:
        a = AMOD;
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OLSHR:
    case OASLSHR:
        a = ASRL;
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OLMUL:
    case OASLMUL:
        a = AMULU;
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OLMOD:
    case OASLMOD:
        a = AMODU;
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OLDIV:
    case OASLDIV:
        a = ADIVU;
        break;
    /*x: [[gopcode()]] switch opcode cases */
    case OAS:
        gmove(f1, t);
        return;
    /*x: [[gopcode()]] switch opcode cases */
    case OFUNC:
        a = ABL;
        break;
    /*x: [[gopcode()]] switch opcode cases */
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
            if(typefd[et] && !btrue)
                a = ABMI;
            break;
        case OLE:
            a = ABLE;
            if(typefd[et] && !btrue)
                a = ABLS;
            break;
        case OGE:
            a = ABGE;
            if(typefd[et] && btrue)
                a = ABPL;
            break;
        case OGT:
            a = ABGT;
            if(typefd[et] && btrue)
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
    /*e: [[gopcode()]] switch opcode cases */
    }
    /*s: [[gopcode()]] sanity check a */
    if(a == AGOK)
        diag(Z, "bad in gopcode %O", o);
    /*e: [[gopcode()]] sanity check a */

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

    /*s: [[gopcode()]] debug */
    if(debug['g'])
        print("%P\n", p);
    /*e: [[gopcode()]] debug */
}
/*e: function [[gopcode]](arm) */

/*s: function [[samaddr]] */
bool
samaddr(Node *f, Node *t)
{

    if(f->op != t->op)
        return false;
    switch(f->op) {
    case OREGISTER:
        return (f->reg == t->reg);
    }
    return false;
}
/*e: function [[samaddr]] */

/*s: function [[gbranch]](arm) */
void
gbranch(int o)
{
    int a;

    nextpc();

    switch(o) {
    case ORETURN: a = ARET; break;
    case OGOTO:   a = AB;   break;
    default:
        a = AGOK;
        diag(Z, "bad in gbranch %O",  o);
        nextpc();
    }
    p->as = a;
}
/*e: function [[gbranch]](arm) */

/*s: function [[patch]] */
void
patch(Prog *op, long pc)
{

    op->to.type = D_BRANCH;
    op->to.offset = pc;
}
/*e: function [[patch]] */

/*s: function [[gpseudo]](arm) */
void
gpseudo(int a, Sym *s, Node *n)
{

    nextpc();

    p->as = a;
    p->from.type = D_OREG;
    p->from.sym = s;
    p->from.symkind = (s->class == CSTATIC) ? N_INTERN : N_EXTERN;
    /*s: [[gpseudo()]] if TEXT, set possible TEXT attributes */
    if(a == ATEXT)
        p->reg = (profileflg ? 0 : NOPROF);
    /*e: [[gpseudo()]] if TEXT, set possible TEXT attributes */
    naddr(n, &p->to);
    if(a == ADATA || a == AGLOBL)
        pc--;
}
/*e: function [[gpseudo]](arm) */

/*s: function [[sconst]](arm) */
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
/*e: function [[sconst]](arm) */

/*s: function [[sval]](arm) */
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
/*e: function [[sval]](arm) */

/*s: function [[exreg]](arm) */
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
/*e: function [[exreg]](arm) */

/*s: global [[ewidth]] */
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
/*e: global [[ewidth]] */

/*s: global [[ncast]] */
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
/*e: global [[ncast]] */
/*e: 5c/txt.c */
