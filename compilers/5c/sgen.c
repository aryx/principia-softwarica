/*s: 5c/sgen.c */
#include "gc.h"

/*s: function noretval(arm) */
void
noretval(int n)
{

    if(n & 1) {
        gins(ANOP, Z, Z);
        p->to.type = D_REG;
        p->to.reg = REGRET;
    }
    if(n & 2) {
        gins(ANOP, Z, Z);
        p->to.type = D_FREG;
        p->to.reg = FREGRET;
    }
}
/*e: function noretval(arm) */

/*s: function xcom(arm) */
/*
 *	calculate addressability as follows
 *		CONST ==> 20		$value
 *		NAME ==> 10		name
 *		REGISTER ==> 11		register
 *		INDREG ==> 12		*[(reg)+offset]
 *
 *		&10 ==> 2		$name
 *		ADD(2, 20) ==> 2	$name+offset
 *		ADD(3, 20) ==> 3	$(reg)+offset
 *		&12 ==> 3		$(reg)+offset
 *
 *		*11 ==> 11		??
 *		*2 ==> 10		name
 *		*3 ==> 12		*(reg)+offset
 *	calculate complexity (number of registers) //$
 */
void
xcom(Node *n)
{
    Node *l, *r;
    int t;

    if(n == Z)
        return;

    l = n->left;
    r = n->right;
    n->addable = 0;
    n->complex = 0;

    switch(n->op) {
    /*s: [[xcom()]] switch op cases to set addable */
    case OCONST:
        n->addable = 20;
        return;
    /*x: [[xcom()]] switch op cases to set addable */
    case ONAME:
        n->addable = 10;
        return;
    /*x: [[xcom()]] switch op cases to set addable */
    case OREGISTER:
        n->addable = 11;
        return;
    /*x: [[xcom()]] switch op cases to set addable */
    case OINDREG:
        n->addable = 12;
        return;
    /*x: [[xcom()]] switch op cases to set addable */
    case OADDR:
        xcom(l);
        if(l->addable == 10)
            n->addable = 2;

        if(l->addable == 12)
            n->addable = 3;
        break;

    /*x: [[xcom()]] switch op cases to set addable */
    case OIND:
        xcom(l);
        if(l->addable == 2)
            n->addable = 10;

        if(l->addable == 3)
            n->addable = 12;
        if(l->addable == 11)
            n->addable = 12;
        break;
    /*x: [[xcom()]] switch op cases to set addable */
    case OADD:
        xcom(l);
        xcom(r);
        if(l->addable == 20) {
            if(r->addable == 2)
                n->addable = 2;

            if(r->addable == 3)
                n->addable = 3;
        }
        if(r->addable == 20) {
            if(l->addable == 2)
                n->addable = 2;

            if(l->addable == 3)
                n->addable = 3;
        }
        break;
    /*x: [[xcom()]] switch op cases to set addable */
    case OMUL:
    case OLMUL:
        xcom(l);
        xcom(r);
        t = vlog(r);
        if(t >= 0) {
            n->op = OASHL;
            r->vconst = t;
            r->type = types[TINT];
        }
        t = vlog(l);
        if(t >= 0) {
            n->op = OASHL;
            n->left = r;
            n->right = l;
            r = l;
            l = n->left;
            r->vconst = t;
            r->type = types[TINT];
        }
        break;
    /*x: [[xcom()]] switch op cases to set addable */
    case OASLMUL:
    case OASMUL:
        xcom(l);
        xcom(r);
        t = vlog(r);
        if(t >= 0) {
            n->op = OASASHL;
            r->vconst = t;
            r->type = types[TINT];
        }
        break;
    /*x: [[xcom()]] switch op cases to set addable */
    case OLDIV:
        xcom(l);
        xcom(r);
        t = vlog(r);
        if(t >= 0) {
            n->op = OLSHR;
            r->vconst = t;
            r->type = types[TINT];
        }
        break;
    /*x: [[xcom()]] switch op cases to set addable */
    case OASLDIV:
        xcom(l);
        xcom(r);
        t = vlog(r);
        if(t >= 0) {
            n->op = OASLSHR;
            r->vconst = t;
            r->type = types[TINT];
        }
        break;
    /*x: [[xcom()]] switch op cases to set addable */
    case OLMOD:
        xcom(l);
        xcom(r);
        t = vlog(r);
        if(t >= 0) {
            n->op = OAND;
            r->vconst--;
        }
        break;
    /*x: [[xcom()]] switch op cases to set addable */
    case OASLMOD:
        xcom(l);
        xcom(r);
        t = vlog(r);
        if(t >= 0) {
            n->op = OASAND;
            r->vconst--;
        }
        break;
    /*e: [[xcom()]] switch op cases to set addable */
    default:
        if(l != Z)
            xcom(l);
        if(r != Z)
            xcom(r);
        break;
    }
    if(n->addable >= 10)
        return;
    // else
    /*s: [[xcom()]] set complex if addable less than 10 */
    if(l != Z)
        n->complex = l->complex;
    if(r != Z) {
        if(r->complex == n->complex)
            n->complex = r->complex+1;
        else
           if(r->complex > n->complex)
               n->complex = r->complex;
    }

    if(n->complex == 0)
        n->complex++;

    /*s: [[xcom()]] if 64 bits operation, transform and return */
    if(com64(n))
        return;
    /*e: [[xcom()]] if 64 bits operation, transform and return */
    // else
    switch(n->op) {
    /*s: [[xcom()]] switch node opkind cases to set complexity */
    case OFUNC:
        n->complex = FNX;
        break;
    /*x: [[xcom()]] switch node opkind cases to set complexity */
    case OADD:
    case OAND:
    case OOR:
    case OXOR:
    case OEQ:
    case ONE:
        /*
         * immediate operators, make const on right
         */
        if(l->op == OCONST) {
            n->left = r;
            n->right = l;
        }
        break;
    /*e: [[xcom()]] switch node opkind cases to set complexity */
    }
    /*e: [[xcom()]] set complex if addable less than 10 */
}
/*e: function xcom(arm) */
/*e: 5c/sgen.c */
