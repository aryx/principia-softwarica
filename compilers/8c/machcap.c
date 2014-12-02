/*s: 8c/machcap.c */
#include "gc.h"

/*s: function machcap(x86) */
bool
machcap(Node *n)
{
//	return false;

    if(n == Z)
        return true;	/* test */

    switch(n->op) {
    case OMUL:
    case OLMUL:
    case OASMUL:
    case OASLMUL:
        if(typechl[n->type->etype])
            return true;
        if(typev[n->type->etype]) {
//		if(typev[n->type->etype] && n->right->op == OCONST) {
//			if(hi64v(n->right) == 0)
                return true;
        }
        break;

    case OCOM:
    case ONEG:
    case OADD:
    case OAND:
    case OOR:
    case OSUB:
    case OXOR:
    case OASHL:
    case OLSHR:
    case OASHR:
        if(typechlv[n->left->type->etype])
            return true;
        break;

    case OCAST:
        if(typev[n->type->etype]) {
            if(typechlp[n->left->type->etype])
                return true;
        }
        else if(!typefd[n->type->etype]) {
            if(typev[n->left->type->etype])
                return true;
        }
        break;

    case OCOND:
    case OCOMMA:
    case OLIST:
    case OANDAND:
    case OOROR:
    case ONOT:
    case ODOT:
        return true;

    case OASADD:
    case OASSUB:
    case OASAND:
    case OASOR:
    case OASXOR:
        return true;

    case OASASHL:
    case OASASHR:
    case OASLSHR:
        return true;

    case OPOSTINC:
    case OPOSTDEC:
    case OPREINC:
    case OPREDEC:
        return true;

    case OEQ:
    case ONE:
    case OLE:
    case OGT:
    case OLT:
    case OGE:
    case OHI:
    case OHS:
    case OLO:
    case OLS:
//print("%O\n", n->op);
        return true;
    }
    return false;
}
/*e: function machcap(x86) */
/*e: 8c/machcap.c */
