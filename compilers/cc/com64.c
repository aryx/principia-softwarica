/*s: cc/com64.c */
#include "cc.h"

/*s: constant FNX */
/*
 * this is machine dependent, but it is totally
 * common on all of the 64-bit symulating machines.
 */

#define	FNX	100	/* botch -- redefinition */
/*e: constant FNX */

/*s: global nodaddv */
Node*	nodaddv;
/*e: global nodaddv */
/*s: global nodsubv */
Node*	nodsubv;
/*e: global nodsubv */
/*s: global nodmulv */
Node*	nodmulv;
/*e: global nodmulv */
/*s: global noddivv */
Node*	noddivv;
/*e: global noddivv */
/*s: global noddivvu */
Node*	noddivvu;
/*e: global noddivvu */
/*s: global nodmodv */
Node*	nodmodv;
/*e: global nodmodv */
/*s: global nodmodvu */
Node*	nodmodvu;
/*e: global nodmodvu */
/*s: global nodlshv */
Node*	nodlshv;
/*e: global nodlshv */
/*s: global nodrshav */
Node*	nodrshav;
/*e: global nodrshav */
/*s: global nodrshlv */
Node*	nodrshlv;
/*e: global nodrshlv */
/*s: global nodandv */
Node*	nodandv;
/*e: global nodandv */
/*s: global nodorv */
Node*	nodorv;
/*e: global nodorv */
/*s: global nodxorv */
Node*	nodxorv;
/*e: global nodxorv */
/*s: global nodnegv */
Node*	nodnegv;
/*e: global nodnegv */
/*s: global nodcomv */
Node*	nodcomv;
/*e: global nodcomv */

/*s: global nodtestv */
Node*	nodtestv;
/*e: global nodtestv */
/*s: global nodeqv */
Node*	nodeqv;
/*e: global nodeqv */
/*s: global nodnev */
Node*	nodnev;
/*e: global nodnev */
/*s: global nodlev */
Node*	nodlev;
/*e: global nodlev */
/*s: global nodltv */
Node*	nodltv;
/*e: global nodltv */
/*s: global nodgev */
Node*	nodgev;
/*e: global nodgev */
/*s: global nodgtv */
Node*	nodgtv;
/*e: global nodgtv */
/*s: global nodhiv */
Node*	nodhiv;
/*e: global nodhiv */
/*s: global nodhsv */
Node*	nodhsv;
/*e: global nodhsv */
/*s: global nodlov */
Node*	nodlov;
/*e: global nodlov */
/*s: global nodlsv */
Node*	nodlsv;
/*e: global nodlsv */

/*s: global nodf2v */
Node*	nodf2v;
/*e: global nodf2v */
/*s: global nodd2v */
Node*	nodd2v;
/*e: global nodd2v */
/*s: global nodp2v */
Node*	nodp2v;
/*e: global nodp2v */
/*s: global nodsi2v */
Node*	nodsi2v;
/*e: global nodsi2v */
/*s: global nodui2v */
Node*	nodui2v;
/*e: global nodui2v */
/*s: global nodsl2v */
Node*	nodsl2v;
/*e: global nodsl2v */
/*s: global nodul2v */
Node*	nodul2v;
/*e: global nodul2v */
/*s: global nodsh2v */
Node*	nodsh2v;
/*e: global nodsh2v */
/*s: global noduh2v */
Node*	noduh2v;
/*e: global noduh2v */
/*s: global nodsc2v */
Node*	nodsc2v;
/*e: global nodsc2v */
/*s: global noduc2v */
Node*	noduc2v;
/*e: global noduc2v */

/*s: global nodv2f */
Node*	nodv2f;
/*e: global nodv2f */
/*s: global nodv2d */
Node*	nodv2d;
/*e: global nodv2d */
/*s: global nodv2ui */
Node*	nodv2ui;
/*e: global nodv2ui */
/*s: global nodv2si */
Node*	nodv2si;
/*e: global nodv2si */
/*s: global nodv2ul */
Node*	nodv2ul;
/*e: global nodv2ul */
/*s: global nodv2sl */
Node*	nodv2sl;
/*e: global nodv2sl */
/*s: global nodv2uh */
Node*	nodv2uh;
/*e: global nodv2uh */
/*s: global nodv2sh */
Node*	nodv2sh;
/*e: global nodv2sh */
/*s: global nodv2uc */
Node*	nodv2uc;
/*e: global nodv2uc */
/*s: global nodv2sc */
Node*	nodv2sc;
/*e: global nodv2sc */

/*s: global nodvpp */
Node*	nodvpp;
/*e: global nodvpp */
/*s: global nodppv */
Node*	nodppv;
/*e: global nodppv */
/*s: global nodvmm */
Node*	nodvmm;
/*e: global nodvmm */
/*s: global nodmmv */
Node*	nodmmv;
/*e: global nodmmv */

/*s: global nodvasop */
Node*	nodvasop;
/*e: global nodvasop */

/*s: global etconv */
char	etconv[NTYPE];	/* for _vasop */
/*e: global etconv */
/*s: global initetconv */
Init	initetconv[] =
{
    TCHAR,		1,	0,
    TUCHAR,		2,	0,
    TSHORT,		3,	0,
    TUSHORT,	4,	0,
    TLONG,		5,	0,
    TULONG,		6,	0,
    TVLONG,		7,	0,
    TUVLONG,	8,	0,
    TINT,		9,	0,
    TUINT,		10,	0,
    -1,		0,	0,
};
/*e: global initetconv */

/*s: function fvn */
Node*
fvn(char *name, int type)
{
    Node *n;

    n = new(ONAME, Z, Z);
    n->sym = slookup(name);
    n->sym->sig = SIGINTERN;
    if(fntypes[type] == nil)
        fntypes[type] = typ(TFUNC, types[type]);
    n->type = fntypes[type];
    n->etype = type; // type of return?
    n->class = CGLOBL;
    n->addable = 10;
    n->complex = 0;
    return n;
}
/*e: function fvn */

/*s: function com64init */
void
com64init(void)
{
    Init *p;

    nodaddv = fvn("_addv", TVLONG);
    nodsubv = fvn("_subv", TVLONG);
    nodmulv = fvn("_mulv", TVLONG);
    noddivv = fvn("_divv", TVLONG);
    noddivvu = fvn("_divvu", TVLONG);
    nodmodv = fvn("_modv", TVLONG);
    nodmodvu = fvn("_modvu", TVLONG);
    nodlshv = fvn("_lshv", TVLONG);
    nodrshav = fvn("_rshav", TVLONG);
    nodrshlv = fvn("_rshlv", TVLONG);
    nodandv = fvn("_andv", TVLONG);
    nodorv = fvn("_orv", TVLONG);
    nodxorv = fvn("_xorv", TVLONG);
    nodnegv = fvn("_negv", TVLONG);
    nodcomv = fvn("_comv", TVLONG);

    nodtestv = fvn("_testv", TLONG);
    nodeqv = fvn("_eqv", TLONG);
    nodnev = fvn("_nev", TLONG);
    nodlev = fvn("_lev", TLONG);
    nodltv = fvn("_ltv", TLONG);
    nodgev = fvn("_gev", TLONG);
    nodgtv = fvn("_gtv", TLONG);
    nodhiv = fvn("_hiv", TLONG);
    nodhsv = fvn("_hsv", TLONG);
    nodlov = fvn("_lov", TLONG);
    nodlsv = fvn("_lsv", TLONG);

    nodf2v = fvn("_f2v", TVLONG);
    nodd2v = fvn("_d2v", TVLONG);
    nodp2v = fvn("_p2v", TVLONG);
    nodsi2v = fvn("_si2v", TVLONG);
    nodui2v = fvn("_ui2v", TVLONG);
    nodsl2v = fvn("_sl2v", TVLONG);
    nodul2v = fvn("_ul2v", TVLONG);
    nodsh2v = fvn("_sh2v", TVLONG);
    noduh2v = fvn("_uh2v", TVLONG);
    nodsc2v = fvn("_sc2v", TVLONG);
    noduc2v = fvn("_uc2v", TVLONG);

    nodv2f = fvn("_v2f", TFLOAT);
    nodv2d = fvn("_v2d", TDOUBLE);
    nodv2sl = fvn("_v2sl", TLONG);
    nodv2ul = fvn("_v2ul", TULONG);
    nodv2si = fvn("_v2si", TINT);
    nodv2ui = fvn("_v2ui", TUINT);
    nodv2sh = fvn("_v2sh", TSHORT);
    nodv2uh = fvn("_v2ul", TUSHORT);
    nodv2sc = fvn("_v2sc", TCHAR);
    nodv2uc = fvn("_v2uc", TUCHAR);

    nodvpp = fvn("_vpp", TVLONG);
    nodppv = fvn("_ppv", TVLONG);
    nodvmm = fvn("_vmm", TVLONG);
    nodmmv = fvn("_mmv", TVLONG);

    nodvasop = fvn("_vasop", TVLONG);

    for(p = initetconv; p->code >= 0; p++)
        etconv[p->code] = p->value;
}
/*e: function com64init */

/*s: function com64 */
bool
com64(Node *n)
{
    Node *l, *r; 
    Node *a, *t;
    bool lv, rv;

    if(n->type == nil)
        return false;

    l = n->left;
    r = n->right;

    lv = false;
    if(l && l->type && typev[l->type->etype])
        lv = true;
    rv = false;
    if(r && r->type && typev[r->type->etype])
        rv = true;

    if(lv) {
        switch(n->op) {
        case OEQ:
            a = nodeqv;
            goto setbool;
        case ONE:
            a = nodnev;
            goto setbool;
        case OLE:
            a = nodlev;
            goto setbool;
        case OLT:
            a = nodltv;
            goto setbool;
        case OGE:
            a = nodgev;
            goto setbool;
        case OGT:
            a = nodgtv;
            goto setbool;
        case OHI:
            a = nodhiv;
            goto setbool;
        case OHS:
            a = nodhsv;
            goto setbool;
        case OLO:
            a = nodlov;
            goto setbool;
        case OLS:
            a = nodlsv;
            goto setbool;

        case OANDAND:
        case OOROR:
            if(machcap(n))
                return true;

            if(rv) {
                r = new(OFUNC, nodtestv, r);
                n->right = r;
                r->complex = FNX;
                r->op = OFUNC;
                r->type = types[TLONG];
            }

        case OCOND:
        case ONOT:
            if(machcap(n))
                return true;

            l = new(OFUNC, nodtestv, l);
            n->left = l;
            l->complex = FNX;
            l->op = OFUNC;
            l->type = types[TLONG];
            n->complex = FNX;
            return 1;
        }
    }

    if(rv) {
        if(machcap(n))
            return true;
        switch(n->op) {
        case OANDAND:
        case OOROR:
            r = new(OFUNC, nodtestv, r);
            n->right = r;
            r->complex = FNX;
            r->op = OFUNC;
            r->type = types[TLONG];
            return 1;
        case OCOND:
            return 1;
        }
    }

    if(typev[n->type->etype]) {
        if(machcap(n))
            return true;
        switch(n->op) {
        default:
            diag(n, "unknown vlong %O", n->op);
        case OFUNC:
            n->complex = FNX;
        case ORETURN:
        case OAS:
        case OIND:
        case OLIST:
        case OCOMMA:
            return 1;
        case OADD:
            a = nodaddv;
            goto setbop;
        case OSUB:
            a = nodsubv;
            goto setbop;
        case OMUL:
        case OLMUL:
            a = nodmulv;
            goto setbop;
        case ODIV:
            a = noddivv;
            goto setbop;
        case OLDIV:
            a = noddivvu;
            goto setbop;
        case OMOD:
            a = nodmodv;
            goto setbop;
        case OLMOD:
            a = nodmodvu;
            goto setbop;
        case OASHL:
            a = nodlshv;
            goto setbop;
        case OASHR:
            a = nodrshav;
            goto setbop;
        case OLSHR:
            a = nodrshlv;
            goto setbop;
        case OAND:
            a = nodandv;
            goto setbop;
        case OOR:
            a = nodorv;
            goto setbop;
        case OXOR:
            a = nodxorv;
            goto setbop;
        case OPOSTINC:
            a = nodvpp;
            goto setvinc;
        case OPOSTDEC:
            a = nodvmm;
            goto setvinc;
        case OPREINC:
            a = nodppv;
            goto setvinc;
        case OPREDEC:
            a = nodmmv;
            goto setvinc;
        case ONEG:
            a = nodnegv;
            goto setfnx;
        case OCOM:
            a = nodcomv;
            goto setfnx;
        case OCAST:
            switch(l->type->etype) {
            case TCHAR:
                a = nodsc2v;
                goto setfnxl;
            case TUCHAR:
                a = noduc2v;
                goto setfnxl;
            case TSHORT:
                a = nodsh2v;
                goto setfnxl;
            case TUSHORT:
                a = noduh2v;
                goto setfnxl;
            case TINT:
                a = nodsi2v;
                goto setfnx;
            case TUINT:
                a = nodui2v;
                goto setfnx;
            case TLONG:
                a = nodsl2v;
                goto setfnx;
            case TULONG:
                a = nodul2v;
                goto setfnx;
            case TFLOAT:
                a = nodf2v;
                goto setfnx;
            case TDOUBLE:
                a = nodd2v;
                goto setfnx;
            case TIND:
                a = nodp2v;
                goto setfnx;
            }
            diag(n, "unknown %T->vlong cast", l->type);
            return true;
        case OASADD:
            a = nodaddv;
            goto setasop;
        case OASSUB:
            a = nodsubv;
            goto setasop;
        case OASMUL:
        case OASLMUL:
            a = nodmulv;
            goto setasop;
        case OASDIV:
            a = noddivv;
            goto setasop;
        case OASLDIV:
            a = noddivvu;
            goto setasop;
        case OASMOD:
            a = nodmodv;
            goto setasop;
        case OASLMOD:
            a = nodmodvu;
            goto setasop;
        case OASASHL:
            a = nodlshv;
            goto setasop;
        case OASASHR:
            a = nodrshav;
            goto setasop;
        case OASLSHR:
            a = nodrshlv;
            goto setasop;
        case OASAND:
            a = nodandv;
            goto setasop;
        case OASOR:
            a = nodorv;
            goto setasop;
        case OASXOR:
            a = nodxorv;
            goto setasop;
        }
    }

    if(typefd[n->type->etype] && l && l->op == OFUNC) {
        switch(n->op) {
        case OASADD:
        case OASSUB:
        case OASMUL:
        case OASLMUL:
        case OASDIV:
        case OASLDIV:
        case OASMOD:
        case OASLMOD:
        case OASASHL:
        case OASASHR:
        case OASLSHR:
        case OASAND:
        case OASOR:
        case OASXOR:
            if(l->right && typev[l->right->etype]) {
                diag(n, "sorry float <asop> vlong not implemented\n");
            }
        }
    }

    if(n->op == OCAST) {
        if(l->type && typev[l->type->etype]) {
            if(machcap(n))
                return true;
            switch(n->type->etype) {
            case TDOUBLE:
                a = nodv2d;
                goto setfnx;
            case TFLOAT:
                a = nodv2f;
                goto setfnx;
            case TLONG:
                a = nodv2sl;
                goto setfnx;
            case TULONG:
                a = nodv2ul;
                goto setfnx;
            case TINT:
                a = nodv2si;
                goto setfnx;
            case TUINT:
                a = nodv2ui;
                goto setfnx;
            case TSHORT:
                a = nodv2sh;
                goto setfnx;
            case TUSHORT:
                a = nodv2uh;
                goto setfnx;
            case TCHAR:
                a = nodv2sc;
                goto setfnx;
            case TUCHAR:
                a = nodv2uc;
                goto setfnx;
            case TIND:	// small pun here
                a = nodv2ul;
                goto setfnx;
            }
            diag(n, "unknown vlong->%T cast", n->type);
            return true;
        }
    }

    return false;

setbop:
    n->left = a;
    n->right = new(OLIST, l, r);
    n->complex = FNX;
    n->op = OFUNC;
    return true;

setfnxl:
    l = new(OCAST, l, 0);
    l->type = types[TLONG];
    l->complex = l->left->complex;

setfnx:
    n->left = a;
    n->right = l;
    n->complex = FNX;
    n->op = OFUNC;
    return true;

setvinc:
    n->left = a;
    l = new(OADDR, l, Z);
    l->type = typ(TIND, l->left->type);
    l->complex = l->left->complex;
    n->right = new(OLIST, l, r);
    n->complex = FNX;
    n->op = OFUNC;
    return true;

setbool:
    if(machcap(n))
        return true;
    n->left = a;
    n->right = new(OLIST, l, r);
    n->complex = FNX;
    n->op = OFUNC;
    n->type = types[TLONG];
    return true;

setasop:
    if(l->op == OFUNC) {
        l = l->right;
        goto setasop;
    }

    t = new(OCONST, 0, 0);
    t->vconst = etconv[l->type->etype];
    t->type = types[TLONG];
    t->addable = 20;
    r = new(OLIST, t, r);

    t = new(OADDR, a, 0);
    t->type = typ(TIND, a->type);
    r = new(OLIST, t, r);

    t = new(OADDR, l, 0);
    t->type = typ(TIND, l->type);
    t->complex = l->complex;
    r = new(OLIST, t, r);

    n->left = nodvasop;
    n->right = r;
    n->complex = FNX;
    n->op = OFUNC;

    return true;
}
/*e: function com64 */

/*s: function bool64 */
void
bool64(Node *n)
{
    Node *n1;

    if(machcap(Z))
        return;

    if(typev[n->type->etype]) {
        n1 = new(OXXX, 0, 0);
        *n1 = *n;

        n->right = n1;
        n->left = nodtestv;
        n->complex = FNX;
        n->addable = 0;
        n->op = OFUNC;
        n->type = types[TLONG];
    }
}
/*e: function bool64 */

/*s: function convvtox */
vlong
convvtox(vlong c, int et)
{
    int n;

    n = 8 * ewidth[et];
    c &= MASK(n);
    if(!typeu[et])
        if(c & SIGN(n))
            c |= ~MASK(n);
    return c;
}
/*e: function convvtox */
/*e: cc/com64.c */
