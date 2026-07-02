/*s: mk/file.c */
#include	"mk.h"

// chgtime() is back in Plan9.c 
extern int chgtime(char *name);
// dirtime() is back in Plan9.c 
// bulkmtime() is back in Plan9.c 
// mkmtime is back in Plan9.c 

/* table-driven version in bootes dump of 12/31/96 */

/*s: function [[timeof]] */
/// newnode -> <> -> mkmtime -> (bulktime; dirstat (libc))
double
timeof(char *name, bool force)
{
    /*s: [[timeof()]] locals */
    //old: was ulong
    double t;
    /*x: [[timeof()]] locals */
    Symtab *sym;
    /*e: [[timeof()]] locals */

    /*s: [[timeof()]] if name archive member */
    if(utfrune(name, '('))
        return atimeof(force, name);		/* archive */
    /*e: [[timeof()]] if name archive member */
    if(force)
        return mkmtime(name, true);
    // else
    /*s: [[timeof()]] if not force, use time cache */
    /*s: [[timeof()]] check time cache */
    sym = symlook(name, S_TIME, nil);
    if (sym)
        return sym->u.time;		/* uggh */
    /*e: [[timeof()]] check time cache */
    t = mkmtime(name, false);
    /*s: [[timeof()]] update time cache */
    if(t == 0)
        return 0;
    symlook(name, S_TIME, &t);		/* install time in cache */
    /*e: [[timeof()]] update time cache */
    return t;
    /*e: [[timeof()]] if not force, use time cache */
}
/*e: function [[timeof]] */

/*s: function [[touch]] */
void
touch(char *name)
{
    Bprint(&bout, "touch(%s)\n", name);
    if(nflag)
        return;

    if(utfrune(name, '('))
        atouch(name);		/* archive */
    else
     if(chgtime(name) < 0) {
        perror(name);
        Exit();
    }
}
/*e: function [[touch]] */

/*s: function [[delete]] */
void
delete(char *name)
{
    if(utfrune(name, '(') == nil) {		/* file */
        if(remove(name) < 0)
            perror(name);
    } else
        fprint(STDERR, "hoon off; mk can't delete archive members\n");
}
/*e: function [[delete]] */

/*s: function [[timeinit]] */
void
timeinit(char *s)
{
    //old: was ulong
    double t;
    char *cp;
    Rune r;
    int c, n;

    t = time(nil);
    while (*s) {
        cp = s;
        do{
            n = chartorune(&r, s);
            if (r == ' ' || r == ',' || r == '\n')
                break;
            s += n;
        } while(*s);
        c = *s;
        *s = '\0';

        symlook(strdup(cp), S_TIME, &t)->u.time = t;

        if (c)
            *s++ = c;
        while(*s){
            n = chartorune(&r, s);
            if(r != ' ' && r != ',' && r != '\n')
                break;
            s += n;
        }
    }
}
/*e: function [[timeinit]] */
/*e: mk/file.c */
