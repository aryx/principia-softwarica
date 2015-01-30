/*s: 5c/cgen.c */
#include "gc.h"

/*s: function cgen(arm) */
void
cgen(Node *n, Node *nn)
{
    cgenrel(n, nn, false);
}
/*e: function cgen(arm) */

/*s: function cgenrel(arm) */
void
cgenrel(Node *n, Node *nn, bool inrel)
{
    Node *l, *r;
    // enum<node_kind>
    int o;
    // enum<type_kind>?
    int t;
    long curs;
    /*s: [[cgenrel()]] locals */
    Prog *p1;
    Node nod, nod1, nod2, nod3, nod4;
    long v;
    /*e: [[cgenrel()]] locals */

    if(debug['g']) {
        prtree(nn, "cgen lhs");
        prtree(n, "cgen");
    }
    if(n == Z || n->type == T)
        return;

    /*s: [[cgenrel()]] if complex type node */
    if(typesuv[n->type->etype]) {
        sugen(n, nn, n->type->width);
    }
    /*e: [[cgenrel()]] if complex type node */
    else {

        l = n->left;
        r = n->right;
        o = n->op;
        
        /*s: [[cgenrel()]] if addable more than INDEXED */
        if(n->addable >= INDEXED) {
            if(nn == Z) {
                switch(o) {
                default:
                    nullwarn(Z, Z);
                    break;
                case OINDEX:
                    nullwarn(l, r);
                    break;
                }
                return;
            }
            gmove(n, nn);
            return;
        }
        /*e: [[cgenrel()]] if addable more than INDEXED */
        curs = cursafe;
        /*s: [[cgenrel()]] if all complex fields more than FNX */
        if(n->complex >= FNX)
         if(l->complex >= FNX)
          if(r != Z && r->complex >= FNX)
            switch(o) {
            case OFUNC:
            case OCOMMA:
            case OANDAND:
            case OOROR:
            case OCOND:
            case ODOT:
                break;
    
            default:
                regret(&nod, r);
                cgen(r, &nod);
    
                regsalloc(&nod1, r);
                gopcode(OAS, &nod, Z, &nod1);
    
                regfree(&nod);
                nod = *n;
                nod.right = &nod1;
                cgen(&nod, nn);
                return;
    
            }
        /*e: [[cgenrel()]] if all complex fields more than FNX */
        
        switch(o) {
        /*s: [[cgenrel()]] switch node kind cases */
        case OCOMMA:
            cgen(l, Z);
            cgen(r, nn);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OSUB:
            if(nn != Z)
             if(l->op == OCONST)
              if(!typefd[n->type->etype]) {
                cgen(r, nn);
                gopcode(o, Z, l, nn);
                break;
            }
            // Fallthrough
        case OADD:
        case OAND:
        case OOR:
        case OXOR:
        case OLSHR:
        case OASHL:
        case OASHR:
            /*
             * immediate operands
             */
            if(nn != Z)
             if(r->op == OCONST)
              if(!typefd[n->type->etype]) {
                cgen(l, nn);
                if((r->vconst == 0) && (o != OAND))
                    break;
                if(nn != Z)
                    gopcode(o, r, Z, nn);
                break;
            }
            // Fallthrough
        case OLMUL:
        case OLDIV:
        case OLMOD:
        case OMUL:
        muldiv:
            /*s: [[cgenrel()]] nullwarn check if nn is null */
            if(nn == Z) {
                nullwarn(l, r);
                break;
            }
            /*e: [[cgenrel()]] nullwarn check if nn is null */
            if(o == OMUL || o == OLMUL) {
                if(mulcon(n, nn))
                    break;
            }
            if(l->complex >= r->complex) {
                regalloc(&nod, l, nn);
                cgen(l, &nod);
                regalloc(&nod1, r, Z);
                cgen(r, &nod1);
                gopcode(o, &nod1, Z, &nod);
            } else {
                regalloc(&nod, r, nn);
                cgen(r, &nod);
                regalloc(&nod1, l, Z);
                cgen(l, &nod1);
                gopcode(o, &nod, &nod1, &nod);
            }
            gopcode(OAS, &nod, Z, nn);
            regfree(&nod);
            regfree(&nod1);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case ODIV:
        case OMOD:
            if(nn != Z)
             if((t = vlog(r)) >= 0) {
                /* signed div/mod by constant power of 2 */
                cgen(l, nn);
                gopcode(OGE, nodconst(0), nn, Z);
                p1 = p;
                if(o == ODIV) {
                    gopcode(OADD, nodconst((1<<t)-1), Z, nn);
                    patch(p1, pc);
                    gopcode(OASHR, nodconst(t), Z, nn);
                } else {
                    gopcode(OSUB, nn, nodconst(0), nn);
                    gopcode(OAND, nodconst((1<<t)-1), Z, nn);
                    gopcode(OSUB, nn, nodconst(0), nn);
                    gbranch(OGOTO);
                    patch(p1, pc);
                    p1 = p;
                    gopcode(OAND, nodconst((1<<t)-1), Z, nn);
                    patch(p1, pc);
                }
                break;
            }
            goto muldiv;
        /*x: [[cgenrel()]] switch node kind cases */
        case OANDAND:
        case OOROR:
            boolgen(n, true, nn);
            if(nn == Z)
                patch(p, pc);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case ONOT:
            /*s: [[cgenrel()]] nullwarn check if nn is null, empty right */
            if(nn == Z) {
                nullwarn(l, Z);
                break;
            }
            /*e: [[cgenrel()]] nullwarn check if nn is null, empty right */
            boolgen(n, true, nn);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OEQ:
        case ONE:
        case OLE:
        case OLT:
        case OGE:
        case OGT:
        case OLO:
        case OLS:
        case OHI:
        case OHS:
            /*s: [[cgenrel()]] nullwarn check if nn is null */
            if(nn == Z) {
                nullwarn(l, r);
                break;
            }
            /*e: [[cgenrel()]] nullwarn check if nn is null */
            boolgen(n, true, nn);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OAS:
            if(l->op == OBIT)
                goto bitas;
            if(l->addable >= INDEXED && l->complex < FNX) {
                if(nn != Z || r->addable < INDEXED) {
                    if(r->complex >= FNX && nn == Z)
                        regret(&nod, r);
                    else
                        regalloc(&nod, r, nn);
                    cgen(r, &nod);
                    gmove(&nod, l);
                    if(nn != Z)
                        gmove(&nod, nn);
                    regfree(&nod);
                } else
                    gmove(r, l);
                break;
            }
            if(l->complex >= r->complex) {
                reglcgen(&nod1, l, Z);
                if(r->addable >= INDEXED) {
                    gmove(r, &nod1);
                    if(nn != Z)
                        gmove(r, nn);
                    regfree(&nod1);
                    break;
                }
                regalloc(&nod, r, nn);
                cgen(r, &nod);
            } else {
                regalloc(&nod, r, nn);
                cgen(r, &nod);
                reglcgen(&nod1, l, Z);
            }
            gmove(&nod, &nod1);
            regfree(&nod);
            regfree(&nod1);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OASLSHR:
        case OASASHL:
        case OASASHR:
        case OASAND:
        case OASADD:
        case OASSUB:
        case OASXOR:
        case OASOR:
            if(l->op == OBIT)
                goto asbitop;
            if(r->op == OCONST)
             if(!typefd[r->type->etype])
              if(!typefd[n->type->etype]) {
                if(l->addable < INDEXED)
                    reglcgen(&nod2, l, Z);
                else
                    nod2 = *l;
                regalloc(&nod, r, nn);
                gopcode(OAS, &nod2, Z, &nod);
                gopcode(o, r, Z, &nod);
                gopcode(OAS, &nod, Z, &nod2);

                regfree(&nod);
                if(l->addable < INDEXED)
                    regfree(&nod2);
                break;
            }
            // Fallthrough
        case OASLMUL:
        case OASLDIV:
        case OASLMOD:
        case OASMUL:
        case OASDIV:
        case OASMOD:
            if(l->op == OBIT)
                goto asbitop;
            if(l->complex >= r->complex) {
                if(l->addable < INDEXED)
                    reglcgen(&nod2, l, Z);
                else
                    nod2 = *l;
                regalloc(&nod1, r, Z);
                cgen(r, &nod1);
            } else {
                regalloc(&nod1, r, Z);
                cgen(r, &nod1);
                if(l->addable < INDEXED)
                    reglcgen(&nod2, l, Z);
                else
                    nod2 = *l;
            }

            regalloc(&nod, n, nn);
            gmove(&nod2, &nod);
            gopcode(o, &nod1, Z, &nod);
            gmove(&nod, &nod2);
            if(nn != Z)
                gopcode(OAS, &nod, Z, nn);
            regfree(&nod);
            regfree(&nod1);
            if(l->addable < INDEXED)
                regfree(&nod2);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OADDR:
            /*s: [[cgenrel()]] nullwarn check if nn is null, empty right */
            if(nn == Z) {
                nullwarn(l, Z);
                break;
            }
            /*e: [[cgenrel()]] nullwarn check if nn is null, empty right */
            lcgen(l, nn);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OIND:
            /*s: [[cgenrel()]] nullwarn check if nn is null, empty right */
            if(nn == Z) {
                nullwarn(l, Z);
                break;
            }
            /*e: [[cgenrel()]] nullwarn check if nn is null, empty right */
            regialloc(&nod, n, nn);
            r = l;
            while(r->op == OADD)
                r = r->right;
            if(sconst(r) && (v = r->vconst+nod.xoffset) > -4096 && v < 4096) {
                v = r->vconst;
                r->vconst = 0;
                cgen(l, &nod);
                nod.xoffset += v;
                r->vconst = v;
            } else
                cgen(l, &nod);
            regind(&nod, n);
            gopcode(OAS, &nod, Z, nn);
            regfree(&nod);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OFUNC:
            if(l->complex >= FNX) {
                if(l->op != OIND)
                    diag(n, "bad function call");

                regret(&nod, l->left);
                cgen(l->left, &nod);
                regsalloc(&nod1, l->left);
                gopcode(OAS, &nod, Z, &nod1);
                regfree(&nod);

                nod = *n;
                nod.left = &nod2;
                nod2 = *l;
                nod2.left = &nod1;
                nod2.complex = 1;
                cgen(&nod, nn);

                return;
            }
            /*s: [[cgenrel()]] OFUNC case, if use REGARG, part one */
            if(REGARG >= 0)
                o = reg[REGARG];
            /*e: [[cgenrel()]] OFUNC case, if use REGARG, part one */
            gargs(r, &nod, &nod1);
            if(l->addable < INDEXED) {
                reglcgen(&nod, l, Z);
                gopcode(OFUNC, Z, Z, &nod);
                regfree(&nod);
            } else
                gopcode(OFUNC, Z, Z, l);
            /*s: [[cgenrel()]] OFUNC case, if use REGARG, part two */
            if(REGARG >= 0)
                if(o != reg[REGARG])
                    reg[REGARG]--;
            /*e: [[cgenrel()]] OFUNC case, if use REGARG, part two */
            if(nn != Z) {
                regret(&nod, n);
                gopcode(OAS, &nod, Z, nn);
                regfree(&nod);
            }
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case ODOT:
            sugen(l, nodrat, l->type->width);
            if(nn != Z) {
                warn(n, "non-interruptable temporary");
                nod = *nodrat;
                if(!r || r->op != OCONST) {
                    diag(n, "DOT and no offset");
                    break;
                }
                nod.xoffset += (long)r->vconst;
                nod.type = n->type;
                cgen(&nod, nn);
            }
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OCAST:
            /*s: [[cgenrel()]] nullwarn check if nn is null, empty right */
            if(nn == Z) {
                nullwarn(l, Z);
                break;
            }
            /*e: [[cgenrel()]] nullwarn check if nn is null, empty right */
            /*
             * convert from types l->n->nn
             */
            if(nocast(l->type, n->type)) {
                if(nocast(n->type, nn->type)) {
                    cgen(l, nn);
                    break;
                }
            }
            regalloc(&nod, l, nn);
            cgen(l, &nod);
            regalloc(&nod1, n, &nod);
            if(inrel)
                gmover(&nod, &nod1);
            else
                gopcode(OAS, &nod, Z, &nod1);
            gopcode(OAS, &nod1, Z, nn);
            regfree(&nod1);
            regfree(&nod);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OCOND:
            bcgen(l, true);
            p1 = p;
            cgen(r->left, nn);
            gbranch(OGOTO);
            patch(p1, pc);
            p1 = p;
            cgen(r->right, nn);
            patch(p1, pc);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OPOSTINC:
        case OPOSTDEC:
            v = 1;
            if(l->type->etype == TIND)
                v = l->type->link->width;
            if(o == OPOSTDEC)
                v = -v;
            if(l->op == OBIT)
                goto bitinc;
            if(nn == Z)
                goto pre;

            if(l->addable < INDEXED)
                reglcgen(&nod2, l, Z);
            else
                nod2 = *l;

            regalloc(&nod, l, nn);
            gopcode(OAS, &nod2, Z, &nod);
            regalloc(&nod1, l, Z);
            if(typefd[l->type->etype]) {
                regalloc(&nod3, l, Z);
                if(v < 0) {
                    gopcode(OAS, nodfconst(-v), Z, &nod3);
                    gopcode(OSUB, &nod3, &nod, &nod1);
                } else {
                    gopcode(OAS, nodfconst(v), Z, &nod3);
                    gopcode(OADD, &nod3, &nod, &nod1);
                }
                regfree(&nod3);
            } else
                gopcode(OADD, nodconst(v), &nod, &nod1);
            gopcode(OAS, &nod1, Z, &nod2);

            regfree(&nod);
            regfree(&nod1);
            if(l->addable < INDEXED)
                regfree(&nod2);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OPREINC:
        case OPREDEC:
            v = 1;
            if(l->type->etype == TIND)
                v = l->type->link->width;
            if(o == OPREDEC)
                v = -v;
            if(l->op == OBIT)
                goto bitinc;

        pre:
            if(l->addable < INDEXED)
                reglcgen(&nod2, l, Z);
            else
                nod2 = *l;

            regalloc(&nod, l, nn);
            gopcode(OAS, &nod2, Z, &nod);
            if(typefd[l->type->etype]) {
                regalloc(&nod3, l, Z);
                if(v < 0) {
                    gopcode(OAS, nodfconst(-v), Z, &nod3);
                    gopcode(OSUB, &nod3, Z, &nod);
                } else {
                    gopcode(OAS, nodfconst(v), Z, &nod3);
                    gopcode(OADD, &nod3, Z, &nod);
                }
                regfree(&nod3);
            } else
                gopcode(OADD, nodconst(v), Z, &nod);
            gopcode(OAS, &nod, Z, &nod2);
            if(nn && l->op == ONAME)	/* in x=++i, emit USED(i) */
                gins(ANOP, l, Z);

            regfree(&nod);
            if(l->addable < INDEXED)
                regfree(&nod2);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        case OBIT:
            /*s: [[cgenrel()]] nullwarn check if nn is null, empty right */
            if(nn == Z) {
                nullwarn(l, Z);
                break;
            }
            /*e: [[cgenrel()]] nullwarn check if nn is null, empty right */
            bitload(n, &nod, Z, Z, nn);
            gopcode(OAS, &nod, Z, nn);
            regfree(&nod);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        asbitop:
            regalloc(&nod4, n, nn);
            if(l->complex >= r->complex) {
                bitload(l, &nod, &nod1, &nod2, &nod4);
                regalloc(&nod3, r, Z);
                cgen(r, &nod3);
            } else {
                regalloc(&nod3, r, Z);
                cgen(r, &nod3);
                bitload(l, &nod, &nod1, &nod2, &nod4);
            }
            gmove(&nod, &nod4);
            gopcode(o, &nod3, Z, &nod4);
            regfree(&nod3);
            gmove(&nod4, &nod);
            regfree(&nod4);
            bitstore(l, &nod, &nod1, &nod2, nn);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        bitinc:
            if(nn != Z && (o == OPOSTINC || o == OPOSTDEC)) {
                bitload(l, &nod, &nod1, &nod2, Z);
                gopcode(OAS, &nod, Z, nn);
                gopcode(OADD, nodconst(v), Z, &nod);
                bitstore(l, &nod, &nod1, &nod2, Z);
                break;
            }
            bitload(l, &nod, &nod1, &nod2, nn);
            gopcode(OADD, nodconst(v), Z, &nod);
            bitstore(l, &nod, &nod1, &nod2, nn);
            break;
        /*x: [[cgenrel()]] switch node kind cases */
        bitas:
            n = l->left;
            regalloc(&nod, r, nn);
            if(l->complex >= r->complex) {
                reglcgen(&nod1, n, Z);
                cgen(r, &nod);
            } else {
                cgen(r, &nod);
                reglcgen(&nod1, n, Z);
            }
            regalloc(&nod2, n, Z);
            gopcode(OAS, &nod1, Z, &nod2);
            bitstore(l, &nod, &nod1, &nod2, nn);
            break;
        /*e: [[cgenrel()]] switch node kind cases */
        default:
            diag(n, "unknown op in cgen: %O", o);
            break;
        }
        cursafe = curs;
    }
    return;
}
/*e: function cgenrel(arm) */

/*s: function reglcgen(arm) */
void
reglcgen(Node *t, Node *n, Node *nn)
{
    Node *r;
    long v;

    regialloc(t, n, nn);
    if(n->op == OIND) {
        r = n->left;
        while(r->op == OADD)
            r = r->right;
        if(sconst(r) && (v = r->vconst+t->xoffset) > -4096 && v < 4096) {
            v = r->vconst;
            r->vconst = 0;
            lcgen(n, t);
            t->xoffset += v;
            r->vconst = v;
            regind(t, n);
            return;
        }
    } else if(n->op == OINDREG) {
        if((v = n->xoffset) > -4096 && v < 4096) {
            n->op = OREGISTER;
            cgen(n, t);
            t->xoffset += v;
            n->op = OINDREG;
            regind(t, n);
            return;
        }
    }
    lcgen(n, t);
    regind(t, n);
}
/*e: function reglcgen(arm) */

/*s: function reglpcgen(arm) */
void
reglpcgen(Node *n, Node *nn, int f)
{
    Type *t;

    t = nn->type;
    nn->type = types[TLONG];
    if(f)
        reglcgen(n, nn, Z);
    else {
        regialloc(n, nn, Z);
        lcgen(nn, n);
        regind(n, nn);
    }
    nn->type = t;
}
/*e: function reglpcgen(arm) */

/*s: function lcgen(arm) */
void
lcgen(Node *n, Node *nn)
{
    Prog *p1;
    Node nod;

    if(debug['g']) {
        prtree(nn, "lcgen lhs");
        prtree(n, "lcgen");
    }
    if(n == Z || n->type == T)
        return;
    if(nn == Z) {
        nn = &nod;
        regalloc(&nod, n, Z);
    }
    switch(n->op) {
    case OCOMMA:
        cgen(n->left, n->left);
        lcgen(n->right, nn);
        break;

    case OIND:
        cgen(n->left, nn);
        break;

    case OCOND:
        bcgen(n->left, 1);
        p1 = p;
        lcgen(n->right->left, nn);
        gbranch(OGOTO);
        patch(p1, pc);
        p1 = p;
        lcgen(n->right->right, nn);
        patch(p1, pc);
        break;

    default:
        if(n->addable < INDEXED) {
            diag(n, "unknown op in lcgen: %O", n->op);
            break;
        }
        nod = *n;
        nod.op = OADDR;
        nod.left = n;
        nod.right = Z;
        nod.type = types[TIND];
        gopcode(OAS, &nod, Z, nn);
        break;

    }
}
/*e: function lcgen(arm) */

/*s: function bcgen */
void
bcgen(Node *n, bool btrue)
{

    if(n->type == T)
        gbranch(OGOTO);
    else
        boolgen(n, btrue, Z);
}
/*e: function bcgen */

/*s: function boolgen(arm) */
void
boolgen(Node *n, int btrue, Node *nn)
{
    int o;
    Prog *p1, *p2;
    Node *l, *r, nod, nod1;
    long curs;

    if(debug['g']) {
        prtree(nn, "boolgen lhs");
        prtree(n, "boolgen");
    }
    curs = cursafe;

    l = n->left;
    r = n->right;
    switch(n->op) {

    case OCONST:
        o = vconst(n);
        if(!btrue)
            o = !o;
        gbranch(OGOTO);
        if(o) {
            p1 = p;
            gbranch(OGOTO);
            patch(p1, pc);
        }
        goto com;

    case OCOMMA:
        cgen(l, Z);
        boolgen(r, btrue, nn);
        break;

    case ONOT:
        boolgen(l, !btrue, nn);
        break;

    case OCOND:
        bcgen(l, 1);
        p1 = p;
        bcgen(r->left, btrue);
        p2 = p;
        gbranch(OGOTO);
        patch(p1, pc);
        p1 = p;
        bcgen(r->right, !btrue);
        patch(p2, pc);
        p2 = p;
        gbranch(OGOTO);
        patch(p1, pc);
        patch(p2, pc);
        goto com;

    case OANDAND:
        if(!btrue)
            goto caseor;

    caseand:
        bcgen(l, btrue);
        p1 = p;
        bcgen(r, !btrue);
        p2 = p;
        patch(p1, pc);
        gbranch(OGOTO);
        patch(p2, pc);
        goto com;

    case OOROR:
        if(!btrue)
            goto caseand;

    caseor:
        bcgen(l, !btrue);
        p1 = p;
        bcgen(r, !btrue);
        p2 = p;
        gbranch(OGOTO);
        patch(p1, pc);
        patch(p2, pc);
        goto com;

    case OEQ:
    case ONE:
    case OLE:
    case OLT:
    case OGE:
    case OGT:
    case OHI:
    case OHS:
    case OLO:
    case OLS:
        o = n->op;
        if(btrue)
            o = comrel[relindex(o)];
        if(l->complex >= FNX && r->complex >= FNX) {
            regret(&nod, r);
            cgenrel(r, &nod, btrue);
            regsalloc(&nod1, r);
            gopcode(OAS, &nod, Z, &nod1);
            regfree(&nod);
            nod = *n;
            nod.right = &nod1;
            boolgen(&nod, btrue, nn);
            break;
        }
        if(sconst(l)) {
            regalloc(&nod, r, nn);
            cgenrel(r, &nod, btrue);
            o = invrel[relindex(o)];
            gopcode(btrue ? o | BTRUE : o, l, &nod, Z);
            regfree(&nod);
            goto com;
        }
        if(sconst(r)) {
            regalloc(&nod, l, nn);
            cgenrel(l, &nod, btrue);
            gopcode(btrue ? o | BTRUE : o, r, &nod, Z);
            regfree(&nod);
            goto com;
        }

        if(l->complex >= r->complex) {
            regalloc(&nod1, l, nn);
            cgenrel(l, &nod1, btrue);
            regalloc(&nod, r, Z);
            cgenrel(r, &nod, btrue);
        } else {
            regalloc(&nod, r, nn);
            cgenrel(r, &nod, btrue);
            regalloc(&nod1, l, Z);
            cgenrel(l, &nod1, btrue);
        }
        gopcode(btrue ? o | BTRUE : o, &nod, &nod1, Z);
        regfree(&nod);
        regfree(&nod1);

    com:
        if(nn != Z) {
            p1 = p;
            gopcode(OAS, nodconst(1), Z, nn);
            gbranch(OGOTO);
            p2 = p;
            patch(p1, pc);
            gopcode(OAS, nodconst(0), Z, nn);
            patch(p2, pc);
        }
        break;

    default:
        regalloc(&nod, n, nn);
        cgen(n, &nod);
        o = ONE;
        if(btrue)
            o = comrel[relindex(o)];
        if(typefd[n->type->etype]) {
            gopcode(btrue ? o | BTRUE : o, nodfconst(0), &nod, Z);
        } else
            gopcode(o, nodconst(0), &nod, Z);
        regfree(&nod);
        goto com;

    }
    cursafe = curs;
}
/*e: function boolgen(arm) */

/*s: function sugen(arm) */
void
sugen(Node *n, Node *nn, long w)
{
    Prog *p1;
    Node nod0, nod1, nod2, nod3, nod4, *l, *r;
    Type *t;
    long pc1;
    int i, m, c;

    if(n == Z || n->type == T)
        return;

    if(debug['g']) {
        prtree(nn, "sugen lhs");
        prtree(n, "sugen");
    }
    if(nn == nodrat)
        if(w > nrathole)
            nrathole = w;

    switch(n->op) {
    case OIND:
        if(nn == Z) {
            nullwarn(n->left, Z);
            break;
        }
    // Fallthrough

    default:
        goto copy;

    case OCONST:
        if(n->type && typev[n->type->etype]) {
            if(nn == Z) {
                nullwarn(n->left, Z);
                break;
            }

            t = nn->type;
            nn->type = types[TLONG];
            reglcgen(&nod1, nn, Z);
            nn->type = t;

            if(align(0, types[TCHAR], Aarg1))	/* isbigendian */
                gopcode(OAS, nod32const(n->vconst>>32), Z, &nod1);
            else
                gopcode(OAS, nod32const(n->vconst), Z, &nod1);
            nod1.xoffset += SZ_LONG;
            if(align(0, types[TCHAR], Aarg1))	/* isbigendian */
                gopcode(OAS, nod32const(n->vconst), Z, &nod1);
            else
                gopcode(OAS, nod32const(n->vconst>>32), Z, &nod1);

            regfree(&nod1);
            break;
        }
        goto copy;

    case ODOT:
        l = n->left;
        sugen(l, nodrat, l->type->width);
        if(nn != Z) {
            warn(n, "non-interruptable temporary");
            nod1 = *nodrat;
            r = n->right;
            if(!r || r->op != OCONST) {
                diag(n, "DOT and no offset");
                break;
            }
            nod1.xoffset += (long)r->vconst;
            nod1.type = n->type;
            sugen(&nod1, nn, w);
        }
        break;

    // struct constructor extension?
    case OSTRUCT:
        /*
         * rewrite so lhs has no fn call
         */
        if(nn != Z && nn->complex >= FNX) {
            nod1 = *n;
            nod1.type = typ(TIND, n->type);
            regret(&nod2, &nod1);
            lcgen(nn, &nod2);
            regsalloc(&nod0, &nod1);
            gopcode(OAS, &nod2, Z, &nod0);
            regfree(&nod2);

            nod1 = *n;
            nod1.op = OIND;
            nod1.left = &nod0;
            nod1.right = Z;
            nod1.complex = 1;

            sugen(n, &nod1, w);
            return;
        }

        r = n->left;
        for(t = n->type->link; t != T; t = t->down) {
            l = r;
            if(r->op == OLIST) {
                l = r->left;
                r = r->right;
            }
            if(nn == Z) {
                cgen(l, nn);
                continue;
            }
            /*
             * hand craft *(&nn + o) = l
             */
            nod0 = znode;
            nod0.op = OAS;
            nod0.type = t;
            nod0.left = &nod1;
            nod0.right = l;

            nod1 = znode;
            nod1.op = OIND;
            nod1.type = t;
            nod1.left = &nod2;

            nod2 = znode;
            nod2.op = OADD;
            nod2.type = typ(TIND, t);
            nod2.left = &nod3;
            nod2.right = &nod4;

            nod3 = znode;
            nod3.op = OADDR;
            nod3.type = nod2.type;
            nod3.left = nn;

            nod4 = znode;
            nod4.op = OCONST;
            nod4.type = nod2.type;
            nod4.vconst = t->offset;

            ccom(&nod0);
            acom(&nod0);
            xcom(&nod0);
            nod0.addable = 0;

            cgen(&nod0, Z);
        }
        break;

    case OAS:
        if(nn == Z) {
            if(n->addable < INDEXED)
                sugen(n->right, n->left, w);
            break;
        }
        sugen(n->right, nodrat, w);
        warn(n, "non-interruptable temporary");
        sugen(nodrat, n->left, w);
        sugen(nodrat, nn, w);
        break;

    case OFUNC:
        if(nn == Z) {
            sugen(n, nodrat, w);
            break;
        }
        if(nn->op != OIND) {
            nn = new1(OADDR, nn, Z);
            nn->type = types[TIND];
            nn->addable = 0;
        } else
            nn = nn->left;
        n = new(OFUNC, n->left, new(OLIST, nn, n->right));
        n->type = types[TVOID];
        n->left->type = types[TVOID];
        cgen(n, Z);
        break;

    case OCOND:
        bcgen(n->left, 1);
        p1 = p;
        sugen(n->right->left, nn, w);
        gbranch(OGOTO);
        patch(p1, pc);
        p1 = p;
        sugen(n->right->right, nn, w);
        patch(p1, pc);
        break;

    case OCOMMA:
        cgen(n->left, Z);
        sugen(n->right, nn, w);
        break;
    }
    return;

copy:
    if(nn == Z)
        return;
    if(n->complex >= FNX && nn->complex >= FNX) {
        t = nn->type;
        nn->type = types[TLONG];
        regialloc(&nod1, nn, Z);
        lcgen(nn, &nod1);
        regsalloc(&nod2, nn);
        nn->type = t;

        gopcode(OAS, &nod1, Z, &nod2);
        regfree(&nod1);

        nod2.type = typ(TIND, t);

        nod1 = nod2;
        nod1.op = OIND;
        nod1.left = &nod2;
        nod1.right = Z;
        nod1.complex = 1;
        nod1.type = t;

        sugen(n, &nod1, w);
        return;
    }

    w /= SZ_LONG;
    if(w <= 2) {
        if(n->complex > nn->complex) {
            reglpcgen(&nod1, n, 1);
            reglpcgen(&nod2, nn, 1);
        } else {
            reglpcgen(&nod2, nn, 1);
            reglpcgen(&nod1, n, 1);
        }
        regalloc(&nod3, &regnode, Z);
        regalloc(&nod4, &regnode, Z);
        if(nod3.reg > nod4.reg){
            /* code below assumes nod3 loaded first */
            Node t = nod3; nod3 = nod4; nod4 = t;
        }
        nod0 = *nodconst((1<<nod3.reg)|(1<<nod4.reg));
        if(w == 2 && nod1.xoffset == 0)
            gmovm(&nod1, &nod0, 0);
        else {
            gmove(&nod1, &nod3);
            if(w == 2) {
                nod1.xoffset += SZ_LONG;
                gmove(&nod1, &nod4);
            }
        }
        if(w == 2 && nod2.xoffset == 0)
            gmovm(&nod0, &nod2, 0);
        else {
            gmove(&nod3, &nod2);
            if(w == 2) {
                nod2.xoffset += SZ_LONG;
                gmove(&nod4, &nod2);
            }
        }
        regfree(&nod1);
        regfree(&nod2);
        regfree(&nod3);
        regfree(&nod4);
        return;
    }

    if(n->complex > nn->complex) {
        reglpcgen(&nod1, n, 0);
        reglpcgen(&nod2, nn, 0);
    } else {
        reglpcgen(&nod2, nn, 0);
        reglpcgen(&nod1, n, 0);
    }

    m = 0;
    for(c = 0; c < w && c < 4; c++) {
        i = tmpreg();
        if (i == 0)
            break;
        reg[i]++;
        m |= 1<<i;
    }
    nod4 = *(nodconst(m));
    if(w < 3*c) {
        for (; w>c; w-=c) {
            gmovm(&nod1, &nod4, 1);
            gmovm(&nod4, &nod2, 1);
        }
        goto out;
    }

    regalloc(&nod3, &regnode, Z);
    gopcode(OAS, nodconst(w/c), Z, &nod3);
    w %= c;
    
    pc1 = pc;
    gmovm(&nod1, &nod4, 1);
    gmovm(&nod4, &nod2, 1);

    gopcode(OSUB, nodconst(1), Z, &nod3);
    gopcode(OEQ, nodconst(0), &nod3, Z);
    p->as = ABGT;
    patch(p, pc1);
    regfree(&nod3);

out:
    if (w) {
        i = 0;
        while (c>w) {
            while ((m&(1<<i)) == 0)
                i++;
            m &= ~(1<<i);
            reg[i] = 0;
            c--;
            i++;
        }
        nod4.vconst = m;
        gmovm(&nod1, &nod4, 0);
        gmovm(&nod4, &nod2, 0);
    }
    i = 0;
    do {
        while ((m&(1<<i)) == 0)
            i++;
        reg[i] = 0;
        c--;
        i++;
    } while (c>0);
    regfree(&nod1);
    regfree(&nod2);
}
/*e: function sugen(arm) */
/*e: 5c/cgen.c */
