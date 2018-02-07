/*s: mk/file.c */
#include	"mk.h"

// was in plan9.c 
/*s: function [[chgtime]] */
int
chgtime(char *name)
{
    Dir sbuf;

    if(access(name, AEXIST) >= 0) {
        nulldir(&sbuf);
        sbuf.mtime = time((long *)nil);
        return dirwstat(name, &sbuf);
    }
    return close(create(name, OWRITE, 0666));
}
/*e: function [[chgtime]] */

// was in plan9.c 
/*s: function [[dirtime]] */
void
dirtime(char *dir, char *path)
{
    int i, fd, n;
    ulong mtime;
    Dir *d;
    char buf[4096];

    fd = open(dir, OREAD);
    if(fd >= 0){
        while((n = dirread(fd, &d)) > 0){
            for(i=0; i<n; i++){
                mtime = d[i].mtime;
                /* defensive driving: this does happen */
                if(mtime == 0)
                    mtime = 1;
                snprint(buf, sizeof buf, "%s%s", path,
                    d[i].name);
                if(symlook(buf, S_TIME, 0) == nil)
                    symlook(strdup(buf), S_TIME,
                        (void*)mtime)->u.value = mtime;
            }
            free(d);
        }
        close(fd);
    }
}
/*e: function [[dirtime]] */

// was in plan9.c 
/*s: function [[bulkmtime]] */
void
bulkmtime(char *dir)
{
    char buf[4096];
    char *ss, *s, *sym;

    if(dir){
        sym = dir;
        s = dir;
        if(strcmp(dir, "/") == 0)
            strecpy(buf, buf + sizeof buf - 1, dir);
        else
            snprint(buf, sizeof buf, "%s/", dir);
    }else{
        s = ".";
        sym = "";
        buf[0] = 0;
    }
    if(symlook(sym, S_BULKED, 0))
        return;
    // else
    ss = strdup(sym);
    symlook(ss, S_BULKED, (void*)ss);
    dirtime(s, buf);
}
/*e: function [[bulkmtime]] */

// was in plan9.c 
/*s: function [[mkmtime]] */
ulong
mkmtime(char *name, bool force)
{
    Dir *d;
    ulong t;
    /*s: [[mkmtime]] locals */
    //char *s, *ss;
    //char carry;
    //Symtab *sym;
    /*x: [[mkmtime]] locals */
    char buf[4096];
    /*e: [[mkmtime]] locals */

    /*s: [[mkmtime()]] bulk dir optimisation */
    /*s: [[mkmtime()]] cleanup name */
    strecpy(buf, buf + sizeof buf - 1, name);
    cleanname(buf);
    name = buf;
    /*e: [[mkmtime()]] cleanup name */
    USED(force);
    //TODO    s = utfrrune(name, '/');
    //TODO    if(s == name)
    //TODO        s++;
    //TODO    if(s){
    //TODO        ss = name;
    //TODO        carry = *s;
    //TODO        *s = '\0';
    //TODO    }else{
    //TODO        ss = nil;
    //TODO        carry = '\0';
    //TODO    }
    //TODO    if(carry)
    //TODO        *s = carry;
    //TODO
    //TODO bulkmtime(ss);
    //TODO if(!force){
    //TODO     sym = symlook(name, S_TIME, 0);
    //TODO     if(sym)
    //TODO         return sym->u.value;
    //TODO     return 0;
    //TODO }
    /*e: [[mkmtime()]] bulk dir optimisation */
    d = dirstat(name);
    /*s: [[mkmtime()]] check if inexistent file */
    if(d == nil)
        return 0;
    /*e: [[mkmtime()]] check if inexistent file */
    t = d->mtime;
    free(d);

    return t;
}
/*e: function [[mkmtime]] */


/* table-driven version in bootes dump of 12/31/96 */

/*s: function [[timeof]] */
ulong
timeof(char *name, bool force)
{
    /*s: [[timeof()]] locals */
    ulong t;
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
        return sym->u.value;		/* uggh */
    /*e: [[timeof()]] check time cache */
    t = mkmtime(name, false);
    /*s: [[timeof()]] update time cache */
    if(t == 0)
        return 0;
    symlook(name, S_TIME, (void*)t);		/* install time in cache */
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
    ulong t;
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

        symlook(strdup(cp), S_TIME, (void *)t)->u.value = t;

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
