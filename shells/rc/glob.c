/*s: rc/glob.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

int	matchfn(void*, void*);

// was in plan9.c
/*s: constant [[NDIR]] */
#define	NDIR	256		/* shoud be a better way */
/*e: constant [[NDIR]] */
/*s: function [[Globsize]] */
int
Globsize(char *p)
{
    int isglob = 0, globlen = NDIR+1;
    for(;*p;p++){
        if(*p==GLOB){
            p++;
            if(*p!=GLOB)
                isglob++;
            globlen+=*p=='*'?NDIR:1;
        }
        else
            globlen++;
    }
    return isglob?globlen:0;
}
/*e: function [[Globsize]] */



/*s: global [[globname]] */
char *globname;
/*e: global [[globname]] */
/*s: global [[globv]] */
struct Word *globv;
/*e: global [[globv]] */
/*s: function [[deglob]] */
/*
 * delete all the GLOB marks from s, in place
 */
void
deglob(void *as)
{
    char *s = as;
    char *t = s;
    do{
        if(*t==GLOB)
            t++;
        *s++=*t;
    }while(*t++);
}
/*e: function [[deglob]] */

/*s: function [[globcmp]] */
int
globcmp(const void *s, const void *t)
{
    return strcmp(*(char**)s, *(char**)t);
}
/*e: function [[globcmp]] */

/*s: function [[globsort]] */
void
globsort(word *left, word *right)
{
    char **list;
    word *a;
    int n = 0;
    for(a = left;a!=right;a = a->next) n++;
    list = (char **)emalloc(n*sizeof(char *));
    for(a = left,n = 0;a!=right;a = a->next,n++) list[n] = a->word;
    qsort((void *)list, n, sizeof(void *), globcmp);
    for(a = left,n = 0;a!=right;a = a->next,n++) a->word = list[n];
    efree((char *)list);
}
/*e: function [[globsort]] */
/*s: function [[globdir]] */
/*
 * Push names prefixed by globname and suffixed by a match of p onto the astack.
 * namep points to the end of the prefix in globname.
 */
void
globdir(uchar *p, uchar *namep)
{
    uchar *t, *newp;
    int f;
    /* scan the pattern looking for a component with a metacharacter in it */
    if(*p=='\0'){
        globv = newword(globname, globv);
        return;
    }
    t = namep;
    newp = p;
    while(*newp){
        if(*newp==GLOB)
            break;
        *t=*newp++;
        if(*t++=='/'){
            namep = t;
            p = newp;
        }
    }
    /* If we ran out of pattern, append the name if accessible */
    if(*newp=='\0'){
        *t='\0';
        if(access(globname, 0)==0)
            globv = newword(globname, globv);
        return;
    }
    /* read the directory and recur for any entry that matches */
    *namep='\0';
    if((f = Opendir(globname[0]?globname:"."))<0) return;
    while(*newp!='/' && *newp!='\0') newp++;
    while(Readdir(f, namep, *newp=='/')){
        if(matchfn(namep, p)){
            for(t = namep;*t;t++);
            globdir(newp, t);
        }
    }
    Closedir(f);
}
/*e: function [[globdir]] */
/*s: function [[glob]] */
/*
 * Push all file names matched by p on the current thread's stack.
 * If there are no matches, the list consists of p.
 */
void
glob(void *ap)
{
    uchar *p = ap;
    word *svglobv = globv;
    int globlen = Globsize(ap);

    if(!globlen){
        deglob(p);
        globv = newword((char *)p, globv);
        return;
    }
    globname = emalloc(globlen);
    globname[0]='\0';
    globdir(p, (uchar *)globname);
    efree(globname);
    if(svglobv==globv){
        deglob(p);
        globv = newword((char *)p, globv);
    }
    else
        globsort(globv, svglobv);
}
/*e: function [[glob]] */

/*s: function [[equtf]] */
/*
 * Do p and q point at equal utf codes
 */
bool
equtf(uchar *p, uchar *q)
{
    Rune pr, qr;
    if(*p!=*q)
        return false;
    
    chartorune(&pr, (char*)p);
    chartorune(&qr, (char*)q);
    return pr == qr;
}
/*e: function [[equtf]] */

/*s: function [[nextutf]] */
/*
 * Return a pointer to the next utf code in the string,
 * not jumping past nuls in broken utf codes!
 */
uchar*
nextutf(uchar *p)
{
    Rune dummy;
    return p + chartorune(&dummy, (char*)p);
}
/*e: function [[nextutf]] */

/*s: function [[unicode]] */
/*
 * Convert the utf code at *p to a unicode value
 */
int
unicode(uchar *p)
{
    Rune r;

    chartorune(&r, (char*)p);
    return r;
}
/*e: function [[unicode]] */

/*s: function [[matchfn]] */
/*
 * Does the string s match the pattern p
 * . and .. are only matched by patterns starting with .
 * * matches any sequence of characters
 * ? matches any single character
 * [...] matches the enclosed list of characters
 */
bool
matchfn(void *as, void *ap)
{
    uchar *s = as, *p = ap;

    if(s[0]=='.' && (s[1]=='\0' || s[1]=='.' && s[2]=='\0') && p[0]!='.')
        return false;
    return match(s, p, '/');
}
/*e: function [[matchfn]] */

/*s: function [[match]] */
bool
match(void *as, void *ap, int stop)
{
    int compl, hit, lo, hi, t, c;
    uchar *s = as, *p = ap;

    for(; *p!=stop && *p!='\0'; s = nextutf(s), p = nextutf(p)){
        if(*p!=GLOB){
            if(!equtf(p, s)) return false;
        }
        else switch(*++p){
        case GLOB:
            if(*s!=GLOB)
                return false;
            break;
        case '*':
            for(;;){
                if(match(s, nextutf(p), stop)) return 1;
                if(!*s)
                    break;
                s = nextutf(s);
            }
            return false;
        case '?':
            if(*s=='\0')
                return false;
            break;
        case '[':
            if(*s=='\0')
                return false;
            c = unicode(s);
            p++;
            compl=*p=='~';
            if(compl)
                p++;
            hit = 0;
            while(*p!=']'){
                if(*p=='\0')
                    return false;		/* syntax error */
                lo = unicode(p);
                p = nextutf(p);
                if(*p!='-')
                    hi = lo;
                else{
                    p++;
                    if(*p=='\0')
                        return false;	/* syntax error */
                    hi = unicode(p);
                    p = nextutf(p);
                    if(hi<lo){ t = lo; lo = hi; hi = t; }
                }
                if(lo<=c && c<=hi)
                    hit = 1;
            }
            if(compl)
                hit=!hit;
            if(!hit)
                return false;
            break;
        }
    }
    return *s=='\0';
}
/*e: function [[match]] */

/*s: function [[globlist1]] */
void
globlist1(word *gl)
{
    if(gl){
        globlist1(gl->next);
        glob(gl->word);
    }
}
/*e: function [[globlist1]] */

/*s: function [[globlist]] */
void
globlist(void)
{
    word *a;
    globv = nil;

    globlist1(runq->argv->words);
    poplist();
    pushlist();

    if(globv){
        for(a = globv;a->next;a = a->next)
            ;
        a->next = runq->argv->words;
        runq->argv->words = globv;
    }
}
/*e: function [[globlist]] */
/*e: rc/glob.c */
