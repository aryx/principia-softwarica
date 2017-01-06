/*s: cc/com.c */
#include "cc.h"

int	tcomd(Node*);
int	tcomx(Node*);
int	tlvalue(Node*);

typedef struct Com Com;
typedef struct Big Big;

/*s: struct Com */
struct Com
{
    int	n;
    Node	*t[500];
};
/*e: struct Com */

int compar(Node*, int);
static void comma(Node*);
static Node*	commas(Com*, Node*);

/*s: function complex */
void
complex(Node *n)
{

    if(n == Z)
        return;

    nearln = n->lineno;

    /*s: [[complex()]] debug tree pre complex */
    if(debug['t'])
        if(n->op != OCONST)
            prtree(n, "pre complex");
    /*e: [[complex()]] debug tree pre complex */
    // typechecking, removing some sugar, lvalue annotate, bitfield annot, etc
    if(tcom(n))
        return;

    // comma hoisting
    comma(n);
    /*s: [[complex()]] debug tree after tcom and comma */
    if(debug['t'])
        if(n->op != OCONST)
            prtree(n, "t complex");
    /*e: [[complex()]] debug tree after tcom and comma */
    // general rewrite
    ccom(n);
    /*s: [[complex()]] debug tree after ccom */
    if(debug['t'])
        if(n->op != OCONST)
            prtree(n, "c complex");
    /*e: [[complex()]] debug tree after ccom */
    // arithmetic rewrite
    acom(n);
    /*s: [[complex()]] debug tree after acom */
    if(debug['t'])
        if(n->op != OCONST)
            prtree(n, "a complex");
    /*e: [[complex()]] debug tree after acom */

    // addressability and complexity (and shl/shr optimizations)
    xcom(n);
    /*s: [[complex()]] debug tree after xcom */
    if(debug['t'])
        if(n->op != OCONST)
            prtree(n, "x complex");
    /*e: [[complex()]] debug tree after xcom */
}
/*e: function complex */

/*s: enum _anon_ (cc/com.c) */
enum
{
    ADDROF	= 1<<0,
    CASTOF	= 1<<1,
    ADDROP	= 1<<2,
};
/*e: enum _anon_ (cc/com.c) */

/*s: function tcom */
/*
 * evaluate types
 * evaluate lvalues (addable == 1)
 */
bool
tcom(Node *n)
{

    return tcomo(n, ADDROF);
}
/*e: function tcom */

/*s: function tcomo */
bool
tcomo(Node *n, int f)
{
    Node *l, *r;
    Type *t;
    // bool | int
    int o; 
    /*s: [[tcomo()]] other locals */
    static TRune zer;
    /*e: [[tcomo()]] other locals */

    /*s: [[tcomo()]] sanity check n */
    if(n == Z) {
        diag(Z, "Z in tcom");
        errorexit();
    }
    /*e: [[tcomo()]] sanity check n */
    n->addable = false;
    l = n->left;
    r = n->right;

    switch(n->op) {
    /*s: [[tcomo()]] switch node kind cases */
    case OCOMMA:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        n->type = r->type;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OCONST:
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OSTRING:
        /*s: [[tcomo()]] in OSTRING case, if not an array or chars */
        if(n->type->link != types[TCHAR]) {
            o = outstring(0, 0);
            while(o & 3) {
                outstring("", 1);
                o = outstring(0, 0);
            }
        }
        /*e: [[tcomo()]] in OSTRING case, if not an array or chars */
        /*s: [[tcomo()]] transform OSTRING in ONAME */
        n->op = ONAME;
        n->xoffset = outstring(n->cstring, n->type->width);
        n->addable = true;
        /*e: [[tcomo()]] transform OSTRING in ONAME */
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case ONAME:
        /*s: [[tcomo()]] in ONAME case, check if name declared */
        if(n->type == T) {
            diag(n, "name not declared: %F", n);
            goto bad;
        }
        /*e: [[tcomo()]] in ONAME case, check if name declared */
        /*s: [[tcomo()]] in ONAME case, if enum constant */
        if(n->type->etype == TENUM) {
            n->op = OCONST;
            if(!typefd[n->type->etype])
                n->vconst = n->sym->vconst;
            else
                n->fconst = n->sym->fconst;
            n->type = n->sym->tenum;
            break;
        }
        /*e: [[tcomo()]] in ONAME case, if enum constant */
        // else
        n->addable = true;
        /*s: [[tcomo()]] in ONAME case, if extern register */
        if(n->class == CEXREG) {
            n->op = OREGISTER;
            /*s: [[tcomo()]] in ONAME case, if extern register, if x86 */
            if(thechar == '8')
                n->op = OEXREG;
            /*e: [[tcomo()]] in ONAME case, if extern register, if x86 */
            n->reg = n->sym->offset;
            n->xoffset = 0;
            break;
        }
        /*e: [[tcomo()]] in ONAME case, if extern register */
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OADD:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, l->type, r->type, tadd))
            goto bad;
        arith(n, true);
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OSUB:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, l->type, r->type, tsub))
            goto bad;
        arith(n, true);
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OMUL:
    case ODIV:
    case OLMUL:
    case OLDIV:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, l->type, r->type, tmul))
            goto bad;
        arith(n, true);
        /*s: [[tcomo()]] in OMUL/ODIV case, adjust opcode if unsigned type */
        if(typeu[n->type->etype]) {
            if(n->op == OMUL)
                n->op = OLMUL;
            if(n->op == ODIV)
                n->op = OLDIV;
        }
        /*e: [[tcomo()]] in OMUL/ODIV case, adjust opcode if unsigned type */
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OMOD:
    case OLMOD:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, l->type, r->type, tand))
            goto bad;
        arith(n, true);
        /*s: [[tcomo()]] in OMOD case, adjust opcode if unsigned type */
        if(typeu[n->type->etype])
            n->op = OLMOD;
        /*e: [[tcomo()]] in OMOD case, adjust opcode if unsigned type */
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OPOS:
        if(tcom(l))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */

        r = l;
        l = new(OCONST, Z, Z);
        l->vconst = 0;
        l->type = types[TINT];
        n->op = OADD;
        n->right = r;
        n->left = l;

        if(tcom(l))
            goto bad;
        if(tcompat(n, l->type, r->type, tsub))
            goto bad;
        arith(n, true);
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case ONEG:
        if(tcom(l))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */

        if(!machcap(n)) {
            r = l;
            l = new(OCONST, Z, Z);
            l->vconst = 0;
            l->type = types[TINT];
            n->op = OSUB;
            n->right = r;
            n->left = l;

            if(tcom(l))
                goto bad;
            if(tcompat(n, l->type, r->type, tsub))
                goto bad;
        }
        arith(n, true);
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OAND:
    case OOR:
    case OXOR:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, l->type, r->type, tand))
            goto bad;
        arith(n, true);
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OASHL:
    case OASHR:
    case OLSHR:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, l->type, r->type, tand))
            goto bad;

        n->right = Z;
        arith(n, true);

        n->right = new1(OCAST, r, Z);
        n->right->type = types[TINT];
        /*s: [[tcomo()]] in OASHL/OASHR case, adjust opcode if unsigned type */
        if(typeu[n->type->etype])
            if(n->op == OASHR)
                n->op = OLSHR;
        /*e: [[tcomo()]] in OASHL/OASHR case, adjust opcode if unsigned type */
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OEQ:
    case ONE:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        /*s: [[tcomo()]] when OEQ/ONE, typing extensions */
        typeext(l->type, r);
        typeext(r->type, l);
        /*e: [[tcomo()]] when OEQ/ONE, typing extensions */
        if(tcompat(n, l->type, r->type, trel))
            goto bad;
        arith(n, false);

        n->type = types[TINT];
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OLT:
    case OGE:
    case OGT:
    case OLE:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        /*s: [[tcomo()]] when OLT/OGE/OGT/OLE, typing extensions */
        typeext1(l->type, r);
        typeext1(r->type, l);
        /*e: [[tcomo()]] when OLT/OGE/OGT/OLE, typing extensions */
        if(tcompat(n, l->type, r->type, trel))
            goto bad;
        arith(n, false);
        /*s: [[tcomo()]] when OLT/OGE/OGT/OLE, adjust opcode if unsigned type */
        if(typeu[n->type->etype])
            n->op = logrel[relindex(n->op)];
        /*e: [[tcomo()]] when OLT/OGE/OGT/OLE, adjust opcode if unsigned type */
        n->type = types[TINT];
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case ONOT:
        if(tcom(l))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, T, l->type, tnot))
            goto bad;
        n->type = types[TINT];
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OCOM:
        if(tcom(l))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */

        if(!machcap(n)) {
            r = l;
            l = new(OCONST, Z, Z);
            l->vconst = -1;
            l->type = types[TINT];
            n->op = OXOR;
            n->right = r;
            n->left = l;

            if(tcom(l))
                goto bad;
            if(tcompat(n, l->type, r->type, tand))
                goto bad;
        }
        arith(n, false);
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OANDAND:
    case OOROR:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        if(tcompat(n, T, l->type, tnot) |
           tcompat(n, T, r->type, tnot))
            goto bad;
        n->type = types[TINT];
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OAS:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        if(tlvalue(l))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        /*s: [[tcomo()]] when OAS, typing extension */
        typeext(l->type, r);
        /*e: [[tcomo()]] when OAS, typing extension */
        if(tcompat(n, l->type, r->type, tasign))
            goto bad;

        /*s: [[tcomo()]] when OAS, const checking */
        constas(n, l->type, r->type);
        /*e: [[tcomo()]] when OAS, const checking */
        if(!sametype(l->type, r->type)) {
            r = new1(OCAST, r, Z);
            r->type = l->type;
            n->right = r;
        }
        n->type = l->type;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OASI:	/* same as as, but no test for const */
        n->op = OAS;
        o = tcom(l);
        if(o | tcom(r))
            goto bad;

        /*s: [[tcomo()]] when OAS, typing extension */
        typeext(l->type, r);
        /*e: [[tcomo()]] when OAS, typing extension */
        if(tlvalue(l) || tcompat(n, l->type, r->type, tasign))
            goto bad;
        if(!sametype(l->type, r->type)) {
            r = new1(OCAST, r, Z);
            r->type = l->type;
            n->right = r;
        }
        n->type = l->type;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OASADD:
    case OASSUB:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        if(tlvalue(l))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        /*s: [[tcomo()]] when OASADD, typing extension */
        typeext(l->type, r);
        /*e: [[tcomo()]] when OASADD, typing extension */
        if(tcompat(n, l->type, r->type, tasadd))
            goto bad;
        /*s: [[tcomo()]] when OASxxx, const checking */
        constas(n, l->type, r->type);
        /*e: [[tcomo()]] when OASxxx, const checking */
        t = l->type;
        arith(n, false);
        /*s: [[tcomo()]] when OASxxx, adjust node after arith */
        /*s: [[tcomo()]] when OASxxx, remove casts added by arith */
        while(n->left->op == OCAST)
            n->left = n->left->left;
        /*e: [[tcomo()]] when OASxxx, remove casts added by arith */
        if(!sametype(t, n->type) && !mixedasop(t, n->type)) {
            r = new1(OCAST, n->right, Z);
            r->type = t;
            n->right = r;
            n->type = t;
        }
        /*e: [[tcomo()]] when OASxxx, adjust node after arith */
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OASMUL:
    case OASDIV:
    case OASLMUL:
    case OASLDIV:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        if(tlvalue(l))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        /*s: [[tcomo()]] when OASMUL/OASDIV, typing extensions */
        typeext1(l->type, r);
        /*e: [[tcomo()]] when OASMUL/OASDIV, typing extensions */
        if(tcompat(n, l->type, r->type, tmul))
            goto bad;
        /*s: [[tcomo()]] when OASxxx, const checking */
        constas(n, l->type, r->type);
        /*e: [[tcomo()]] when OASxxx, const checking */
        t = l->type;
        arith(n, false);
        /*s: [[tcomo()]] when OASxxx, adjust node after arith */
        /*s: [[tcomo()]] when OASxxx, remove casts added by arith */
        while(n->left->op == OCAST)
            n->left = n->left->left;
        /*e: [[tcomo()]] when OASxxx, remove casts added by arith */
        if(!sametype(t, n->type) && !mixedasop(t, n->type)) {
            r = new1(OCAST, n->right, Z);
            r->type = t;
            n->right = r;
            n->type = t;
        }
        /*e: [[tcomo()]] when OASxxx, adjust node after arith */
        /*s: [[tcomo()]] when OASMUL/OASDIV, adjust opcode if unsigned type */
        if(typeu[n->type->etype]) {
            if(n->op == OASDIV)
                n->op = OASLDIV;
            if(n->op == OASMUL)
                n->op = OASLMUL;
        }
        /*e: [[tcomo()]] when OASMUL/OASDIV, adjust opcode if unsigned type */
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OASLSHR:
    case OASASHR:
    case OASASHL:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        if(tlvalue(l))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, l->type, r->type, tand))
            goto bad;
        n->type = l->type;
        n->right = new1(OCAST, r, Z);
        n->right->type = types[TINT];
        /*s: [[tcomo()]] when OASLSHR, adjust opcode if unsigned type */
        if(typeu[n->type->etype]) {
            if(n->op == OASASHR)
                n->op = OASLSHR;
        }
        /*e: [[tcomo()]] when OASLSHR, adjust opcode if unsigned type */
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OASMOD:
    case OASLMOD:
    case OASOR:
    case OASAND:
    case OASXOR:
        o = tcom(l);
        if(o | tcom(r))
            goto bad;
        if(tlvalue(l))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, l->type, r->type, tand))
            goto bad;
        t = l->type;
        arith(n, false);
        /*s: [[tcomo()]] when OASxxx, adjust node after arith */
        /*s: [[tcomo()]] when OASxxx, remove casts added by arith */
        while(n->left->op == OCAST)
            n->left = n->left->left;
        /*e: [[tcomo()]] when OASxxx, remove casts added by arith */
        if(!sametype(t, n->type) && !mixedasop(t, n->type)) {
            r = new1(OCAST, n->right, Z);
            r->type = t;
            n->right = r;
            n->type = t;
        }
        /*e: [[tcomo()]] when OASxxx, adjust node after arith */
        /*s: [[tcomo()]] when OASMOD, adjust opcode if unsigned type */
        if(typeu[n->type->etype]) {
            if(n->op == OASMOD)
                n->op = OASLMOD;
        }
        /*e: [[tcomo()]] when OASMOD, adjust opcode if unsigned type */
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OADDR:
        if(tcomo(l, ADDROP))
            goto bad;
        if(tlvalue(l))
            goto bad;

        /*s: [[tcomo()]] when OADDR, check if bitfield */
        if(l->type->nbits) {
            diag(n, "address of a bit field");
            goto bad;
        }
        /*e: [[tcomo()]] when OADDR, check if bitfield */
        /*s: [[tcomo()]] when OADDR, check if register */
        if(l->op == OREGISTER) {
            diag(n, "address of a register");
            goto bad;
        }
        /*e: [[tcomo()]] when OADDR, check if register */

        n->type = typ(TIND, l->type);
        n->type->width = types[TIND]->width;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OIND:
        if(tcom(l))
            goto bad;
        if(tcompat(n, T, l->type, tindir))
            goto bad;
        n->type = l->type->link;
        n->addable = true;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case ODOT:
        if(tcom(l))
            goto bad;
        if(tcompat(n, T, l->type, tdot))
            goto bad;
        if(tcomd(n))
            goto bad;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OFUNC:
        o = tcomo(l, 0);
        if(o)
            goto bad;
        /*s: [[tcomo()]] when OFUNC case, add OIND if pointer function call */
        if(l->type->etype == TIND && l->type->link->etype == TFUNC) {
            l = new1(OIND, l, Z);
            l->type = l->left->type->link;
            n->left = l;
        }
        /*e: [[tcomo()]] when OFUNC case, add OIND if pointer function call */
        if(tcompat(n, T, l->type, tfunct))
            goto bad;
        if(o | tcoma(l, r, l->type->down, true))
            goto bad;
        n->type = l->type->link;
        /*s: [[tcomo()]] when OFUNC case, warn function args not checked if old proto */
        if(!debug['B'])
            if(l->type->down == T || l->type->down->etype == TOLD) {
                nerrors--;
                diag(n, "function args not checked: %F", l);
            }
        /*e: [[tcomo()]] when OFUNC case, warn function args not checked if old proto */
        /*s: [[tcomo()]] when OFUNC case, format check */
        dpcheck(n);
        /*e: [[tcomo()]] when OFUNC case, format check */
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case ODOTDOT:
        /*
         * tcom has already been called on this subtree
         */
        *n = *n->left;
        if(n->type == T)
            goto bad;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OCAST:
        if(n->type == T)
            break;
        if(n->type->width == types[TLONG]->width) {
            if(tcomo(l, ADDROF|CASTOF))
                goto bad;
        } else
            if(tcom(l))
                goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, l->type, n->type, tcast))
            goto bad;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OCOND:
        o = tcom(l);
        o |= tcom(r->left);
        if(o | tcom(r->right))
            goto bad;
        /*s: [[tcomo()]] in OCOND, nil handling */
        if(r->right->type->etype == TIND && vconst(r->left) == 0) {
            r->left->type = r->right->type;
            r->left->vconst = 0;
        }
        if(r->left->type->etype == TIND && vconst(r->right) == 0) {
            r->right->type = r->left->type;
            r->right->vconst = 0;
        }
        /*e: [[tcomo()]] in OCOND, nil handling */
        if(sametype(r->right->type, r->left->type)) {
            r->type = r->right->type;
            n->type = r->type;
            break;
        }
        if(tcompat(r, r->left->type, r->right->type, trel))
            goto bad;
        arith(r, false);
        n->type = r->type;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OPREINC:
    case OPREDEC:
    case OPOSTINC:
    case OPOSTDEC:
        if(tcom(l))
            goto bad;
        if(tlvalue(l))
            goto bad;
        /*s: [[tcomo()]] break if isfunct */
        if(isfunct(n))
            break;
        /*e: [[tcomo()]] break if isfunct */
        if(tcompat(n, l->type, types[TINT], tadd))
            goto bad;
        n->type = l->type;

        if(n->type->etype == TIND)
        if(n->type->link->width < 1) {
            snap(n->type->link);
            if(n->type->link->width < 1)
                diag(n, "inc/dec of a void pointer");
        }
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OSIZE:
        if(l != Z) {
            if(l->op != OSTRING && l->op != OLSTRING)
                if(tcomo(l, 0))
                    goto bad;
            /*s: [[tcomo()]] when OSIZE, check if sizeof bitfield */
            if(l->op == OBIT) {
                diag(n, "sizeof bitfield");
                goto bad;
            }
            /*e: [[tcomo()]] when OSIZE, check if sizeof bitfield */
            n->type = l->type;
        }
        /*s: [[tcomo()]] when OSIZE, sanity check n */
        if(n->type == T)
            goto bad;
        if(n->type->width <= 0) {
            diag(n, "sizeof undefined type");
            goto bad;
        }
        if(n->type->etype == TFUNC) {
            diag(n, "sizeof function");
            goto bad;
        }
        /*e: [[tcomo()]] when OSIZE, sanity check n */
        n->op = OCONST;
        n->left = Z;
        n->right = Z;
        n->vconst = convvtox(n->type->width, TINT);
        n->type = types[TINT];
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case ORETURN:
        if(l == Z) {
            if(n->type->etype != TVOID)
                warn(n, "null return of a typed function");
            break;
        }
        if(tcom(l))
            goto bad;
        /*s: [[tcomo()]] when ORETURN, typing extensions */
        typeext(n->type, l);
        /*e: [[tcomo()]] when ORETURN, typing extensions */
        if(tcompat(n, n->type, l->type, tasign))
            break;
        /*s: [[tcomo()]] when ORETURN, const checking */
        constas(n, n->type, l->type);
        /*e: [[tcomo()]] when ORETURN, const checking */
        if(!sametype(n->type, l->type)) {
            l = new1(OCAST, l, Z);
            l->type = n->type;
            n->left = l;
        }
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OLSTRING:
        if(n->type->link != types[TRUNE]) {
            o = outstring(0, 0);
            while(o & 3) {
                outlstring(&zer, sizeof(TRune));
                o = outlstring(0, 0);
            }
        }
        n->op = ONAME;
        n->xoffset = outlstring(n->rstring, n->type->width);
        n->addable = 1;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OSTRUCT:
        if(tcomx(n))
            goto bad;
        break;
    /*x: [[tcomo()]] switch node kind cases */
    case OSIGN:	/* extension signof(type) returns a hash */
        if(l != Z) {
            if(l->op != OSTRING && l->op != OLSTRING)
                if(tcomo(l, 0))
                    goto bad;
            /*s: [[tcomo()]] when OSIGN, sanity check no bitfield */
            if(l->op == OBIT) {
                diag(n, "signof bitfield");
                goto bad;
            }
            /*e: [[tcomo()]] when OSIGN, sanity check no bitfield */
            n->type = l->type;
        }
        /*s: [[tcomo()]] when OSIGN, typecheck and sanity check */
        if(n->type == T)
            goto bad;
        if(n->type->width < 0) {
            diag(n, "signof undefined type");
            goto bad;
        }
        /*e: [[tcomo()]] when OSIGN, typecheck and sanity check */
        n->op = OCONST;
        n->left = Z;
        n->right = Z;
        n->vconst = convvtox(signature(n->type), TULONG);
        n->type = types[TULONG];
        break;
    /*e: [[tcomo()]] switch node kind cases */
    default:
        diag(n, "unknown op in type complex: %O", n->op);
        goto bad;

    }

    t = n->type;
    /*s: [[tcomo()]] sanity check t after typing */
    if(t == T)
        goto bad;
    /*e: [[tcomo()]] sanity check t after typing */
    /*s: [[tcomo()]] sanity check type width */
    if(t->width < 0) {
        snap(t);
        if(t->width < 0) {
            if(typesu[t->etype] && t->tag)
                diag(n, "structure not fully declared %s", t->tag->name);
            else
                diag(n, "structure not fully declared");
            goto bad;
        }
    }
    /*e: [[tcomo()]] sanity check type width */
    /*s: [[tcomo()]] special adjustments for arrays and functions */
    if(typeaf[t->etype]) {
        if(f & ADDROF)
            goto addaddr;
        if(f & ADDROP)
            warn(n, "address of array/func ignored");
    }
    /*e: [[tcomo()]] special adjustments for arrays and functions */
    return false; // everything is fine

/*s: [[tcomo()]] extra labels */
addaddr:
    if(tlvalue(n))
        goto bad;
    l = new1(OXXX, Z, Z);
    *l = *n;
    n->op = OADDR;
    if(l->type->etype == TARRAY)
        l->type = l->type->link;
    n->left = l;
    n->right = Z;
    n->addable = false;
    n->type = typ(TIND, l->type);
    n->type->width = types[TIND]->width;
    return false;
/*e: [[tcomo()]] extra labels */

bad:
    n->type = T;
    return true;
}
/*e: function tcomo */

/*s: function tcoma */
bool
tcoma(Node *l, Node *n, Type *t, bool f)
{
    Node *n1;
    int o;

    /*s: [[tcoma()]] start of tcoma, adjust t if TDOT */
    if(t != T)
    if(t->etype == TOLD || t->etype == TDOT)	/* .../old in prototype */
        t = T;
    /*e: [[tcoma()]] start of tcoma, adjust t if TDOT */
    /*s: [[tcoma()]] if no arguments */
    if(n == Z) {
        if(t != T && !sametype(t, types[TVOID])) {
            diag(n, "not enough function arguments: %F", l);
            return true;
        }
        return false;
    }
    /*e: [[tcoma()]] if no arguments */

    // recurse over arguments and parameters
    if(n->op == OLIST) {
        o = tcoma(l, n->left, t, false);
        if(t != T) {
            t = t->down;
            if(t == T)
                t = types[TVOID];
        }
        return o | tcoma(l, n->right, t, true);
    }

    // process each argument

    if(f && t != T)
        tcoma(l, Z, t->down, false);

    if(tcom(n) || tcompat(n, T, n->type, targ))
        return true;
    /*s: [[tcoma()]] check if too many arguments */
    if(sametype(t, types[TVOID])) {
        diag(n, "too many function arguments: %F", l);
        return true;
    }
    /*e: [[tcoma()]] check if too many arguments */
    if(t != T) {
        /*s: [[tcoma()]] typing extension on argument */
        typeext(t, n);
        /*e: [[tcoma()]] typing extension on argument */
        if(stcompat(nodproto, t, n->type, tasign)) {
            diag(l, "argument prototype mismatch \"%T\" for \"%T\": %F",
                n->type, t, l);
            return true;
        }
        /*s: [[tcoma()]] adjust type if argument with small type */
        switch(t->etype) {
        case TCHAR:
        case TSHORT:
            t = types[TINT];
            break;

        case TUCHAR:
        case TUSHORT:
            t = types[TUINT];
            break;
        }
        /*e: [[tcoma()]] adjust type if argument with small type */
    }
    else
        /*s: [[tcoma()]] adjust type when empty type */
        switch(n->type->etype)
        {
        case TCHAR:
        case TSHORT:
            t = types[TINT];
            break;

        case TUCHAR:
        case TUSHORT:
            t = types[TUINT];
            break;

        case TFLOAT:
            t = types[TDOUBLE];
        }
        /*e: [[tcoma()]] adjust type when empty type */

    if(t != T && !sametype(t, n->type)) {
        n1 = new1(OXXX, Z, Z);
        *n1 = *n;
        n->op = OCAST;
        n->left = n1;
        n->right = Z;
        n->type = t;
        n->addable = false;
    }
    return false; // everything is fine
}
/*e: function tcoma */

/*s: function tcomd */
bool
tcomd(Node *n)
{
    Type *t;
    long o;

    o = 0;
    t = dotsearch(n->sym, n->left->type->link, n, &o);
    if(t == T) {
        diag(n, "not a member of struct/union: %F", n);
        return true;
    }
    // convert field and set n->type
    /*s: [[tcomd()]] convert field access in offset */
    makedot(n, t, o);
    /*e: [[tcomd()]] convert field access in offset */
    return false;
}
/*e: function tcomd */

/*s: function tcomx */
bool
tcomx(Node *n)
{
    Type *t;
    Node *l, *r, **ar, **al;
    bool e;

    e = false;
    if(n->type->etype != TSTRUCT) {
        diag(n, "constructor must be a structure");
        return true;
    }
    l = invert(n->left);
    n->left = l;
    al = &n->left;
    for(t = n->type->link; t != T; t = t->down) {
        if(l == Z) {
            diag(n, "constructor list too short");
            return true;
        }
        if(l->op == OLIST) {
            r = l->left;
            ar = &l->left;
            al = &l->right;
            l = l->right;
        } else {
            r = l;
            ar = al;
            l = Z;
        }
        if(tcom(r))
            e++;
        typeext(t, r);
        if(tcompat(n, t, r->type, tasign))
            e++;
        constas(n, t, r->type);
        if(!e && !sametype(t, r->type)) {
            r = new1(OCAST, r, Z);
            r->type = t;
            *ar = r;
        }
    }
    if(l != Z) {
        diag(n, "constructor list too long");
        return true;
    }
    return e;
}
/*e: function tcomx */

/*s: function tlvalue */
bool
tlvalue(Node *n)
{

    if(!n->addable) {
        diag(n, "not an l-value");
        return true;
    }
    return false;
}
/*e: function tlvalue */

/*s: function comargs */
/*
 * hoist comma operators out of expressions
 *	(a,b) OP c => (a, b OP c)
 *	OP(a,b) =>	(a, OP b)
 *	a OP (b,c) => (b, a OP c)
 */

static Node*
comargs(Com *com, Node *n)
{
    if(n != Z && n->op == OLIST){
        n->left = comargs(com, n->left);
        n->right = comargs(com, n->right);
    }
    return commas(com, n);
}
/*e: function comargs */

/*s: function commas */
static Node*
commas(Com *com, Node *n)
{
    Node *t;

    if(n == Z)
        return n;
    switch(n->op){
    case OREGISTER:
    case OINDREG:
    case OCONST:
    case ONAME:
    case OSTRING:
        /* leaf */
        return n;

    case OCOMMA:
        t = commas(com, n->left);
        if(com->n >= nelem(com->t))
            fatal(n, "comma list overflow");
        com->t[com->n++] = t;
        return commas(com, n->right);

    case OFUNC:
        n->left = commas(com, n->left);
        n->right = comargs(com, n->right);
        return n;

    case OCOND:
        n->left = commas(com, n->left);
        comma(n->right->left);
        comma(n->right->right);
        return n;

    case OANDAND:
    case OOROR:
        n->left = commas(com, n->left);
        comma(n->right);
        return n;

    case ORETURN:
        comma(n->left);
        return n;
    }
    n->left = commas(com, n->left);
    if(n->right != Z)
        n->right = commas(com, n->right);
    return n;
}
/*e: function commas */

/*s: function comma */
static void
comma(Node *n)
{
    Com com;
    Node *nn;

    com.n = 0;
    nn = commas(&com, n);

    if(com.n > 0){

if(debug['y'])print("n=%d\n", com.n);
if(debug['y']) prtree(nn, "res");

        if(nn != n)
            *n = *nn;
        while(com.n > 0){

if(debug['y']) prtree(com.t[com.n-1], "tree");

            nn = new1(OXXX, Z, Z);
            *nn = *n;
            n->op = OCOMMA;
            n->type = nn->type;
            n->left = com.t[--com.n];
            n->right = nn;
            n->lineno = n->left->lineno;
        }
if(debug['y']) prtree(n, "final");

    }else if(n != nn)
        fatal(n, "odd tree");
}
/*e: function comma */

/*s: function ccom */
/*
 *	general rewrite
 *	(IND(ADDR x)) ==> x
 *	(ADDR(IND x)) ==> x
 *	remove some zero operands
 *	remove no op casts
 *	evaluate constants // important one!
 */
void
ccom(Node *n)
{
    Node *l, *r;
    int t;

loop:
    if(n == Z)
        return;

    l = n->left;
    r = n->right;
    switch(n->op) {
    /*s: [[ccom()]] switch node kind cases */
    case OAS:
    case OASADD:
    case OASSUB:
    case OASMUL:
    case OASLMUL:
    case OASDIV:
    case OASLDIV:
    case OASMOD:
    case OASLMOD:
    case OASASHR:
    case OASASHL:
    case OASLSHR:
    case OASAND:
    case OASOR:
    case OASXOR:
        ccom(l);
        ccom(r);
        if(n->op == OASLSHR || n->op == OASASHR || n->op == OASASHL)
          if(r->op == OCONST) {
            t = n->type->width * 8;	/* bits per byte */
            if(r->vconst >= t || r->vconst < 0)
                warn(n, "stupid shift: %lld", r->vconst);
        }
        break;
    /*x: [[ccom()]] switch node kind cases */
    case OCONST:
    case ONAME:
        break;
    /*x: [[ccom()]] switch node kind cases */
    case OREGISTER:
    case OINDREG:
        break;
    /*x: [[ccom()]] switch node kind cases */
    case OCAST:
        ccom(l);
        if(l->op == OCONST) {
            evconst(n);
            if(n->op == OCONST)
                break;
        }
        if(nocast(l->type, n->type) &&
           (!typefd[l->type->etype] || 
           typeu[l->type->etype] && typeu[n->type->etype]) ) {
            l->type = n->type;
            *n = *l;
        }
        break;
    /*x: [[ccom()]] switch node kind cases */
    case OADDR:
        ccom(l);
        l->etype = TVOID; // ????
        if(l->op == OIND) {
            l->left->type = n->type;
            *n = *l->left;
            break;
        }
        goto common;
    /*x: [[ccom()]] switch node kind cases */
    case OIND:
        ccom(l);
        if(l->op == OADDR) {
            l->left->type = n->type;
            *n = *l->left;
            break;
        }
        goto common;
    /*x: [[ccom()]] switch node kind cases */
    case OADD:
    case OOR:
    case OXOR:
        ccom(l);
        if(vconst(l) == 0) {
            *n = *r;
            goto loop;
        }
        ccom(r);
        if(vconst(r) == 0) {
            *n = *l;
            break;
        }
        goto commute;
    /*x: [[ccom()]] switch node kind cases */
    commute:
        /* look for commutative constant */
        if(r->op == OCONST) {
            if(l->op == n->op) {
                if(l->left->op == OCONST) {
                    n->right = l->right;
                    l->right = r;
                    goto loop;
                }
                if(l->right->op == OCONST) {
                    n->right = l->left;
                    l->left = r;
                    goto loop;
                }
            }
        }
        if(l->op == OCONST) {
            if(r->op == n->op) {
                if(r->left->op == OCONST) {
                    n->left = r->right;
                    r->right = l;
                    goto loop;
                }
                if(r->right->op == OCONST) {
                    n->left = r->left;
                    r->left = l;
                    goto loop;
                }
            }
        }
        goto common;
    /*x: [[ccom()]] switch node kind cases */
    case OAND:
        ccom(l);
        ccom(r);
        if(vconst(l) == 0 && !side(r)) {
            *n = *l;
            break;
        }
        if(vconst(r) == 0 && !side(l)) {
            *n = *r;
            break;
        }
        // Fallthrough, goto commute
    /*x: [[ccom()]] switch node kind cases */
    case OASHR:
    case OASHL:
    case OLSHR:
        ccom(l);
        if(vconst(l) == 0 && !side(r)) {
            *n = *l;
            break;
        }
        ccom(r);
        if(vconst(r) == 0) {
            *n = *l;
            break;
        }
        /*s: [[ccom()]] when OASHR/OASHL/OLSHR, check for stupid shift */
        if(r->op == OCONST) {
            t = n->type->width * 8;	/* bits per byte */
            if(r->vconst >= t || r->vconst <= -t)
                warn(n, "stupid shift: %lld", r->vconst);
        }
        /*e: [[ccom()]] when OASHR/OASHL/OLSHR, check for stupid shift */
        goto common;
    /*x: [[ccom()]] switch node kind cases */
    case OMUL:
    case OLMUL:
        ccom(l);
        t = vconst(l);
        if(t == 0 && !side(r)) {
            *n = *l;
            break;
        }
        if(t == 1) {
            *n = *r;
            goto loop;
        }
        ccom(r);
        t = vconst(r);
        if(t == 0 && !side(l)) {
            *n = *r;
            break;
        }
        if(t == 1) {
            *n = *l;
            break;
        }
        goto common;
    /*x: [[ccom()]] switch node kind cases */
    case ODIV:
    case OLDIV:
        ccom(l);
        if(vconst(l) == 0 && !side(r)) {
            *n = *l;
            break;
        }
        ccom(r);
        t = vconst(r);
        if(t == 0) {
            diag(n, "divide check");
            *n = *r;
            break;
        }
        if(t == 1) {
            *n = *l;
            break;
        }
        goto common;
    /*x: [[ccom()]] switch node kind cases */
    case OSUB:
        ccom(r);
        if(r->op == OCONST) {
            if(typefd[r->type->etype]) {
                n->op = OADD;
                r->fconst = -r->fconst;
                goto loop;
            } else {
                n->op = OADD;
                r->vconst = -r->vconst;
                goto loop;
            }
        }
        ccom(l);
        goto common;
    /*x: [[ccom()]] switch node kind cases */
    case OEQ:
    case ONE:

    case OLE:
    case OGE:
    case OLT:
    case OGT:

    case OLS:
    case OHS:
    case OLO:
    case OHI:
        ccom(l);
        ccom(r);
        if(compar(n, 0) || compar(n, 1))
            break;
        relcon(l, r);
        relcon(r, l);
        goto common;
    /*x: [[ccom()]] switch node kind cases */
    case OANDAND:
        ccom(l);
        if(vconst(l) == 0) {
            *n = *l;
            break;
        }
        ccom(r);
        goto common;
    /*x: [[ccom()]] switch node kind cases */
    case OOROR:
        ccom(l);
        if(l->op == OCONST && l->vconst != 0) {
            *n = *l;
            n->vconst = 1;
            break;
        }
        ccom(r);
        goto common;
    /*x: [[ccom()]] switch node kind cases */
    case OCOND:
        ccom(l);
        ccom(r);
        if(l->op == OCONST)
            if(vconst(l) == 0)
                *n = *r->right;
            else
                *n = *r->left;
        break;
    /*e: [[ccom()]] switch node kind cases */
    default:
        // recurse
        if(l != Z)
            ccom(l);
        if(r != Z)
            ccom(r);
        // Fallthrough
    common:
        if(((l == Z)||(l->op == OCONST)) && ((r == Z)||(r->op == OCONST)))
            evconst(n);
    }
}
/*e: function ccom */

/*s: global cmps */
/*	OEQ, ONE, OLE, OLS, OLT, OLO, OGE, OHS, OGT, OHI */
static char *cmps[12] = 
{
    "==", "!=", "<=", "<=", "<", "<", ">=", ">=", ">", ">",
};
/*e: global cmps */

/*s: struct Big */
/* 128-bit numbers */
struct Big
{
    vlong a;
    uvlong b;
};
/*e: struct Big */
/*s: function cmp */
static int
cmp(Big x, Big y)
{
    if(x.a != y.a){
        if(x.a < y.a)
            return -1;
        return 1;
    }
    if(x.b != y.b){
        if(x.b < y.b)
            return -1;
        return 1;
    }
    return 0;
}
/*e: function cmp */
/*s: function add */
static Big
add(Big x, int y)
{
    uvlong ob;
    
    ob = x.b;
    x.b += y;
    if(y > 0 && x.b < ob)
        x.a++;
    if(y < 0 && x.b > ob)
        x.a--;
    return x;
} 
/*e: function add */

/*s: function big */
Big
big(vlong a, uvlong b)
{
    Big x;

    x.a = a;
    x.b = b;
    return x;
}
/*e: function big */

/*s: function compar */
int
compar(Node *n, int reverse)
{
    Big lo, hi, x;
    int op;
    char xbuf[40], cmpbuf[50];
    Node *l, *r;
    Type *lt, *rt;

    /*
     * The point of this function is to diagnose comparisons 
     * that can never be true or that look misleading because
     * of the `usual arithmetic conversions'.  As an example 
     * of the latter, if x is a ulong, then if(x <= -1) really means
     * if(x <= 0xFFFFFFFF), while if(x <= -1LL) really means
     * what it says (but 8c compiles it wrong anyway).
     */

    if(reverse){
        r = n->left;
        l = n->right;
        op = comrel[relindex(n->op)];
    }else{
        l = n->left;
        r = n->right;
        op = n->op;
    }

    /*
     * Skip over left casts to find out the original expression range.
     */
    while(l->op == OCAST)
        l = l->left;
    if(l->op == OCONST)
        return 0;

    lt = l->type;
    if(l->op == ONAME && l->sym->type){
        lt = l->sym->type;
        if(lt->etype == TARRAY)
            lt = lt->link;
    }
    if(lt == T)
        return 0;
    if(lt->etype == TXXX || lt->etype > TUVLONG)
        return 0;
    
    /*
     * Skip over the right casts to find the on-screen value.
     */
    if(r->op != OCONST)
        return 0;
    while(r->oldop == OCAST && !r->xcast)
        r = r->left;
    rt = r->type;
    if(rt == T)
        return 0;

    x.b = r->vconst;
    x.a = 0;
    if((rt->etype&1) && r->vconst < 0)	/* signed negative */
        x.a = ~0ULL;

    if((lt->etype&1)==0){
        /* unsigned */
        lo = big(0, 0);
        if(lt->width == 8)
            hi = big(0, ~0ULL);
        else
            hi = big(0, (1LL<<(l->type->width*8))-1);
    }else{
        lo = big(~0ULL, -(1LL<<(l->type->width*8-1)));
        hi = big(0, (1LL<<(l->type->width*8-1))-1);
    }

    switch(op){
    case OLT:
    case OLO:
    case OGE:
    case OHS:
        if(cmp(x, lo) <= 0)
            goto useless;
        if(cmp(x, add(hi, 1)) >= 0)
            goto useless;
        break;
    case OLE:
    case OLS:
    case OGT:
    case OHI:
        if(cmp(x, add(lo, -1)) <= 0)
            goto useless;
        if(cmp(x, hi) >= 0)
            goto useless;
        break;
    case OEQ:
    case ONE:
        /*
         * Don't warn about comparisons if the expression
         * is as wide as the value: the compiler-supplied casts
         * will make both outcomes possible.
         */
        if(lt->width >= rt->width && debug['w'] < 2)
            return 0;
        if(cmp(x, lo) < 0 || cmp(x, hi) > 0)
            goto useless;
        break;
    }
    return 0;

useless:
    if((x.a==0 && x.b<=9) || (x.a==~0LL && x.b >= -9ULL))
        snprint(xbuf, sizeof xbuf, "%lld", x.b);
    else if(x.a == 0)
        snprint(xbuf, sizeof xbuf, "%#llux", x.b);
    else
        snprint(xbuf, sizeof xbuf, "%#llx", x.b);
    if(reverse)
        snprint(cmpbuf, sizeof cmpbuf, "%s %s %T",
            xbuf, cmps[relindex(n->op)], lt);
    else
        snprint(cmpbuf, sizeof cmpbuf, "%T %s %s",
            lt, cmps[relindex(n->op)], xbuf);
    if(debug['y']) 
        prtree(n, "strange");
    warn(n, "useless or misleading comparison: %s", cmpbuf);
    return 0;
}
/*e: function compar */

/*e: cc/com.c */
