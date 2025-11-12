/*s: linkers/5l/layout.c */
#include	"l.h"
#include	"m.h"

// Data layout and relocation.

/*s: global [[pool]](arm) */
static struct {
    // PC of first instruction referencing the pool
    ulong	start;
    // a multiple of 4
    ulong	size;
} pool;
/*e: global [[pool]](arm) */

/*s: global [[blitrl]](arm) */
// list<ref_own<prog>> (next = Prog.link)
Prog*	blitrl;
/*e: global [[blitrl]](arm) */
/*s: global [[elitrl]](arm) */
// ref<Prog> (end from = blitrl)
Prog*	elitrl;
/*e: global [[elitrl]](arm) */

void	checkpool(Prog*);
void 	flushpool(Prog*, int);
void    addpool(Prog*, Adr*);



/*s: function [[xdefine]](arm) */
void
xdefine(char *p, int t, long v)
{
    Sym *s;

    s = lookup(p, 0);
    if(s->type == SNONE || s->type == SXREF) {
        s->type = t;
        s->value = v;
    }
}
/*e: function [[xdefine]](arm) */


/*s: function [[dodata]](arm) */
/// main -> <>
void
dodata(void)
{
    Prog *p;
    Sym *s;
    // offset to start of data section
    long orig;
    // size of data
    long v;
    //enum<Section>
    int t;
    int i;

    DBG("%5.2f dodata\n", cputime());

    // DATA instructions loop
    for(p = datap; p != P; p = p->link) {
        s = p->from.sym;
        if(s->type == SBSS)
            s->type = SDATA;
        /*s: [[dodata()]] sanity check DATA instructions */
        if(s->type != SDATA)
            diag("initialize non-data (%d): %s\n%P",
                s->type, s->name, p);

        v = p->from.offset + p->reg;
        if(v > s->value)
            diag("initialize bounds (%ld): %s\n%P",
                s->value, s->name, p);
        /*e: [[dodata()]] sanity check DATA instructions */
    }

    /*s: [[dodata()]] if string in text segment */
    if(debug['t']) {
        /*
         * pull out string constants
         */
        for(p = datap; p != P; p = p->link) {
            s = p->from.sym;
            if(p->to.type == D_SCONST)
                s->type = SSTRING;
        }
    }
    /*e: [[dodata()]] if string in text segment */

    orig = 0;

    /*
     * pass 1
     *  sanity check data values, and align.
     */
    // symbol table loop
    for(i=0; i<NHASH; i++)
     for(s = hash[i]; s != S; s = s->link) {
        t = s->type;
        if(t == SDATA || t == SBSS) {
            v = s->value; // size of global for now
            /*s: [[dodata()]] sanity check GLOBL instructions, size of data v */
            if(v == 0) { // check
                diag("%s: no size", s->name);
                v = 1;
            }
            /*e: [[dodata()]] sanity check GLOBL instructions, size of data v */
            v = rnd(v, 4); // align
            s->value = v;   // adjust
            /*s: [[dodata()]] in pass 1, if small data size, adjust orig */
            /*
             *	assign 'small' variables to data segment
             *	(rational is that data segment is more easily
             *	 addressed through offset on R12)
             */
            if(v <= MINSIZ) {
                s->value = orig;
                orig += v;
                s->type = SDATA1;
            }
            /*e: [[dodata()]] in pass 1, if small data size, adjust orig */
        }
    }

    /*
     * pass 2
     *	assign (large) 'data' variables to data segment
     */
    for(i=0; i<NHASH; i++)
     for(s = hash[i]; s != S; s = s->link) {
        t = s->type;
        if(t == SDATA) {
            v = s->value;
            // s->value used to contain the size of the GLOBL.
            // Now it contains its location (as an offset to INITDAT)
            s->value = orig;
            orig += v;
        } else {
            /*s: [[dodata()]] in pass 2, retag small data */
            if(t == SDATA1)
                s->type = SDATA;
            /*e: [[dodata()]] in pass 2, retag small data */
        }
    }
    orig = rnd(orig, 8);

    datsize = orig;

    /*
     * pass 3
     *	everything else to bss segment
     */
    for(i=0; i<NHASH; i++)
     for(s = hash[i]; s != S; s = s->link) {
        if(s->type == SBSS) {
            v = s->value;
            s->value = orig;
            orig += v;
        }
    }
    orig = rnd(orig, 8);

    bsssize = orig-datsize;

    /*s: [[dodata()]] define special symbols */
    xdefine("bdata", SDATA, 0L);
    xdefine("edata", SDATA, datsize);
    xdefine("end",   SBSS, datsize+bsssize);
    /*x: [[dodata()]] define special symbols */
    xdefine("etext", STEXT, 0L);
    /*x: [[dodata()]] define special symbols */
    xdefine("setR12", SDATA, 0L+BIG);
    /*e: [[dodata()]] define special symbols */
}
/*e: function [[dodata]](arm) */

/*s: function [[span]](arm) */
/// main -> <>
void
dotext(void)
{
    Prog *p;
    Optab *o;
    // real code address
    long c; // ulong?
    // size of instruction
    int m;
    /*s: [[dotext()]] other locals */
    long    otxt;
    /*x: [[dotext()]] other locals */
    Sym *s;
    long v;
    int i;
    /*e: [[dotext()]] other locals */

    DBG("%5.2f span\n", cputime());

    c = INITTEXT;
    /*s: [[dotext()]] initialisation */
    otxt = c;
    /*e: [[dotext()]] initialisation */

    for(p = firstp; p != P; p = p->link) {
       /*s: adjust curtext when iterate over instructions p */
       if(p->as == ATEXT)
           curtext = p;
       /*e: adjust curtext when iterate over instructions p */
       /*s: adjust autosize when iterate over instructions p */
       if(p->as == ATEXT) {
           autosize = p->to.offset + 4;
       }
       /*e: adjust autosize when iterate over instructions p */

        // real program counter transition
        p->pc = c;

        o = oplook(p);
        m = o->size;
        c += m;

        if(m == 0) {
            if(p->as == ATEXT) {
                if(p->from.sym != S)
                    p->from.sym->value = c;
                /*s: [[dotext()]] detect if large procedure */
                if(c - otxt >= 1L<<17) {
                    diag("Procedure %s too large\n", TNAME);
                    errorexit();
                }
                otxt = c;
                /*e: [[dotext()]] detect if large procedure */
            } else {
                diag("zero-width instruction\n%P", p);
            }
        } else {
            /*s: [[dotext()]] pool handling for optab o */
            switch(o->flag & (LFROM|LTO|LPOOL)) {
            /*s: [[dotext()]] pool handling, switch flag cases */
            case LFROM:
                addpool(p, &p->from);
                break;
            case LTO:
                addpool(p, &p->to);
                break;
            /*x: [[dotext()]] pool handling, switch flag cases */
            case LPOOL:
                if ((p->scond&C_SCOND) == COND_ALWAYS)
                    flushpool(p, false);
                break;
            /*e: [[dotext()]] pool handling, switch flag cases */
            }
            /*s: [[dotext()]] pool handling, flush if MOVW REGPC */
            // MOVW ..., R15  => flush
            if(p->as==AMOVW && p->to.type==D_REG && p->to.reg==REGPC && 
               (p->scond&C_SCOND) == COND_ALWAYS)
                flushpool(p, false);
            /*e: [[dotext()]] pool handling, flush if MOVW REGPC */
            /*s: [[dotext()]] pool handling, checkpool */
            if(blitrl)
                checkpool(p);
            /*e: [[dotext()]] pool handling, checkpool */
            /*e: [[dotext()]] pool handling for optab o */
        }
    }

    /*s: [[dotext()]] if string in text segment */
    if(debug['t']) {
        /* 
         * add strings to text segment
         */
        c = rnd(c, 8);
        for(i=0; i<NHASH; i++)
         for(s = hash[i]; s != S; s = s->link)
          if(s->type == SSTRING) {
              v = s->value;
              v = rnd(v, 4);
              s->value = c;
              c += v;
          }
    }
    /*e: [[dotext()]] if string in text segment */

    c = rnd(c, 8);

    textsize = c - INITTEXT;
    /*s: [[dotext()]] refine special symbols */
    lookup("etext", 0)->value = INITTEXT+textsize;
    /*e: [[dotext()]] refine special symbols */
    if(INITRND)
        INITDAT = rnd(c, INITRND);
    DBG("tsize = %lux\n", textsize);
}
/*e: function [[span]](arm) */




/*s: function [[checkpool]](arm) */
void
checkpool(Prog *p)
{
    if(p->link == P)
        flushpool(p, true);
    else
    /*s: [[checkpool()]] if special condition */
    /*
     * When the first reference to the literal pool threatens
     * to go out of range of a 12-bit PC-relative offset,
     * drop the pool now, and branch round it.
     * This happens only in extended basic blocks that exceed 4k.
     */
    if(pool.size >= 0xffc || 
       immaddr((p->pc+4) + 4 + pool.size - pool.start + 8) == 0)
        flushpool(p, true);
    /*e: [[checkpool()]] if special condition */
}
/*e: function [[checkpool]](arm) */

/*s: function [[flushpool]](arm) */
/// (dotext -> checkpool) | dotext -> <>
void
flushpool(Prog *p, bool skip)
{
    Prog *q;

    if(blitrl) {
        /*s: [[flushpool()]] if skip or corner case */
        if(skip){
            DBG("note: flush literal pool at %lux: len=%lud ref=%lux\n", 
                p->pc+4, pool.size, pool.start);
            q = prg();
            q->as = AB;
            q->to.type = D_BRANCH;
            q->cond = p->link;
    
            //insert_list(q, blitrl)
            q->link = blitrl;
            blitrl = q;
        }
        /*s: [[flushpool()]] else if not skip and corner case */
        else if((p->pc + pool.size - pool.start) < 2048)
            return;
        /*e: [[flushpool()]] else if not skip and corner case */
        /*e: [[flushpool()]] if skip or corner case */

        //insert_list_after_elt(blitlr, elitrl, p)
        elitrl->link = p->link;
        p->link = blitrl;

        blitrl = nil;/* BUG: should refer back to values until out-of-range */
        elitrl = nil;
        pool.size = 0;
        pool.start = 0;
    }
}
/*e: function [[flushpool]](arm) */

/*s: function [[addpool]](arm) */
void
addpool(Prog *p, Adr *a)
{
    Prog *q;
    Prog t;
    //enum<Operand_class>
    int c;

    c = aclass(a);

    t = zprg;
    t.as = AWORD;
    /*s: [[addpool()]] set t.to using a */
    switch(c) {
    /*s: [[addpool()]] switch operand class [[c]] cases */
    // C_LCON|C_xCON, C_LEXT|C_xEXT? TODO warning if other case?
    default:
        t.to = *a;
        break;
    /*x: [[addpool()]] switch operand class [[c]] cases */
    case C_SROREG:
    case C_LOREG:
    case C_ROREG:
    case C_FOREG:
    case C_SOREG:
    case C_FAUTO:
    case C_SAUTO:
    case C_LAUTO:
    case C_LACON:
        t.to.type = D_CONST;
        t.to.offset = instoffset;
        break;
    /*e: [[addpool()]] switch operand class [[c]] cases */
    }
    /*e: [[addpool()]] set t.to using a */

    /*s: [[addpool()]] if literal already present in pool */
    // find_list(t.to, blitrl)
    for(q = blitrl; q != P; q = q->link)
        if(memcmp(&q->to, &t.to, sizeof(Adr)) == 0) {
            // for omvl()
            p->cond = q;
            return;
        }
    /*e: [[addpool()]] if literal already present in pool */
    // else

    q = prg();
    *q = t;

    /*s: [[addpool()]] set pool.start and pool.size */
    // will be overwritten when dotext() layout the pool later
    q->pc = pool.size;

    if(blitrl == P) {
        pool.start = p->pc;
    }
    pool.size += 4;
    /*e: [[addpool()]] set pool.start and pool.size */

    // add_queue(q, blitrl, elitrl)
    if(blitrl == P) {
        blitrl = q;
    } else
        elitrl->link = q;
    elitrl = q;

    // for omvl()!
    p->cond = q;
}
/*e: function [[addpool]](arm) */

/*e: linkers/5l/layout.c */
