/*s: linkers/5l/datagen.c */
#include	"l.h"

/*s: constant Dbufslop */
#define	Dbufslop	100
/*e: constant Dbufslop */

/*s: function datblk(arm) */
void
datblk(long s, long n, bool sstring)
{
    Prog *p; 
    // absolute address of a DATA
    long a; // ulong?
    // size of a DATA
    int c; 
    // index in output buffer for a DATA
    long l;
    // index in value of a DATA
    int i;
    /*s: [[datblk()]] other locals */
    long j;
    /*x: [[datblk()]] other locals */
    char *cast;
    long d;
    /*x: [[datblk()]] other locals */
    Sym *v;
    /*x: [[datblk()]] other locals */
    long fl;
    /*e: [[datblk()]] other locals */

    memset(buf.dbuf, 0, n+Dbufslop);

    for(p = datap; p != P; p = p->link) {
        /*s: [[datblk()]] if sstring might continue */
        if(sstring != (p->from.sym->type == SSTRING))
            continue;
        /*e: [[datblk()]] if sstring might continue */
        // else
        curp = p;

        a = p->from.sym->value + p->from.offset;
        l = a - s;
        c = p->reg;

        i = 0;
        if(l < 0) {
            if(l+c <= 0)
                continue;
            while(l < 0) {
                l++;
                i++;
            }
        }
        if(l >= n)
            continue;
        // else

        /*s: [[datblk()]] sanity check multiple initialization */
        for(j=l+(c-i)-1; j>=l; j--)
            if(buf.dbuf[j]) {
                print("%P\n", p);
                diag("multiple initialization");
                break;
            }
        /*e: [[datblk()]] sanity check multiple initialization */

        switch(p->to.type) {
        /*s: [[datblk()]] switch type of destination cases */
        case D_SCONST:
            for(; i<c; i++) {
                buf.dbuf[l] = p->to.sval[i];
                l++;
            }
            break;
        /*x: [[datblk()]] switch type of destination cases */
        case D_CONST: case D_ADDR:
            d = p->to.offset;
            /*s: [[datblk()]] if D_ADDR case */
            v = p->to.sym;
            if(v) {
                switch(v->type) {
                /*s: [[datblk()]] in D_ADDR case, switch symbol type cases */
                case STEXT: case SSTRING:
                    d += p->to.sym->value;
                    break;
                case SDATA: case SBSS:
                    d += p->to.sym->value + INITDAT;
                    break;
                /*x: [[datblk()]] in D_ADDR case, switch symbol type cases */
                case SUNDEF:
                    ckoff(v, d);
                    d += p->to.sym->value;
                    break;
                /*e: [[datblk()]] in D_ADDR case, switch symbol type cases */
                }
                /*s: [[datblk()]] if dynamic module(arm) */
                if(dlm)
                    dynreloc(v, a+INITDAT, 1);
                /*e: [[datblk()]] if dynamic module(arm) */
            }
            /*e: [[datblk()]] if D_ADDR case */
            cast = (char*)&d;

            switch(c) {
            case 1:
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[inuxi1[i]];
                    l++;
                }
                break;
            case 2:
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[inuxi2[i]];
                    l++;
                }
                break;
            case 4:
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[inuxi4[i]];
                    l++;
                }
                break;

            default:
                diag("bad nuxi %d %d%P", c, i, curp);
                break;
            }
            break;
        /*x: [[datblk()]] switch type of destination cases */
        case D_FCONST:
            switch(c) {
            default:
            case 4:
                fl = ieeedtof(p->to.ieee);
                cast = (char*)&fl;
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[fnuxi4[i]];
                    l++;
                }
                break;
            case 8:
                cast = (char*)p->to.ieee;
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[fnuxi8[i]];
                    l++;
                }
                break;
            }
            break;
        /*e: [[datblk()]] switch type of destination cases */
        default:
            diag("unknown mode in initialization%P", p);
            break;
        }
    }
    write(cout, buf.dbuf, n);
}
/*e: function datblk(arm) */

/*e: linkers/5l/datagen.c */
