/*s: mk/word.c */
#include	"mk.h"

static	Word	*nextword(char**);

/*s: constructor newword */
Word*
newword(char *s)
{
    Word *w;

    w = (Word *)Malloc(sizeof(Word));
    w->s = strdup(s);
    w->next = nil;
    return w;
}
/*e: constructor newword */

/*s: function stow */
Word *
stow(char *s)
{
    Word *head, *w, *new;

    w = head = nil;
    while(*s){
        new = nextword(&s);
        if(new == nil)
            break;
        // add_list(new, head)
        if (w)
            w->next = new;
        else
            head = w = new;
        // concat_list(head, new)
        while(w->next)
            w = w->next;
        
    }
    if (!head)
        head = newword("");
    return head;
}
/*e: function stow */

/*s: function wtos */
char *
wtos(Word *w, int sep)
{
    Bufblock *buf;
    char *cp;

    buf = newbuf();
    for(; w; w = w->next){
        for(cp = w->s; *cp; cp++)
            insert(buf, *cp);
        if(w->next)
            insert(buf, sep);
    }
    insert(buf, '\0');
    cp = strdup(buf->start);
    freebuf(buf);
    return cp;
}
/*e: function wtos */

/*s: function wdup */
Word*
wdup(Word *w)
{
    Word *v, *new, *base;

    v = base = nil;
    while(w){
        new = newword(w->s);
        if(v)
            v->next = new;
        else
            base = new;
        v = new;
        w = w->next;
    }
    return base;
}
/*e: function wdup */

/*s: destructor delword */
void
delword(Word *w)
{
    Word *v;

    while(v = w){
        w = w->next;
        if(v->s)
            free(v->s);
        free(v);
    }
}
/*e: destructor delword */

/*s: function nextword */
/*
 *	break out a word from a string handling quotes, executions,
 *	and variable expansions.
 */
static Word*
nextword(char **s)
{
    char *cp;
    Bufblock *b;
    Word *head, *tail;
    Rune r;
    bool empty;
    /*s: [[nextword()]] other locals */
    Word *w;
    /*e: [[nextword()]] other locals */

    cp = *s;
    b = newbuf();

restart:
    head = tail = nil;
    while(*cp == ' ' || *cp == '\t')		/* leading white space */
        cp++;
    empty = true;

    while(*cp){
        cp += chartorune(&r, cp);
        switch(r)
        {
        case ' ':
        case '\t':
        case '\n':
            goto out;
        /*s: [[nextword()]] switch rune cases */
        case '\\':
        case '\'':
        case '"':
            empty = false;
            cp = expandquote(cp, r, b);
            if(cp == 0){
                fprint(STDERR, "missing closing quote: %s\n", *s);
                Exit();
            }
            break;
        /*x: [[nextword()]] switch rune cases */
        case '$':
            w = varsub(&cp);
            if(w == nil){
                if(empty)
                    goto restart;
                break;
            }
            empty = false;
            if(b->current != b->start){
                bufcpy(b, w->s, strlen(w->s));
                insert(b, '\0');
                free(w->s);
                w->s = strdup(b->start);
                b->current = b->start;
            }
            if(head){
                bufcpy(b, tail->s, strlen(tail->s));
                bufcpy(b, w->s, strlen(w->s));
                insert(b, '\0');
                free(tail->s);
                tail->s = strdup(b->start);
                tail->next = w->next;
                free(w->s);
                free(w);
                b->current = b->start;
            } else
                tail = head = w;

            while(tail->next)
                tail = tail->next;
            break;
        /*e: [[nextword()]] switch rune cases */
        default:
            empty = false;
            rinsert(b, r);
            break;
        }
    }
out:
    *s = cp;
    if(b->current != b->start){
        if(head){
            cp = b->current;
            bufcpy(b, tail->s, strlen(tail->s));
            bufcpy(b, b->start, cp - b->start);
            insert(b, '\0');
            free(tail->s);
            tail->s = strdup(cp);
        } else {
            insert(b, '\0');
            head = newword(b->start);
        }
    }
    freebuf(b);
    return head;
}
/*e: function nextword */

/*s: dumper dumpw */
void
dumpw(char *s, Word *w)
{
    Bprint(&bout, "%s", s);
    for(; w; w = w->next)
        Bprint(&bout, " '%s'", w->s);
    Bputc(&bout, '\n');
}
/*e: dumper dumpw */
/*e: mk/word.c */
