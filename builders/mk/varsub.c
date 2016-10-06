/*s: mk/varsub.c */
#include	"mk.h"

static	Word		*subsub(Word*, char*, char*);
static	Word		*expandvar(char**);
static	Bufblock	*varname(char**);
static	Word		*extractpat(char*, char**, char*, char*);
static	int		submatch(char*, Word*, Word*, int*, char**);
static	Word		*varmatch(char *);

/*s: function varsub */
Word*
varsub(char **s)
{
    Bufblock *b;
    Word *w;

    /*s: [[varsub()]] if variable starts with open brace */
    if(**s == '{')		/* either ${name} or ${name: A%B==C%D}*/
        return expandvar(s);
    /*e: [[varsub()]] if variable starts with open brace */
    // else

    b = varname(s);
    if(b == nil)
        return nil;

    w = varmatch(b->start);

    freebuf(b);
    return w;
}
/*e: function varsub */

/*s: function varname */
/*
 *	extract a variable name
 */
static Bufblock*
varname(char **s)
{
    Bufblock *b;
    char *cp;
    Rune r;
    int n;

    b = newbuf();
    cp = *s;
    for(;;){
        n = chartorune(&r, cp);
        if (!WORDCHR(r))
            break;
        rinsert(b, r);
        cp += n;
    }
    /*s: [[varname()]] sanity check b */
    if (b->current == b->start){
        SYNERR(-1);
        fprint(STDERR, "missing variable name <%s>\n", *s);
        freebuf(b);
        return nil;
    }
    /*e: [[varname()]] sanity check b */
    *s = cp;
    insert(b, '\0');
    return b;
}
/*e: function varname */

/*s: function varmatch */
static Word*
varmatch(char *name)
{
    Word *w;
    Symtab *sym;
    
    sym = symlook(name, S_VAR, nil);
    if(sym){
            /* check for at least one non-NULL value */
        for (w = sym->u.ptr; w; w = w->next)
            if(w->s && *w->s)
                return wdup(w);
    }
    return nil;
}
/*e: function varmatch */

/*s: function expandvar */
static Word*
expandvar(char **s)
{
    Word *w;
    Bufblock *buf;
    Symtab *sym;
    char *cp, *begin, *end;

    begin = *s;
    (*s)++;						/* skip the '{' */
    buf = varname(s);
    if (buf == nil)
        return nil;
    cp = *s;
    if (*cp == '}') {				/* ${name} variant*/ //$
        (*s)++;					/* skip the '}' */
        w = varmatch(buf->start);
        freebuf(buf);
        return w;
    }


    if (*cp != ':') {
        SYNERR(-1);
        fprint(STDERR, "bad variable name <%s>\n", buf->start);
        freebuf(buf);
        return nil;
    }
    cp++;
    end = charin(cp , "}");
    if(end == nil){
        SYNERR(-1);
        fprint(STDERR, "missing '}': %s\n", begin);
        Exit();
    }
    *end = '\0';
    *s = end+1;
    
    sym = symlook(buf->start, S_VAR, 0);
    if(sym == nil || sym->u.value == 0)
        w = newword(buf->start);
    else
        w = subsub(sym->u.ptr, cp, end);
    freebuf(buf);
    return w;
}
/*e: function expandvar */

/*s: function extractpat */
static Word*
extractpat(char *s, char **r, char *term, char *end)
{
    int save;
    char *cp;
    Word *w;

    cp = charin(s, term);
    if(cp){
        *r = cp;
        if(cp == s)
            return nil;
        save = *cp;
        *cp = '\0';
        w = stow(s);
        *cp = save;
    } else {
        *r = end;
        w = stow(s);
    }
    return w;
}
/*e: function extractpat */

/*s: function subsub */
static Word*
subsub(Word *v, char *s, char *end)
{
    int nmid;
    Word *head, *tail, *w, *h;
    Word *a, *b, *c, *d;
    Bufblock *buf;
    char *cp, *enda;

    a = extractpat(s, &cp, "=%&", end);
    b = c = d = nil;
    if(PERCENT(*cp))
        b = extractpat(cp+1, &cp, "=", end);
    if(*cp == '=')
        c = extractpat(cp+1, &cp, "&%", end);
    if(PERCENT(*cp))
        d = stow(cp+1);
    else if(*cp)
        d = stow(cp);

    head = tail = nil;
    buf = newbuf();
    for(; v; v = v->next){
        h = w = 0;
        if(submatch(v->s, a, b, &nmid, &enda)){
            /* enda points to end of A match in source;
             * nmid = number of chars between end of A and start of B
             */
            if(c){
                h = w = wdup(c);
                while(w->next)
                    w = w->next;
            }
            if(PERCENT(*cp) && nmid > 0){	
                if(w){
                    bufcpy(buf, w->s, strlen(w->s));
                    bufcpy(buf, enda, nmid);
                    insert(buf, '\0');
                    free(w->s);
                    w->s = strdup(buf->start);
                } else {
                    bufcpy(buf, enda, nmid);
                    insert(buf, '\0');
                    h = w = newword(buf->start);
                }
                buf->current = buf->start;
            }
            if(d && *d->s){
                if(w){

                    bufcpy(buf, w->s, strlen(w->s));
                    bufcpy(buf, d->s, strlen(d->s));
                    insert(buf, '\0');
                    free(w->s);
                    w->s = strdup(buf->start);
                    w->next = wdup(d->next);
                    while(w->next)
                        w = w->next;
                    buf->current = buf->start;
                } else
                    h = w = wdup(d);
            }
        }
        if(w == 0)
            h = w = newword(v->s);
    
        if(head == 0)
            head = h;
        else
            tail->next = h;
        tail = w;
    }
    freebuf(buf);
    freewords(a);
    freewords(b);
    freewords(c);
    freewords(d);
    return head;
}
/*e: function subsub */

/*s: function submatch */
static bool
submatch(char *s, Word *a, Word *b, int *nmid, char **enda)
{
    Word *w;
    int n;
    char *end;

    n = 0;
    for(w = a; w; w = w->next){
        n = strlen(w->s);
        if(strncmp(s, w->s, n) == 0)
            break;
    }
    if(a && w == nil)		/*  a == NULL matches everything*/
        return false;

    *enda = s+n;		/* pointer to end a A part match */
    *nmid = strlen(s)-n;	/* size of remainder of source */
    end = *enda+*nmid;
    for(w = b; w; w = w->next){
        n = strlen(w->s);
        if(strcmp(w->s, end-n) == 0){
            *nmid -= n;
            break;
        }
    }
    if(b && w == nil)		/* b == NULL matches everything */
        return false;
    return true;
}
/*e: function submatch */


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
            if(cp == nil){
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

        // concat_list(new, head, w)
        if (w)
            w->next = new;
        else
            head = w = new;
        while(w->next)
            w = w->next;
        
    }
    if (!head)
        head = newword("");
    return head;
}
/*e: function stow */

/*e: mk/varsub.c */
