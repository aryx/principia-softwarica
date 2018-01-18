/*s: machine/5i/stats.c */
/*s: basic includes */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "arm.h"
/*e: basic includes */

/*s: function [[Percent]] */
#define Percent(num, max)	((max)?((num)*100)/(max):0)
/*e: function [[Percent]] */

typedef struct Prof Prof;

/*s: global [[tables]] */
Inst *tables[] = { itab, 0 };
/*e: global [[tables]] */

/*s: function [[isum]] */
void
isum(void)
{
    Inst *i;
    int total, mems, arith, branch;
    int useddelay, taken, syscall;
    int pct, j;

    total = 0;
    mems = 0;
    arith = 0;
    branch = 0;
    useddelay = 0;
    taken = 0;
    syscall = 0;

    /* Compute the total so we can have percentages */
    for(i = itab; i->func; i++)
        if(i->name && i->count)
            total += i->count;

    Bprint(bout, "\nInstruction summary.\n\n");

    for(j = 0; tables[j]; j++) {
        for(i = tables[j]; i->func; i++) {
            if(i->name) {
                /* This is gross */
                if(i->count == 0)
                    continue;
                pct = Percent(i->count, total);
                if(pct != 0)
                    Bprint(bout, "%-8ud %3d%% %s\n",
                        i->count, Percent(i->count,
                        total), i->name);
                else
                    Bprint(bout, "%-8ud      %s\n",
                        i->count, i->name);


                switch(i->type) {
                case Imem:
                    mems += i->count;
                    break;
                case Iarith:
                    arith += i->count;
                    break;
                case Ibranch:
                    branch += i->count;
                    taken += i->taken;
                    useddelay += i->useddelay;
                    break;
                case Isyscall:
                    syscall += i->count;
                    break;
                default:
                    fatal(false, "isum bad stype %d\n", i->type);
                }
        
            }
        }
    }

    Bprint(bout, "\n%-8ud      Memory cycles\n", mems+total);	
    Bprint(bout, "%-8ud %3d%% Instruction cycles\n",
            total, Percent(total, mems+total));
    Bprint(bout, "%-8ud %3d%% Data cycles\n\n",
            mems, Percent(mems, mems+total));	

    Bprint(bout, "%-8ud %3d%% Arithmetic\n",
            arith, Percent(arith, total));

    Bprint(bout, "%-8ud %3d%% System calls\n",
            syscall, Percent(syscall, total));

    Bprint(bout, "%-8ud %3d%% Branches\n",
            branch, Percent(branch, total));

    Bprint(bout, "   %-8ud %3d%% Branches taken\n",
            taken, Percent(taken, branch));

    Bprint(bout, "   %-8ud %3d%% Delay slots\n",
            useddelay, Percent(useddelay, branch));

    Bprint(bout, "   %-8ud %3d%% Unused delay slots\n", 
            branch-useddelay, Percent(branch-useddelay, branch));

    Bprint(bout, "%-8ud %3d%% Program total delay slots\n",
            nopcount, Percent(nopcount, total));
}
/*e: function [[isum]] */

/*s: function [[tlbsum]] */
void
tlbsum(void)
{
    if(tlb.on == false)
        return;

    Bprint(bout, "\n\nTlb summary\n");

    Bprint(bout, "\n%-8d User entries\n", tlb.tlbsize);
    Bprint(bout, "%-8d Accesses\n", tlb.hit+tlb.miss);
    Bprint(bout, "%-8d Tlb hits\n", tlb.hit);
    Bprint(bout, "%-8d Tlb misses\n", tlb.miss);
    Bprint(bout, "%7d%% Hit rate\n", Percent(tlb.hit, tlb.hit+tlb.miss));
}
/*e: function [[tlbsum]] */

/*s: global [[stype]] */
char *stype[] = { "Stack", "Text", "Data", "Bss" };
/*e: global [[stype]] */

/*s: function [[segsum]] */
void
segsum(void)
{
    Segment *s;
    int i;

    Bprint(bout, "\n\nMemory Summary\n\n");
    Bprint(bout, "      Base     End      Resident References\n");
    for(i = 0; i < Nseg; i++) {
        s = &memory.seg[i];
        Bprint(bout, "%-5s %.8lux %.8lux %-8d %-8d\n",
                stype[i], s->base, s->end, s->rss*BY2PG, s->refs);
    }
}
/*e: function [[segsum]] */

/*s: struct [[Prof]] */
struct Prof
{
    Symbol	s;
    long	count;
};
/*e: struct [[Prof]] */
/*s: global [[aprof]] */
// can't use prof, conflict with libc.h prof()
Prof	aprof[5000];
/*e: global [[aprof]] */

/*s: function [[profcmp]] */
int
profcmp(void *va, void *vb)
{
    Prof *a, *b;

    a = va;
    b = vb;
    return b->count - a->count;
}
/*e: function [[profcmp]] */

/*s: function [[iprofile]] */
void
iprofile(void)
{
    Prof *p, *n;
    int i, b, e;
    ulong total;

    i = 0;
    p = aprof;
    if(textsym(&p->s, i) == 0)
        return;
    i++;
    for(;;) {
        n = p+1;
        if(textsym(&n->s, i) == 0)
            break;
        b = (p->s.value-textbase)/PROFGRAN;
        e = (n->s.value-textbase)/PROFGRAN;
        while(b < e)
            p->count += iprof[b++];
        i++;
        p = n;
    }

    qsort(prof, i, sizeof(Prof), profcmp);

    total = 0;
    for(b = 0; b < i; b++)
        total += aprof[b].count;

    Bprint(bout, "  cycles     %% symbol          file\n");
    for(b = 0; b < i; b++) {
        if(aprof[b].count == 0)
            continue;

        Bprint(bout, "%8ld %3ld.%ld %-15s ",
            aprof[b].count,
            100*aprof[b].count/total,
            (1000*aprof[b].count/total)%10,
            aprof[b].s.name);

        printsource(aprof[b].s.value);
        Bputc(bout, '\n');
    }
    memset(prof, 0, sizeof(Prof)*i);
}
/*e: function [[iprofile]] */
/*e: machine/5i/stats.c */
