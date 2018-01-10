/*s: linkers/5l/profile.c */
#include	"l.h"

/*s: function [[doprof1]](arm) */
void
doprof1(void)
{
    Sym *s;
    Prog *p, *q;
    long n;

    DBG("%5.2f profile 1\n", cputime());

    s = lookup("__mcount", 0);
    n = 1;
    for(p = firstp->link; p != P; p = p->link) {
        if(p->as == ATEXT) {

            // DATA __mcount+n*4(SB)/4,  $foo(SB) //$
            q = prg();
            q->line = p->line;
            q->as = ADATA;
            q->from.type = D_OREG;
            q->from.symkind = N_EXTERN;
            q->from.offset = n*4;
            q->from.sym = s;
            q->reg = 4; // size of this DATA slice
            q->to = p->from;
            q->to.type = D_ADDR;

            // add_list(q, datap)
            q->link = datap;
            datap = q;

            // MOVW __mcount+ n*4+4(SB), R11
            q = prg();
            q->line = p->line;
            q->pc = p->pc;
            q->as = AMOVW;
            q->from.type = D_OREG;
            q->from.symkind = N_EXTERN;
            q->from.sym = s;
            q->from.offset = n*4 + 4;
            q->to.type = D_REG;
            q->to.reg = REGTMP;

            // add_after(q, p)            
            q->link = p->link;
            p->link = q;

            p = q;

            // ADD, $1, R11 //$
            q = prg();
            q->line = p->line;
            q->pc = p->pc;
            q->as = AADD;
            q->from.type = D_CONST;
            q->from.offset = 1;
            q->to.type = D_REG;
            q->to.reg = REGTMP;

            // add_after(q, p)            
            q->link = p->link;
            p->link = q;

            p = q;

            // MOVW R11, __mcount+ n*4+4(SB)
            q = prg();
            q->line = p->line;
            q->pc = p->pc;
            q->as = AMOVW;
            q->from.type = D_REG;
            q->from.reg = REGTMP;
            q->to.type = D_OREG;
            q->to.symkind = N_EXTERN;
            q->to.sym = s;
            q->to.offset = n*4 + 4;

            // add_after(q, p)            
            q->link = p->link;
            p->link = q;

            p = q;

            n += 2;
            continue;
        }
    }

    // DATA __mcount+0(SB)/4, $n 
    q = prg();
    q->line = 0;
    q->as = ADATA;
    q->from.type = D_OREG;
    q->from.symkind = N_EXTERN;
    q->from.sym = s;
    q->reg = 4;
    q->to.type = D_CONST;
    q->to.offset = n;

    // add_list(q, datap)
    q->link = datap;
    datap = q;

    s->type = SBSS;
    s->value = n*4;
}
/*e: function [[doprof1]](arm) */

/*s: global [[brcond]](arm) */
static int brcond[] = 
 {ABEQ, ABNE, 
  ABHS, ABLO, 
  ABMI, ABPL, 
  ABVS, ABVC, 
  ABHI, ABLS, 
  ABGE, ABLT, 
  ABGT, ABLE};
/*e: global [[brcond]](arm) */

/*s: function [[doprof2]](arm) */
void
doprof2(void)
{
    Sym *s2, *s4;
    Prog *p, *q;
    /*s: [[doprof2()]] other locals */
    Prog *ps2 = P;
    Prog *ps4 = P;
    /*e: [[doprof2()]] other locals */

    DBG("%5.2f profile 2\n", cputime());

    // in lib_core/libc/port/profile.c
    s2 = lookup("_profin", 0);
    s4 = lookup("_profout", 0);
    /*s: [[doprof2()]] sanity check s2 and s4 */
    if(s2->type != STEXT || s4->type != STEXT) {
        diag("_profin/_profout not defined");
        return;
    }
    /*e: [[doprof2()]] sanity check s2 and s4 */

    /*s: [[doprof2()]] find ps2, ps4, the Instr of s2 and s4 */
    for(p = firstp; p != P; p = p->link) {
        if(p->as == ATEXT) {
            if(p->from.sym == s2) {
                ps2 = p;
                /*s: [[doprof2()]] set TEXT attribute of [[_profin]] or [[_profout]] */
                p->reg = NOPROF;
                /*e: [[doprof2()]] set TEXT attribute of [[_profin]] or [[_profout]] */
            }
            if(p->from.sym == s4) {
                ps4 = p;
                /*s: [[doprof2()]] set TEXT attribute of [[_profin]] or [[_profout]] */
                p->reg = NOPROF;
                /*e: [[doprof2()]] set TEXT attribute of [[_profin]] or [[_profout]] */
            }
        }
    }
    /*e: [[doprof2()]] find ps2, ps4, the Instr of s2 and s4 */

    for(p = firstp; p != P; p = p->link) {
        if(p->as == ATEXT) {
            /*s: [[doprof2()]] if NOPROF p(arm) */
            if(p->reg & NOPROF) {
                for(;;) {
                    q = p->link;
                    if(q == P || q->as == ATEXT)
                        break;
                    p = q;
                }
                continue;
            }
            /*e: [[doprof2()]] if NOPROF p(arm) */
            /*s: [[doprof2()]] ATEXT instrumentation */
            /*
             * BL	profin
             */
            q = prg();
            q->line = p->line;
            q->pc = p->pc;
            q->as = ABL;
            q->to.type = D_BRANCH;
            q->cond = ps2; // _profin
            q->to.sym = s2;

            //insert_after(q, p)
            q->link = p->link;
            p->link = q;

            p = q;
            /*e: [[doprof2()]] ATEXT instrumentation */
            continue;
        }
        if(p->as == ARET) {
            /*s: [[doprof2()]] ARET instrumentation */
            /*
             * RET
             */
            q = prg();
            // *q = *p;
            q->as = ARET;
            q->from = p->from;
            q->to = p->to;
            q->cond = p->cond;
            q->link = p->link;
            q->reg = p->reg;

            // insert_after(q, p)
            p->link = q;

            /*s: [[doprof2()]] in ARET case, if conditinal execution */
            if(p->scond != COND_ALWAYS) {
                // BL _profout
                q = prg();
                q->as = ABL;
                q->from = zprg.from;
                q->to = zprg.to;
                q->to.type = D_BRANCH;
                q->cond = ps4; // _profout
                q->to.sym = s4;

                // insert_after(q, p)
                q->link = p->link;
                p->link = q;

                // overwrite original RET instruction with  B.XXX 
                p->as = brcond[p->scond^1];	/* complement */
                p->scond = COND_ALWAYS;
                p->from = zprg.from;
                p->to = zprg.to;
                p->to.type = D_BRANCH;
                p->cond = q->link->link;	/* successor of RET */
                p->to.offset = q->link->link->pc; // useful??
        
                p = q->link->link;
            }
            /*e: [[doprof2()]] in ARET case, if conditinal execution */
            else {
               /*
                * BL	profout
                */
               // overwrite original RET instruction
               p->as = ABL;
               p->from = zprg.from;
               p->to = zprg.to;
               p->to.type = D_BRANCH;
               p->cond = ps4; // _profout
               p->to.sym = s4;
               p->scond = COND_ALWAYS;

               p = q;
            }
            /*e: [[doprof2()]] ARET instrumentation */
            continue;
        }
    }
}
/*e: function [[doprof2]](arm) */

/*e: linkers/5l/profile.c */
