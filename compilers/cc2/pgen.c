/*s: cc2/pgen.c */
#include "gc.h"

void	gen(Node*);
void	usedset(Node*, int);
int		bcomplex(Node*, Node*);

/*s: function codgen */
//@Scheck: used by cc.y
void codgen(Node *n, Node *nn)
{
    Prog *sp;
    Node *n1, nod, nod1;

    /*s: [[codgen()]] initialisation */
    hasdoubled = false;
    /*x: [[codgen()]] initialisation */
    cursafe = 0;
    curarg = 0;
    maxargsafe = 0;
    /*e: [[codgen()]] initialisation */

    /*s: [[codgen()]] set n1 node to node in nn where have ONAME */
    /*
     * isolate name
     */
    for(n1 = nn;; n1 = n1->left) {
        if(n1 == Z) {
            diag(nn, "cant find function name");
            return;
        }
        if(n1->op == ONAME)
            break;
    }
    /*e: [[codgen()]] set n1 node to node in nn where have ONAME */

    nearln = nn->lineno;
    gpseudo(ATEXT, n1->sym, nodconst(stkoff));
    sp = p;

    /*s: [[codgen()]] if complex return type */
    if(typecmplx[thisfntype->link->etype]) {
        if(nodret == nil) {
            nodret = new(ONAME, Z, Z);
            nodret->sym = slookup(".ret");
            nodret->class = CPARAM;
            nodret->type = types[TIND];
            nodret->etype = TIND;
            nodret = new(OIND, nodret, Z);
        }
        n1 = nodret->left;
        if(n1->type == T || n1->type->link != thisfntype->link) {
            n1->type = typ(TIND, thisfntype->link);
            n1->etype = n1->type->etype; // TIND
            nodret = new(OIND, n1, Z);
            complex(nodret);
        }
    }
    /*e: [[codgen()]] if complex return type */
    /*s: [[codgen()]] if use REGARG */
    /*
     * isolate first argument
     */
    if(REGARG >= 0) {	
        if(typecmplx[thisfntype->link->etype]) {
            nod1 = *nodret->left;
            nodreg(&nod, &nod1, REGARG);
            gmove(&nod, &nod1);
        }
        else
        if(firstarg && typeword[firstargtype->etype]) {
            nod1 = znode;
            nod1.op = ONAME;
            nod1.sym = firstarg;
            nod1.type = firstargtype;
            nod1.class = CPARAM;
            nod1.xoffset = align(0, firstargtype, Aarg1);
            nod1.etype = firstargtype->etype;
            xcom(&nod1);
            nodreg(&nod, &nod1, REGARG);
            gmove(&nod, &nod1);
        }
    }
    /*e: [[codgen()]] if use REGARG */

    /*s: [[codgen()]] initialisation before call to gen */
    canreach = true;
    warnreach = true;
    /*e: [[codgen()]] initialisation before call to gen */

    // generate the assembly for the statements in the body
    gen(n);

    /*s: [[codgen()]] warn for possible missing return after call to gen */
    if(canreach && thisfntype->link->etype != TVOID){
        if(debug['B'])
            warn(Z, "no return at end of function: %s", n1->sym->name);
        else
            diag(Z, "no return at end of function: %s", n1->sym->name);
    }
    /*e: [[codgen()]] warn for possible missing return after call to gen */

    noretval(1 | 2);

    gbranch(ORETURN);

    if(!debug['N'] || debug['R'] || debug['P'])
        regopt(sp);
    
    if(thechar=='6' || thechar=='7' || thechar=='9' || hasdoubled)	/* [sic] */
        maxargsafe = round(maxargsafe, 8);
    sp->to.offset += maxargsafe;
}
/*e: function codgen */

/*s: function supgen */
void
supgen(Node *n)
{
    bool owarn;
    long opc;
    Prog *olastp;

    if(n == Z)
        return;

    suppress++;
    owarn = warnreach;
    opc = pc;
    olastp = lastp;

    warnreach = false;

    gen(n);

    lastp = olastp;
    olastp->link = nil;
    pc = opc;
    warnreach = owarn;
    suppress--;
}
/*e: function supgen */

/*s: function uncomma */
Node*
uncomma(Node *n)
{
    while(n != Z && n->op == OCOMMA) {
        cgen(n->left, Z);
        n = n->right;
    }
    return n;
}
/*e: function uncomma */

/*s: function gen */
void
gen(Node *n)
{
    /*s: [[gen()]] locals */
    // enum<node_kind> of a statement
    int o;
    /*x: [[gen()]] locals */
    Prog *sp;
    bool oldreach;
    Node *l;
    bool err;
    /*x: [[gen()]] locals */
    bool f;
    /*x: [[gen()]] locals */
    Prog *spc, *spb;
    long sbc, scc;
    int snbreak, sncontin;
    /*x: [[gen()]] locals */
    Node nod;
    Node rn;
    /*x: [[gen()]] locals */
    Case *cn;
    /*e: [[gen()]] locals */

loop:
    if(n == Z)
        return;

    nearln = n->lineno;
    o = n->op;

    if(debug['G'] && (o != OLIST))
        print("%L %O\n", nearln, o);

    /*s: [[gen()]] if not canreach */
    if(!canreach) {
        switch(o) {
        case OLABEL:
        case OCASE:
        case OLIST:
        case OCOMMA:
        case OBREAK:
        case OFOR:
        case OWHILE:
        case ODWHILE:
            /* all handled specially - see switch body below */
            break;
        default:
            if(warnreach) {
                warn(n, "unreachable code %O", o);
                warnreach = false;
            }
        }
    }
    /*e: [[gen()]] if not canreach */

    switch(o) {
    /*s: [[gen()]] switch node kind cases */
    case OLIST:
    case OCOMMA:
        gen(n->left);

    rloop:
        n = n->right;
        goto loop;
    /*x: [[gen()]] switch node kind cases */
    case OIF:
        l = n->left;
        err = bcomplex(l, n->right);
        /*s: [[gen()]] switch node kind cases, OIF case, if bcomplex error */
        if(err) {
            if(typefd[l->type->etype])
                f = !l->fconst;
            else
                f = !l->vconst;
            if(debug['c'])
                print("%L const if %s\n", nearln, f ? "false" : "true");

            if(f) {
                canreach = true;
                supgen(n->right->left);
                oldreach = canreach;
                canreach = true;
                gen(n->right->right);
                /*
                 * treat constant ifs as regular ifs for 
                 * reachability warnings.
                 */
                if(!canreach && oldreach && debug['w'] < 2)
                    warnreach = 0;
            } else {
                canreach = true;
                gen(n->right->left);
                oldreach = canreach;
                canreach = true;
                supgen(n->right->right);
                /*
                 * treat constant ifs as regular ifs for 
                 * reachability warnings.
                 */
                if(!oldreach && canreach && debug['w'] < 2)
                    warnreach = 0;
                canreach = oldreach;
            }
        }
        /*e: [[gen()]] switch node kind cases, OIF case, if bcomplex error */
        else {
            sp = p;
            canreach = true;

            if(n->right->left != Z)
                gen(n->right->left);

            oldreach = canreach;
            canreach = true;

            if(n->right->right != Z) {
                gbranch(OGOTO);
                patch(sp, pc);
                sp = p;

                gen(n->right->right);
            }
            patch(sp, pc);

            canreach = canreach || oldreach;
            if(canreach == false)
                warnreach = !suppress;
        }
        break;
    /*x: [[gen()]] switch node kind cases */
    case OWHILE:
    case ODWHILE:
        l = n->left;

        gbranch(OGOTO);		/* entry */
        sp = p;

        scc = continpc;
        continpc = pc;

        gbranch(OGOTO);
        spc = p;

        sbc = breakpc;
        breakpc = pc;
        snbreak = nbreak;
        nbreak = 0;

        gbranch(OGOTO);
        spb = p;

        patch(spc, pc);
        if(n->op == OWHILE)
            patch(sp, pc);
        bcomplex(l, Z);		/* test */
        patch(p, breakpc);
        if(l->op != OCONST || vconst(l) == 0)
            nbreak++;

        if(n->op == ODWHILE)
            patch(sp, pc);
        gen(n->right);		/* body */

        gbranch(OGOTO);
        patch(p, continpc);

        patch(spb, pc);

        continpc = scc;
        breakpc = sbc;
        canreach = nbreak!=0;
        if(canreach == false)
            warnreach = !suppress;
        nbreak = snbreak;
        break;
    /*x: [[gen()]] switch node kind cases */
    case OFOR:
        l = n->left;
        if(!canreach && l->right->left && warnreach) {
            warn(n, "unreachable code FOR");
            warnreach = 0;
        }
        gen(l->right->left);	/* init */

        gbranch(OGOTO);		/* entry */
        sp = p;

        /* 
         * if there are no incoming labels in the 
         * body and the top's not reachable, warn
         */
        if(!canreach && warnreach && deadheads(n)) {
            warn(n, "unreachable code %O", o);
            warnreach = 0;
        }

        scc = continpc;
        continpc = pc;

        gbranch(OGOTO);
        spc = p;

        sbc = breakpc;
        breakpc = pc;
        snbreak = nbreak;
        nbreak = 0;
        sncontin = ncontin;
        ncontin = 0;

        gbranch(OGOTO);
        spb = p;

        patch(spc, pc);
        gen(l->right->right);	/* inc */
        patch(sp, pc);	
        if(l->left != Z) {	/* test */
            bcomplex(l->left, Z);
            patch(p, breakpc);
            if(l->left->op != OCONST || vconst(l->left) == 0)
                nbreak++;
        }
        canreach = true;
        gen(n->right);		/* body */
        if(canreach){
            gbranch(OGOTO);
            patch(p, continpc);

            ncontin++;
        }
        if(!ncontin && l->right->right && warnreach) {
            warn(l->right->right, "unreachable FOR inc");
            warnreach = 0;
        }

        patch(spb, pc);
        continpc = scc;
        breakpc = sbc;
        canreach = nbreak!=0;
        if(canreach == false)
            warnreach = !suppress;
        nbreak = snbreak;
        ncontin = sncontin;
        break;
    /*x: [[gen()]] switch node kind cases */
    case ORETURN:
        canreach = false;
        warnreach = !suppress;

        complex(n);
        if(n->type == T)
            break;
        l = uncomma(n->left);
        if(l == Z) {
            noretval(3);
            gbranch(ORETURN);
            break;
        }
        if(typecmplx[n->type->etype]) {
            nod = znode;
            nod.op = OAS;
            nod.left = nodret;
            nod.right = l;
            nod.type = n->type;
            nod.complex = l->complex;
            cgen(&nod, Z);
            noretval(3);
            gbranch(ORETURN);
            break;
        }
        if(newvlongcode && !typefd[n->type->etype]){
            regret(&rn, n);
            regfree(&rn);
            nod = znode;
            nod.op = OAS;
            nod.left = &rn;
            nod.right = l;
            nod.type = n->type;
            nod.complex = l->complex;
            cgen(&nod, Z);
            noretval(2);
            gbranch(ORETURN);
            break;
        }
        regret(&nod, n);
        cgen(l, &nod);
        regfree(&nod);
        if(typefd[n->type->etype])
            noretval(1);
        else
            noretval(2);
        gbranch(ORETURN);
        break;
    /*x: [[gen()]] switch node kind cases */
    case OCONTINUE:
        if(continpc < 0) {
            diag(n, "continue not in a loop");
            break;
        }
        gbranch(OGOTO);
        patch(p, continpc);

        ncontin++;
        canreach = false;
        warnreach = !suppress;
        break;
    /*x: [[gen()]] switch node kind cases */
    case OBREAK:
        if(breakpc < 0) {
            diag(n, "break not in a loop");
            break;
        }
        /*
         * Don't complain about unreachable break statements.
         * There are breaks hidden in yacc's output and some people
         * write return; break; in their switch statements out of habit.
         * However, don't confuse the analysis by inserting an 
         * unreachable reference to breakpc either.
         */
        if(!canreach)
            break;

        gbranch(OGOTO);
        patch(p, breakpc);

        nbreak++;
        canreach = false;
        warnreach = !suppress;
        break;
    /*x: [[gen()]] switch node kind cases */
    case OSWITCH:
        l = n->left;
        complex(l);
        if(l->type == T)
            break;
        if(!typeswitch[l->type->etype]) {
            diag(n, "switch expression must be integer");
            break;
        }

        gbranch(OGOTO);		/* entry */
        sp = p;

        cn = cases;
        cases = C;
        casf();

        sbc = breakpc;
        breakpc = pc;
        snbreak = nbreak;
        nbreak = 0;

        gbranch(OGOTO);
        spb = p;

        gen(n->right);		/* body */
        if(canreach){
            gbranch(OGOTO);
            patch(p, breakpc);

            nbreak++;
        }

        patch(sp, pc);
        regalloc(&nod, l, Z);
        /* always signed */
        if(typev[l->type->etype])
            nod.type = types[TVLONG];
        else
            nod.type = types[TLONG];
        cgen(l, &nod);
        doswit(&nod);
        regfree(&nod);
        patch(spb, pc);

        cases = cn;
        breakpc = sbc;
        canreach = nbreak!=0;
        if(canreach == false)
            warnreach = !suppress;
        nbreak = snbreak;
        break;
    /*x: [[gen()]] switch node kind cases */
    case OCASE:
        canreach = true;
        l = n->left;
        if(cases == C)
            diag(n, "case/default outside a switch");
        if(l == Z) {
            casf();
            cases->val = 0;
            cases->def = 1;
            cases->label = pc;
            cases->isv = 0;
            goto rloop;
        }
        complex(l);
        if(l->type == T)
            goto rloop;
        if(l->op != OCONST || !typeswitch[l->type->etype]) {
            diag(n, "case expression must be integer constant");
            goto rloop;
        }
        casf();
        cases->val = l->vconst;
        cases->def = 0;
        cases->label = pc;
        cases->isv = typev[l->type->etype];
        goto rloop;
    /*x: [[gen()]] switch node kind cases */
    case OLABEL:
        canreach = true;
        l = n->left;
        if(l) {
            l->pc = pc;
            if(l->label)
                patch(l->label, pc);
        }
        gbranch(OGOTO);	/* prevent self reference in reg */
        patch(p, pc);

        goto rloop;
    /*x: [[gen()]] switch node kind cases */
    case OGOTO:
        canreach = false;
        warnreach = !suppress;

        n = n->left;
        if(n == Z) // possible?
            return;
        if(n->complex == 0) {
            diag(Z, "label undefined: %s", n->sym->name);
            return;
        }
        if(suppress)
            return;

        gbranch(OGOTO);
        if(n->pc) {
            patch(p, n->pc);
        } else {
            if(n->label)
                 patch(n->label, pc-1);
            n->label = p;
        }
        return;
    /*x: [[gen()]] switch node kind cases */
    default:
        complex(n);
        cgen(n, Z);
        break;
    /*x: [[gen()]] switch node kind cases */
    case OSET:
    case OUSED:
        usedset(n->left, o);
        break;
    /*e: [[gen()]] switch node kind cases */
    }
}
/*e: function gen */

/*s: function usedset */
void
usedset(Node *n, int o)
{
    if(n->op == OLIST) {
        usedset(n->left, o);
        usedset(n->right, o);
        return;
    }

    complex(n);

    switch(n->op) {
    case OADDR:	/* volatile */
        gins(ANOP, n, Z);
        break;
    case ONAME:
        if(o == OSET)
            gins(ANOP, Z, n);
        else
            gins(ANOP, n, Z);
        break;
    }
}
/*e: function usedset */

/*s: function bcomplex */
bool
bcomplex(Node *n, Node *c)
{

    complex(n);

    if(n->type != T)
       if(tcompat(n, T, n->type, tnot))
          n->type = T;
    if(n->type == T) {
        gbranch(OGOTO);
        return false;
    }
    if(c != Z && n->op == OCONST && deadheads(c))
        return true;

    bool64(n);
    boolgen(n, true, Z);
    return false;
}
/*e: function bcomplex */
/*e: cc2/pgen.c */
