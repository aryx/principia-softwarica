/*s: machine/5i/symbols.c */
/*s: basic includes */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "arm.h"
/*e: basic includes */

/*s: constant STRINGSZ */
#define	STRINGSZ	128
/*e: constant STRINGSZ */

void		printlocals(Symbol*, ulong);

/*s: function printsource */
/*
 *	print the value of dot as file:line
 */
void
printsource(long dot)
{
    char str[STRINGSZ];

    if (fileline(str, STRINGSZ, dot))
        Bprint(bioout, "%s", str);
}
/*e: function printsource */

/*s: function printlocals */
void
printlocals(Symbol *fn, ulong fp)
{
    int i;
    Symbol s;

    s = *fn;
    for (i = 0; localsym(&s, i); i++) {
        if (s.class != CAUTO)
            continue;
        Bprint(bioout, "\t%s=#%lux\n", s.name, getmem_4(fp-s.value));
    }
}
/*e: function printlocals */

/*s: function printparams */
void
printparams(Symbol *fn, ulong fp)
{
    int i;
    Symbol s;
    int first;

    fp += mach->szreg;			/* skip saved pc */
    s = *fn;
    for (first = i = 0; localsym(&s, i); i++) {
        if (s.class != CPARAM)
            continue;
        if (first++)
            Bprint(bioout, ", ");
        Bprint(bioout, "%s=#%lux", s.name, getmem_4(fp+s.value));
    }
    Bprint(bioout, ") ");
}
/*e: function printparams */
/*s: constant STARTSYM */
#define STARTSYM	"_main"
/*e: constant STARTSYM */
/*s: constant FRAMENAME */
#define	FRAMENAME	".frame"
/*e: constant FRAMENAME */

/*s: function stktrace */
void
stktrace(int modif)
{
    ulong pc, sp;
    Symbol s, f;
    int i;
    char buf[512];

    pc = reg.r[REGPC];
    sp = reg.r[REGSP];
    i = 0;
    while (findsym(pc, CTEXT, &s)) {
        if(strcmp(STARTSYM, s.name) == 0) {
            Bprint(bioout, "%s() at #%llux\n", s.name, s.value);
            break;
        }
        if (pc == s.value)	/* at first instruction */
            f.value = 0;
        else if (findlocal(&s, FRAMENAME, &f) == 0)
            break;
        if (s.type == 'L' || s.type == 'l' || pc <= s.value+4)
            pc = reg.r[REGLINK];
        else pc = getmem_4(sp);
        sp += f.value;
        Bprint(bioout, "%s(", s.name);
        printparams(&s, sp);
        printsource(s.value);
        Bprint(bioout, " called from ");
        symoff(buf, sizeof(buf), pc-8, CTEXT);
        Bprint(bioout, buf);
        printsource(pc-8);
        Bprint(bioout, "\n");
        if(modif == 'C')
            printlocals(&s, sp);
        if(++i > 40){
            Bprint(bioout, "(trace truncated)\n");
            break;
        }
    }
}
/*e: function stktrace */
/*e: machine/5i/symbols.c */
