/*s: linkers/5l/utils.c */
#include "l.h"

// Utilities.

/*s: function [[log]] */
void mylog(char *fmt, ...) {

    va_list arg;

    va_start(arg, fmt);
    Bvprint(&bso, fmt, arg);
    va_end(arg);
    Bflush(&bso);
}
/*e: function [[log]] */

/*s: constructor [[prg]] */
Prog*
prg(void)
{
    Prog *p;

    p = malloc(sizeof(Prog));
    *p = zprg;
    return p;
}
/*e: constructor [[prg]] */

/*s: function [[lookup]] */
Sym*
lookup(char *symb, int v)
{
    Sym *sym;
    long h;
    int len;
    /*s: [[lookup()]] other locals */
    char *p;
    int c;
    /*e: [[lookup()]] other locals */

    /*s: [[lookup()]] compute hash value [[h]] of [[(symb, v)]] and [[len]] */
    // h = hashcode(symb, v); 
    // len = strlen(symb);
    h = v;
    for(p=symb; *p; p++) {
        c = *p;
        h = h+h+h + c;
    }
    len = (p - symb) + 1;
    h &= 0xffffff;
    h %= NHASH;
    /*e: [[lookup()]] compute hash value [[h]] of [[(symb, v)]] and [[len]] */
    
    // sym = hash_lookup((symb, v), h, hash)
    for(sym = hash[h]; sym != S; sym = sym->link)
        if(sym->version == v)
            if(memcmp(sym->name, symb, len) == 0)
                return sym;

    // else
    /*s: [[lookup()]] if symbol name not found */
    sym = malloc(sizeof(Sym));
    sym->name = malloc(len + 1); // +1 again?
    memmove(sym->name, symb, len);
    sym->version = v;

    sym->value = 0;
    sym->type = SNONE;
    sym->sig = 0;

    // add_hash(sym, hash)
    sym->link = hash[h];
    hash[h] = sym;

    /*s: [[lookup()]] profiling */
    nsymbol++;
    /*e: [[lookup()]] profiling */
    return sym;
    /*e: [[lookup()]] if symbol name not found */
}
/*e: function [[lookup]] */

/*s: function [[atolwhex]] */
long
atolwhex(char *s)
{
    long n;
    int f;

    n = 0;
    f = 0;
    while(*s == ' ' || *s == '\t')
        s++;
    if(*s == '-' || *s == '+') {
        if(*s++ == '-')
            f = 1;
        while(*s == ' ' || *s == '\t')
            s++;
    }
    if(s[0]=='0' && s[1]){
        if(s[1]=='x' || s[1]=='X'){
            s += 2;
            for(;;){
                if(*s >= '0' && *s <= '9')
                    n = n*16 + *s++ - '0';
                else if(*s >= 'a' && *s <= 'f')
                    n = n*16 + *s++ - 'a' + 10;
                else if(*s >= 'A' && *s <= 'F')
                    n = n*16 + *s++ - 'A' + 10;
                else
                    break;
            }
        } else
            while(*s >= '0' && *s <= '7')
                n = n*8 + *s++ - '0';
    } else
        while(*s >= '0' && *s <= '9')
            n = n*10 + *s++ - '0';
    if(f)
        n = -n;
    return n;
}
/*e: function [[atolwhex]] */

/*s: function [[rnd]] */
long
rnd(long v, long r)
{
    long c;

    /*s: [[rnd()]] if r is null or negative */
    if(r <= 0)
        return v;
    /*e: [[rnd()]] if r is null or negative */
    v += r - 1;
    c = v % r;
    /*s: [[rnd()]] if v was negative */
    if(c < 0)
        c += r;
    /*e: [[rnd()]] if v was negative */
    v -= c;
    return v;
}
/*e: function [[rnd]] */

/*s: function [[fileexists]] */
int
fileexists(char *s)
{
    byte dirbuf[400];

    /* it's fine if stat result doesn't fit in dirbuf, since even then the file exists */
    return stat(s, dirbuf, sizeof(dirbuf)) >= 0;
}
/*e: function [[fileexists]] */

/*s: global [[hunk]] */
char*	hunk;
/*e: global [[hunk]] */
/*s: global [[nhunk]] */
long	nhunk;
/*e: global [[nhunk]] */
// thunk defined in globals.c because also used by main.c for profiling report

/*s: function [[gethunk]] */
void
gethunk(void)
{
    char *h;
    long nh;

    nh = NHUNK;
    if(thunk >= 5L*NHUNK) {
        nh = 5L*NHUNK;
        if(thunk >= 25L*NHUNK)
            nh = 25L*NHUNK;
    }
    h = sbrk(nh);
    if(h == (char*)-1) {
        diag("out of memory");
        errorexit();
    }
    hunk = h;
    nhunk = nh;
    thunk += nh;
}
/*e: function [[gethunk]] */

/*s: function [[malloc]] */
/*
 * fake malloc
 */
void*
malloc(ulong n)
{
    void *p;

    // upper_round(n, 8)
    while(n & 7)
        n++;

    while(nhunk < n)
        gethunk();
    p = hunk;
    nhunk -= n;
    hunk += n;
    return p;
}
/*e: function [[malloc]] */

/*s: function [[free]] */
void
free(void *p)
{
    USED(p);
}
/*e: function [[free]] */

/*s: function [[setmalloctag]] */
//@Scheck: looks dead, but because we redefine malloc/free we must also redefine that
void setmalloctag(void *v, ulong pc)
{
    USED(v, pc);
}
/*e: function [[setmalloctag]] */

/*e: linkers/5l/utils.c */
