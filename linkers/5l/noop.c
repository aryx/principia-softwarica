/*s: linkers/5l/noop.c */
#include	"l.h"

/*s: global sym_div(arm) */
static	Sym*	sym_div;
/*e: global sym_div(arm) */
/*s: global sym_divu(arm) */
static	Sym*	sym_divu;
/*e: global sym_divu(arm) */
/*s: global sym_mod(arm) */
static	Sym*	sym_mod;
/*e: global sym_mod(arm) */
/*s: global sym_modu(arm) */
static	Sym*	sym_modu;
/*e: global sym_modu(arm) */

/*s: function noops(arm) */
void
noops(void)
{
    Prog *p, *q, *q1;
    int o;

    /*
     * find leaf subroutines
     * strip NOPs
     * expand RET
     */

    DBG("%5.2f noops\n", cputime());

    // pass 1
    curtext = P;
    for(p = firstp; p != P; p = p->link) {
        /*s: adjust curtext when iterate over instructions p */
        if(p->as == ATEXT)
            curtext = p;
        /*e: adjust curtext when iterate over instructions p */
        switch(p->as) {
        /*s: [[noops()]] first pass switch opcode cases */
        case ATEXT:
            p->mark |= LEAF;
            break;

        case ABL:
            if(curtext != P)
                curtext->mark &= ~LEAF;
            // fallthrough
        /*s: [[noops()]] first pass switch opcode ABL fallthrough */
        case AB:

        case ABEQ:
        case ABNE:
        case ABCS:
        case ABHS:
        case ABCC:
        case ABLO:
        case ABMI:
        case ABPL:
        case ABVS:
        case ABVC:
        case ABHI:
        case ABLS:
        case ABGE:
        case ABLT:
        case ABGT:
        case ABLE:

        case ABCASE:

            q1 = p->cond;
            if(q1 != P) {
                while(q1->as == ANOP) {
                    q1 = q1->link;
                    p->cond = q1;
                }
            }
            break;
        /*e: [[noops()]] first pass switch opcode ABL fallthrough */
        /*x: [[noops()]] first pass switch opcode cases */
        case ADIV:
        case ADIVU:
        case AMOD:
        case AMODU:
            if(prog_div == P)
                initdiv();
            if(curtext != P)
                curtext->mark &= ~LEAF;
            continue;
        /*e: [[noops()]] first pass switch opcode cases */
        }
    }

    // pass 2
    curtext = P;
    for(p = firstp; p != P; p = p->link) {
        /*s: adjust curtext when iterate over instructions p */
        if(p->as == ATEXT)
            curtext = p;
        /*e: adjust curtext when iterate over instructions p */
        o = p->as;
        switch(o) {
        /*s: [[noops()]] second pass swith opcode cases */
        case ATEXT:
            autosize = p->to.offset + 4;
            if(autosize <= 4)
              if(curtext->mark & LEAF) {
                p->to.offset = -4;
                autosize = 0;
            }

            if(!autosize && !(curtext->mark & LEAF)) {
                DBG("save suppressed in: %s\n", curtext->from.sym->name);
                curtext->mark |= LEAF;
            }

            if(curtext->mark & LEAF) {
                if(curtext->from.sym)
                    curtext->from.sym->type = SLEAF;
                if(!autosize)
                    break;
            }

            // MOVW R14, -autosize(SP)
            q1 = prg();
            q1->as = AMOVW;
            q1->scond |= C_WBIT;
            q1->line = p->line;
            q1->from.type = D_REG;
            q1->from.reg = REGLINK;
            q1->to.type = D_OREG;
            q1->to.offset = -autosize;
            q1->to.reg = REGSP;

            q1->link = p->link;
            p->link = q1;
            break;
        /*x: [[noops()]] second pass swith opcode cases */
        case ARET:
            nocache(p);
            if((curtext->mark & LEAF) && !autosize) {
                // B (R14)
                p->as = AB;
                p->from = zprg.from;
                p->to.type = D_OREG;
                p->to.offset = 0;
                p->to.reg = REGLINK;
            } else {
                // MOVW autosize(SP), PC
                p->as = AMOVW;
                p->scond |= C_PBIT;
                p->from.type = D_OREG;
                p->from.offset = autosize;
                p->from.reg = REGSP;
                p->to.type = D_REG;
                p->to.reg = REGPC;
            }
            break;
        /*x: [[noops()]] second pass swith opcode cases */
        case ADIV:
        case ADIVU:
        case AMOD:
        case AMODU:
            if(debug['M'])
                break;
            if(p->from.type != D_REG)
                break;
            if(p->to.type != D_REG)
                break;
            q1 = p;

            q = prg();
            q->link = p->link;
            p->link = q;
            p = q;

            /* MOV a,4(SP) */
            p->as = AMOVW;
            p->line = q1->line;
            p->from.type = D_REG;
            p->from.reg = q1->from.reg;
            p->to.type = D_OREG;
            p->to.reg = REGSP;
            p->to.offset = 4;

            q = prg();
            q->link = p->link;
            p->link = q;
            p = q;

            /* MOV b,REGTMP */
            p->as = AMOVW;
            p->line = q1->line;
            p->from.type = D_REG;
            p->from.reg = q1->reg;
            if(q1->reg == R_NONE)
                p->from.reg = q1->to.reg;
            p->to.type = D_REG;
            p->to.reg = REGTMP;
            p->to.offset = 0;

            q = prg();
            q->link = p->link;
            p->link = q;
            p = q;

            /* CALL appropriate */
            p->as = ABL;
            p->line = q1->line;
            p->to.type = D_BRANCH;
            p->cond = p;
            switch(o) {
            case ADIV:
                p->cond = prog_div;
                p->to.sym = sym_div;
                break;
            case ADIVU:
                p->cond = prog_divu;
                p->to.sym = sym_divu;
                break;
            case AMOD:
                p->cond = prog_mod;
                p->to.sym = sym_mod;
                break;
            case AMODU:
                p->cond = prog_modu;
                p->to.sym = sym_modu;
                break;
            }

            q = prg();
            q->link = p->link;
            p->link = q;
            p = q;

            /* MOV REGTMP, b */
            p->as = AMOVW;
            p->line = q1->line;
            p->from.type = D_REG;
            p->from.reg = REGTMP;
            p->from.offset = 0;
            p->to.type = D_REG;
            p->to.reg = q1->to.reg;

            q = prg();
            q->link = p->link;
            p->link = q;
            p = q;

            /* ADD $8,SP */
            p->as = AADD;
            p->from.type = D_CONST;
            p->from.reg = R_NONE;
            p->from.offset = 8;
            p->reg = R_NONE;
            p->to.type = D_REG;
            p->to.reg = REGSP;

            /* SUB $8,SP */
            q1->as = ASUB;
            q1->from.type = D_CONST;
            q1->from.offset = 8;
            q1->from.reg = R_NONE;
            q1->reg = R_NONE;
            q1->to.type = D_REG;
            q1->to.reg = REGSP;
            break;
        /*x: [[noops()]] second pass swith opcode cases */
        /*
         * 5c code generation for unsigned -> double made the
         * unfortunate assumption that single and double floating
         * point registers are aliased - true for emulated 7500
         * but not for vfp.  Now corrected, but this test is
         * insurance against old 5c compiled code in libraries.
         */
        case AMOVWD:
            if((q = p->link) != P && q->as == ACMP)
             if((q = q->link) != P && q->as == AMOVF)
              if((q1 = q->link) != P && q1->as == AADDF)
               if(q1->to.type == D_FREG && q1->to.reg == p->to.reg) {
                q1->as = AADDD;
                q1 = prg();
                q1->scond = q->scond;
                q1->line = q->line;
                q1->as = AMOVFD;
                q1->from = q->to;
                q1->to = q1->from;
                q1->link = q->link;
                q->link = q1;
            }
            break;
        /*e: [[noops()]] second pass swith opcode cases */
        }
    }
}
/*e: function noops(arm) */

/*s: function sigdiv(arm) */
static void
sigdiv(char *n)
{
    Sym *s;

    s = lookup(n, 0);
    if(s->type == STEXT){
        if(s->sig == 0)
            s->sig = SIGNINTERN;
    }
    else if(s->type == SNONE || s->type == SXREF)
        s->type = SUNDEF;
}
/*e: function sigdiv(arm) */

/*s: function divsig(arm) */
void
divsig(void)
{
    sigdiv("_div");
    sigdiv("_divu");
    sigdiv("_mod");
    sigdiv("_modu");
}
/*e: function divsig(arm) */

/*s: function sdiv(arm) */
static void
sdiv(Sym *s)
{
    if(s->type == SNONE || s->type == SXREF){
        /* undefsym(s); */
        s->type = SXREF;
        if(s->sig == 0)
            s->sig = SIGNINTERN;
        s->subtype = SIMPORT;
    }
    else if(s->type != STEXT)
        diag("undefined: %s", s->name);
}
/*e: function sdiv(arm) */

/*s: function initdiv(arm) */
void
initdiv(void)
{
    Sym *s2, *s3, *s4, *s5;
    Prog *p;

    if(prog_div != P)
        return;
    sym_div = s2 = lookup("_div", 0);
    sym_divu = s3 = lookup("_divu", 0);
    sym_mod = s4 = lookup("_mod", 0);
    sym_modu = s5 = lookup("_modu", 0);
    if(dlm) {
        sdiv(s2); if(s2->type == SXREF) prog_div = UP;
        sdiv(s3); if(s3->type == SXREF) prog_divu = UP;
        sdiv(s4); if(s4->type == SXREF) prog_mod = UP;
        sdiv(s5); if(s5->type == SXREF) prog_modu = UP;
    }
    for(p = firstp; p != P; p = p->link)
        if(p->as == ATEXT) {
            if(p->from.sym == s2)
                prog_div = p;
            if(p->from.sym == s3)
                prog_divu = p;
            if(p->from.sym == s4)
                prog_mod = p;
            if(p->from.sym == s5)
                prog_modu = p;
        }
    if(prog_div == P) {
        diag("undefined: %s", s2->name);
        prog_div = curtext;
    }
    if(prog_divu == P) {
        diag("undefined: %s", s3->name);
        prog_divu = curtext;
    }
    if(prog_mod == P) {
        diag("undefined: %s", s4->name);
        prog_mod = curtext;
    }
    if(prog_modu == P) {
        diag("undefined: %s", s5->name);
        prog_modu = curtext;
    }
}
/*e: function initdiv(arm) */

/*s: function nocache(arm) */
void
nocache(Prog *p)
{
    p->optab = 0;
    p->from.class = C_NONE;
    p->to.class = C_NONE;
}
/*e: function nocache(arm) */
/*e: linkers/5l/noop.c */
