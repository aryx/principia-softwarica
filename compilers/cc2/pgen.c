/*s: cc2/pgen.c */
#include "gc.h"

void	gen(Node*);
void	usedset(Node*, int);
int		bcomplex(Node*, Node*);

/*s: function [[codgen]] */
//@Scheck: used by cc.y
void codgen(Node *n, Node *nn)
{
    Prog *sp;
    /*s: [[codgen()]] locals */
    Node *n1;
    /*x: [[codgen()]] locals */
    Node nod, nod1;
    /*e: [[codgen()]] locals */

    /*s: [[codgen()]] initialisation */
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
        n1 = nodret->left;
        if(n1->type == T || n1->type->link != thisfntype->link) {
            n1->type = typ(TIND, thisfntype->link);
            n1->etype = n1->type->etype; // TIND, dead instruction
            nodret = new(OIND, n1, Z); // useful? to force retyping?
            complex(nodret);
        }
    }
    /*e: [[codgen()]] if complex return type */
    /*s: [[codgen()]] if use REGARG */
    /*
     * isolate first argument
     */
    if(REGARG >= 0) {	
        /*s: [[codegen()]] if use REGARG, if complex return type */
        if(typecmplx[thisfntype->link->etype]) {
            nod1 = *nodret->left;
            nodreg(&nod, &nod1, REGARG);
            gmove(&nod, &nod1);
        }
        /*e: [[codegen()]] if use REGARG, if complex return type */
        else
        if(firstarg && typeword[firstargtype->etype]) {
            nod1 = znode;
            nod1.op = ONAME;
            nod1.sym = firstarg;
            nod1.type = firstargtype;
            nod1.etype = firstargtype->etype;
            nod1.class = CPARAM;
            nod1.xoffset = align(0, firstargtype, Aarg1);
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
    /*s: [[codgen()]] before RET */
    noretval(1 | 2);
    /*e: [[codgen()]] before RET */
    gbranch(ORETURN);

    /*s: [[codgen()]] register optimisation */
    if(!debug['N'] || debug['R'] || debug['P'])
        regopt(sp);
    /*e: [[codgen()]] register optimisation */
    
    sp->to.offset += maxargsafe;
}
/*e: function [[codgen]] */

/*s: function [[supgen]] */
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
/*e: function [[supgen]] */

/*s: function [[uncomma]] */
Node*
uncomma(Node *n)
{
    while(n != Z && n->op == OCOMMA) {
        cgen(n->left, Z);
        n = n->right;
    }
    return n;
}
/*e: function [[uncomma]] */

/*s: function [[gen]] */
void
gen(Node *n)
{
    // enum<Node_kind> of a statement
    int o;
    /*s: [[gen()]] locals */
    bool f;
    /*x: [[gen()]] locals */
    Prog *sp;
    Node *l;
    bool err;
    /*x: [[gen()]] locals */
    bool oldreach;
    /*x: [[gen()]] locals */
    Prog *spc, *spb;
    /*x: [[gen()]] locals */
    long sbc, scc;
    int snbreak, sncontin;
    /*x: [[gen()]] locals */
    Node nod;
    /*x: [[gen()]] locals */
    Case *cn;
    /*e: [[gen()]] locals */

loop:
    if(n == Z)
        return;

    nearln = n->lineno;
    o = n->op;
    /*s: [[gen()]] debug opcode */
    if(debug['G'] && (o != OLIST))
        print("%L %O\n", nearln, o);
    /*e: [[gen()]] debug opcode */
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
    case OSET:
    case OUSED:
        usedset(n->left, o);
        break;
    /*x: [[gen()]] switch node kind cases */
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
                    warnreach = false;
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
                    warnreach = false;
                canreach = oldreach;
            }
        }
        /*e: [[gen()]] switch node kind cases, OIF case, if bcomplex error */
        else {
            sp = p; // AB instr for 'else' to patch later (created by bcomplex())
            /*s: [[gen()]] when OIF, before gen then part, set canreach */
            canreach = true;
            /*e: [[gen()]] when OIF, before gen then part, set canreach */

            if(n->right->left != Z) // gen then part
                gen(n->right->left); 

            /*s: [[gen()]] when OIF, before gen else part, save and set canreach */
            oldreach = canreach;
            canreach = true;
            /*e: [[gen()]] when OIF, before gen else part, save and set canreach */

            if(n->right->right != Z) { // gen else part
                gbranch(OGOTO);
                patch(sp, pc);
                sp = p;
                gen(n->right->right);
            }
            patch(sp, pc);

            /*s: [[gen()]] when OIF, after gen everything, set canreach */
            canreach = canreach || oldreach;
            if(!canreach)
                warnreach = !suppress;
            /*e: [[gen()]] when OIF, after gen everything, set canreach */
        }
        break;
    /*x: [[gen()]] switch node kind cases */
    case OLABEL:
        /*s: [[gen()]] when LABEL, set canreach */
        canreach = true;
        /*e: [[gen()]] when LABEL, set canreach */
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
        /*s: [[gen()]] when OGOTO/ORETURN/OBREAK/OCONTINUE, set canreach */
        canreach = false;
        warnreach = !suppress;
        /*e: [[gen()]] when OGOTO/ORETURN/OBREAK/OCONTINUE, set canreach */
        n = n->left;
        /*s: [[gen()]] when OGOTO, sanity check n */
        if(n == Z) // possible?
            return;
        if(n->complex == 0) {
            diag(Z, "label undefined: %s", n->sym->name);
            return;
        }
        if(suppress)
            return;
        /*e: [[gen()]] when OGOTO, sanity check n */
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
    case OWHILE:
    case ODWHILE:
        l = n->left;

        gbranch(OGOTO);		/* entry */
        sp = p;

        /*s: [[gen()]] when OWHILE/ODWHILE, set continpc to pc before goto continue */
        scc = continpc;
        continpc = pc;
        /*e: [[gen()]] when OWHILE/ODWHILE, set continpc to pc before goto continue */
        gbranch(OGOTO); // goto for continue
        spc = p;

        /*s: [[gen()]] when OWHILE/ODWHILE, set breakpc to pc before goto break */
        sbc = breakpc;
        snbreak = nbreak;
        breakpc = pc;
        nbreak = 0;
        /*e: [[gen()]] when OWHILE/ODWHILE, set breakpc to pc before goto break */
        gbranch(OGOTO); // goto for break
        spb = p;

        patch(spc, pc);
        if(n->op == OWHILE)
            patch(sp, pc);

        bcomplex(l, Z);		/* test */
        patch(p, breakpc);
        /*s: [[gen()]] when OWHILE/ODWHILE, increment nbreak */
        if(l->op != OCONST || vconst(l) == 0)
            nbreak++;
        /*e: [[gen()]] when OWHILE/ODWHILE, increment nbreak */

        if(n->op == ODWHILE)
            patch(sp, pc);

        gen(n->right);		/* body */

        gbranch(OGOTO);
        patch(p, continpc);

        patch(spb, pc); // exit

        /*s: [[gen()]] when OWHILE/ODWHILE, before exit, restore continpc/breakpc */
        // restore
        continpc = scc;
        breakpc = sbc;
        /*s: [[gen()]] when OWHILE/ODWHILE, set canreach */
        canreach = nbreak!=0;
        if(!canreach)
             warnreach = !suppress;
        /*e: [[gen()]] when OWHILE/ODWHILE, set canreach */
        nbreak = snbreak;
        /*e: [[gen()]] when OWHILE/ODWHILE, before exit, restore continpc/breakpc */
        break;
    /*x: [[gen()]] switch node kind cases */
    case OFOR:
        l = n->left;
        /*s: [[gen()]] when OFOR, check canreach before gen anything */
        if(!canreach && warnreach && l->right->left) {
            warn(n, "unreachable code FOR");
            warnreach = false;
        }
        /*e: [[gen()]] when OFOR, check canreach before gen anything */

        gen(l->right->left);	/* init */

        gbranch(OGOTO);		/* entry */
        sp = p;

        /*s: [[gen()]] when OFOR, check canreach after gen init */
        /* 
         * if there are no incoming labels in the 
         * body and the top's not reachable, warn
         */
        if(!canreach && warnreach && deadheads(n)) {
            warn(n, "unreachable code %O", o);
            warnreach = false;
        }
        /*e: [[gen()]] when OFOR, check canreach after gen init */

        /*s: [[gen()]] when OFOR, set continpc to pc before goto continue */
        scc = continpc;
        continpc = pc;
        /*e: [[gen()]] when OFOR, set continpc to pc before goto continue */
        gbranch(OGOTO); // goto for continue
        spc = p;

        /*s: [[gen()]] when OFOR, set breakpc to pc before goto break */
        sbc = breakpc;
        snbreak = nbreak;
        sncontin = ncontin;
        breakpc = pc;
        nbreak = 0;
        ncontin = 0;
        /*e: [[gen()]] when OFOR, set breakpc to pc before goto break */
        gbranch(OGOTO); // goto for break
        spb = p;

        patch(spc, pc);

        gen(l->right->right);	/* inc */

        patch(sp, pc); // entry

        if(l->left != Z) {	/* test */
            bcomplex(l->left, Z);
            patch(p, breakpc);
            /*s: [[gen()]] when OFOR, increment nbreak */
            if(l->left->op != OCONST || vconst(l->left) == 0)
                nbreak++;
            /*e: [[gen()]] when OFOR, increment nbreak */
        }

        /*s: [[gen()]] when OFOR, set canreach before gen body */
        canreach = true;
        /*e: [[gen()]] when OFOR, set canreach before gen body */

        gen(n->right);		/* body */

        if(canreach){
            gbranch(OGOTO);
            patch(p, continpc);
            /*s: [[gen()]] when OFOR, increment ncontin when canreach end of body */
            ncontin++;
            /*e: [[gen()]] when OFOR, increment ncontin when canreach end of body */
        }
        /*s: [[gen()]] when OFOR, check can reach for inc */
        if(!ncontin && l->right->right && warnreach) {
            warn(l->right->right, "unreachable FOR inc");
            warnreach = false;
        }
        /*e: [[gen()]] when OFOR, check can reach for inc */

        patch(spb, pc);

        /*s: [[gen()]] when OFOR, before exit, restore continpc/breakpc */
        // restore
        continpc = scc;
        breakpc = sbc;
        /*s: [[gen()]] when OFOR, set canreach after gen */
        canreach = nbreak!=0;
        if(!canreach)
            warnreach = !suppress;
        /*e: [[gen()]] when OFOR, set canreach after gen */
        nbreak = snbreak;
        ncontin = sncontin;
        /*e: [[gen()]] when OFOR, before exit, restore continpc/breakpc */
        break;
    /*x: [[gen()]] switch node kind cases */
    case OCONTINUE:
        /*s: [[gen()]] when OCONTINUE, sanity check continpc */
        if(continpc < 0) {
            diag(n, "continue not in a loop");
            break;
        }
        /*e: [[gen()]] when OCONTINUE, sanity check continpc */
        gbranch(OGOTO);
        patch(p, continpc);
        ncontin++;
        /*s: [[gen()]] when OGOTO/ORETURN/OBREAK/OCONTINUE, set canreach */
        canreach = false;
        warnreach = !suppress;
        /*e: [[gen()]] when OGOTO/ORETURN/OBREAK/OCONTINUE, set canreach */
        break;
    /*x: [[gen()]] switch node kind cases */
    case OBREAK:
        /*s: [[gen()]] when OBREAK, sanity check breakpc */
        if(breakpc < 0) {
            diag(n, "break not in a loop");
            break;
        }
        /*e: [[gen()]] when OBREAK, sanity check breakpc */
        /*s: [[gen()]] when OBREAK, check canreach */
        /*
         * Don't complain about unreachable break statements.
         * There are breaks hidden in yacc's output and some people
         * write return; break; in their switch statements out of habit.
         * However, don't confuse the analysis by inserting an 
         * unreachable reference to breakpc either.
         */
        if(!canreach)
            break;
        /*e: [[gen()]] when OBREAK, check canreach */
        gbranch(OGOTO);
        patch(p, breakpc);
        nbreak++;
        /*s: [[gen()]] when OGOTO/ORETURN/OBREAK/OCONTINUE, set canreach */
        canreach = false;
        warnreach = !suppress;
        /*e: [[gen()]] when OGOTO/ORETURN/OBREAK/OCONTINUE, set canreach */
        break;
    /*x: [[gen()]] switch node kind cases */
    case ORETURN:
        /*s: [[gen()]] when OGOTO/ORETURN/OBREAK/OCONTINUE, set canreach */
        canreach = false;
        warnreach = !suppress;
        /*e: [[gen()]] when OGOTO/ORETURN/OBREAK/OCONTINUE, set canreach */

        complex(n);
        /*s: [[gen()]] when ORETURN, after complex, if no type */
        if(n->type == T)
            break;
        /*e: [[gen()]] when ORETURN, after complex, if no type */

        l = uncomma(n->left);
        if(l == Z) {
            /*s: [[gen()]] case ORETURN with no argument, before RET */
            noretval(1 | 2);
            /*e: [[gen()]] case ORETURN with no argument, before RET */
            gbranch(ORETURN);
        } 
        else
        /*s: [[gen()]] case ORETURN, if complex type */
        if(typecmplx[n->type->etype]) {
            nod = znode;
            nod.op = OAS;
            nod.left = nodret;
            nod.right = l;
            nod.type = n->type;
            nod.complex = l->complex;
            cgen(&nod, Z);
            noretval(1 | 2);
            gbranch(ORETURN);
            break;
        }
        /*e: [[gen()]] case ORETURN, if complex type */
        else {
            regret(&nod, n);
            cgen(l, &nod);
            regfree(&nod);
            /*s: [[gen()]] case ORETURN with argument, before RET */
            if(typefd[n->type->etype])
                noretval(1);
            else
                noretval(2);
            /*e: [[gen()]] case ORETURN with argument, before RET */
            gbranch(ORETURN);
        }
        break;
    /*x: [[gen()]] switch node kind cases */
    default:
        complex(n);
        cgen(n, Z);
        break;
    /*x: [[gen()]] switch node kind cases */
    case OSWITCH:
        l = n->left;
        complex(l);
        if(l->type == T)
            break;
        /*s: [[gen()]] when OSWITCH, typecheck condition */
        if(!typeswitch[l->type->etype]) {
            diag(n, "switch expression must be integer");
            break;
        }
        /*e: [[gen()]] when OSWITCH, typecheck condition */

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
    /*e: [[gen()]] switch node kind cases */
    }
}
/*e: function [[gen]] */

/*s: function [[usedset]] */
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
    case ONAME:
        if(o == OSET)
            gins(ANOP, Z, n);
        else
            gins(ANOP, n, Z);
        break;
    /*s: [[usedset()]] switch node op cases */
    case OADDR:	/* volatile */
        gins(ANOP, n, Z);
        break;
    /*e: [[usedset()]] switch node op cases */
    }
}
/*e: function [[usedset]] */

/*s: function [[bcomplex]] */
bool
bcomplex(Node *n, Node *c)
{

    complex(n);
    /*s: [[bcomplex()]] typecheck n */
    if(n->type != T)
       if(tcompat(n, T, n->type, tnot))
          n->type = T;
    /*e: [[bcomplex()]] typecheck n */
    /*s: [[bcomplex()]] generate fake goto if type error */
    if(n->type == T) {
        gbranch(OGOTO);
        return false;
    }
    /*e: [[bcomplex()]] generate fake goto if type error */
    /*s: [[bcomplex()]] check for deadheads */
    if(c != Z && n->op == OCONST && deadheads(c))
        return true;
    /*e: [[bcomplex()]] check for deadheads */

    /*s: [[bcomplex()]] possibly convert node to funcall if unsupported 64 bit op */
    bool64(n);
    /*e: [[bcomplex()]] possibly convert node to funcall if unsupported 64 bit op */
    boolgen(n, true, Z);

    return false; // no error
}
/*e: function [[bcomplex]] */
/*e: cc2/pgen.c */
