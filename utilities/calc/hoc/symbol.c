/*s: hoc/symbol.c */
#include <u.h>
#include <libc.h>
#include "hoc.h"
#include "y.tab.h"

static Symbol *symlist = 0;  /* symbol table: linked list */

/*s: function [[lookup]](hoc) */
Symbol*
lookup(char* s)	/* find s in symbol table */
{
    Symbol *sp;

    for (sp = symlist; sp != (Symbol *) 0; sp = sp->next)
        if (strcmp(sp->name, s) == 0)
            return sp;
    return 0;	/* 0 ==> not found */	
}
/*e: function [[lookup]](hoc) */

/*s: function [[install]](hoc) */
Symbol*
install(char* s, int t, double d)  /* install s in symbol table */
{
    Symbol *sp;

    sp = emalloc(sizeof(Symbol));
    sp->name = emalloc(strlen(s)+1); /* +1 for '\0' */
    strcpy(sp->name, s);
    sp->type = t;
    sp->u.val = d;
    sp->next = symlist; /* put at front of list */
    symlist = sp;
    return sp;
}
/*e: function [[install]](hoc) */

/*s: function [[emalloc]](hoc) */
void*
emalloc(unsigned n)	/* check return from malloc */
{
    char *p;

    p = malloc(n);
    if (p == 0)
        execerror("out of memory", (char *) 0);
    return p;
}
/*e: function [[emalloc]](hoc) */

/*s: function [[formallist]](hoc) */
Formal*
formallist(Symbol *formal, Formal *list)	/* add formal to list */
{
    Formal *f;

    f = emalloc(sizeof(Formal));
    f->sym = formal;
    f->save = 0;
    f->next = list;
    return f;
}
/*e: function [[formallist]](hoc) */
/*e: hoc/symbol.c */
