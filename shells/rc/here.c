/*s: rc/here.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */
#include "y.tab.h"

/*s: global [[here]] */
struct Here *here;
/*e: global [[here]] */
/*s: global [[ehere]] */
struct Here **ehere;
/*e: global [[ehere]] */
/*s: global [[ser]] */
int ser = 0;
/*e: global [[ser]] */
/*s: global tmp (rc/here.c) */
char tmp[] = "/tmp/here0000.0000";
/*e: global tmp (rc/here.c) */
/*s: global [[hex]] */
char hex[] = "0123456789abcdef";
/*e: global [[hex]] */

void psubst(io*, uchar*);
void pstrs(io*, word*);

/*s: function [[hexnum]] */
void
hexnum(char *p, int n)
{
    *p++ = hex[(n>>12)&0xF];
    *p++ = hex[(n>>8)&0xF];
    *p++ = hex[(n>>4)&0xF];
    *p = hex[n&0xF];
}
/*e: function [[hexnum]] */

/*s: function [[heredoc]] */
//@Scheck: used by syn.y
tree* heredoc(tree *tag)
{
    struct Here *h = new(struct Here);

    if(tag->type != WORD)
        yyerror("Bad here tag");
    h->next = 0;
    if(here)
        *ehere = h;
    else
        here = h;
    ehere = &h->next;
    h->tag = tag;
    hexnum(&tmp[9], getpid());
    hexnum(&tmp[14], ser++);
    h->name = strdup(tmp);
    return token(tmp, WORD);
}
/*e: function [[heredoc]] */

/*s: constant [[NLINE]] */
/*
 * bug: lines longer than NLINE get split -- this can cause spurious
 * missubstitution, or a misrecognized EOF marker.
 */
#define	NLINE	4096
/*e: constant [[NLINE]] */

/*s: function [[readhere]] */
void
readhere(void)
{
    int c, subst;
    char *s, *tag;
    char line[NLINE+1];
    io *f;
    struct Here *h, *nexth;

    for(h = here; h; h = nexth){
        subst = !h->tag->quoted;
        tag = h->tag->str;
        c = Creat(h->name);
        if(c < 0)
            yyerror("can't create here document");
        f = openfd(c);
        s = line;
        pprompt();
        while((c = rchr(runq->cmdfd)) != EOF){
            if(c == '\n' || s == &line[NLINE]){
                *s = '\0';
                if(tag && strcmp(line, tag) == 0)
                    break;
                if(subst)
                    psubst(f, (uchar *)line);
                else
                    pstr(f, line);
                s = line;
                if(c == '\n'){
                    pprompt();
                    pchr(f, c);
                }else
                    *s++ = c;
            }else
                *s++ = c;
        }
        flush(f);
        closeio(f);
        cleanhere(h->name);
        nexth = h->next;
        efree((char *)h);
    }
    here = nil;
    doprompt = true;
}
/*e: function [[readhere]] */

/*s: function [[psubst]] */
void
psubst(io *f, uchar *s)
{
    int savec, n;
    uchar *t, *u;
    Rune r;
    word *star;

    while(*s){
        if(*s != '$'){		/* copy plain text rune */
            if(*s < Runeself)
                pchr(f, *s++);
            else{
                n = chartorune(&r, (char *)s);
                while(n-- > 0)
                    pchr(f, *s++);
            }
        }else{			/* $something -- perform substitution */
            t = ++s;
            if(*t == '$')
                pchr(f, *t++);
            else{
                while(*t && idchr(*t))
                    t++;
                savec = *t;
                *t = '\0';
                n = 0;
                for(u = s; *u && '0' <= *u && *u <= '9'; u++)
                    n = n*10 + *u - '0';
                if(n && *u == '\0'){
                    star = vlook("*")->val;
                    if(star && 1 <= n && n <= count(star)){
                        while(--n)
                            star = star->next;
                        pstr(f, star->word);
                    }
                }else
                    pstrs(f, vlook((char *)s)->val);
                *t = savec;
                if(savec == '^')
                    t++;
            }
            s = t;
        }
    }
}
/*e: function [[psubst]] */

/*s: function [[pstrs]] */
void
pstrs(io *f, word *a)
{
    if(a){
        while(a->next && a->next->word){
            pstr(f, a->word);
            pchr(f, ' ');
            a = a->next;
        }
        pstr(f, a->word);
    }
}
/*e: function [[pstrs]] */
/*e: rc/here.c */
