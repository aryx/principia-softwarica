/*s: mk/varsub.c */
#include	"mk.h"

static	Word		*subsub(Word*, char*, char*);
static	Word		*expandvar(char**);
static	Bufblock	*varname(char**);
static	Word		*extractpat(char*, char**, char*, char*);
static	bool		submatch(char*, Word*, Word*, int*, char**);
static	Word		*varmatch(char *);

/*s: function [[varsub]] */
Word*
varsub(char **s)
{
    Bufblock *buf;
    Word *w;

    /*s: [[varsub()]] if variable starts with open brace */
    if(**s == '{')		/* either ${name} or ${name: A%B==C%D}*/
        return expandvar(s);
    /*e: [[varsub()]] if variable starts with open brace */
    // else

    buf = varname(s);
    /*s: [[varsub()]] sanity check buf */
    if(buf == nil)
        return nil;
    /*e: [[varsub()]] sanity check buf */
    w = varmatch(buf->start);

    freebuf(buf);
    return w;
}
/*e: function [[varsub]] */

/*s: function [[varname]] */
/*
 *	extract a variable name
 */
static Bufblock*
varname(char **s)
{
    Bufblock *buf;
    char *cp = *s;
    Rune r;
    int n;

    buf = newbuf();
    for(;;){
        n = chartorune(&r, cp);
        if (!WORDCHR(r))
            break;
        rinsert(buf, r);
        cp += n;
    }
    /*s: [[varname()]] sanity check buf */
    if (isempty(buf)){
        SYNERR(-1);
        fprint(STDERR, "missing variable name <%s>\n", *s);
        freebuf(buf);
        return nil;
    }
    /*e: [[varname()]] sanity check buf */
    *s = cp;
    insert(buf, '\0');
    return buf;
}
/*e: function [[varname]] */

/*s: function [[varmatch]] */
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
/*e: function [[varmatch]] */

/*s: function [[expandvar]] */
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
    
    sym = symlook(buf->start, S_VAR, nil);
    if(sym == nil || sym->u.value == 0)
        w = newword(buf->start);
    else
        w = subsub(sym->u.ptr, cp, end);
    freebuf(buf);
    return w;
}
/*e: function [[expandvar]] */

/*s: function [[extractpat]] */
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
/*e: function [[extractpat]] */

/*s: function [[subsub]] */
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
        h = w = nil;
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
        if(w == nil)
            h = w = newword(v->s);
    
        if(head == nil)
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
/*e: function [[subsub]] */

/*s: function [[submatch]] */
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
/*e: function [[submatch]] */


/*s: function [[nextword]] */
/*
 *	break out a word from a string handling quotes, executions,
 *	and variable expansions.
 */
static Word*
nextword(char **s)
{
    char *cp = *s;
    Bufblock *buf;
    Rune r;
    // list<ref_own<Word>>
    Word *head;
    // option<ref<Word>>
    Word  *lastw;
    bool empty;
    /*s: [[nextword()]] other locals */
    // list<ref_own<Word>
    Word *w;
    /*e: [[nextword()]] other locals */

    buf = newbuf();

restart:
    head = lastw = nil;
    empty = true;
    /*s: [[nextword()]] skipping leading white space */
    while(*cp == ' ' || *cp == '\t')		/* leading white space */
        cp++;
    /*e: [[nextword()]] skipping leading white space */

    while(*cp){
        cp += chartorune(&r, cp);
        switch(r)
        {
        case ' ':
        case '\t':
        case '\n':
            goto out;
        /*s: [[nextword()]] switch rune cases */
        case '\'':
        case '"':
        case '\\':
            empty = false;
            cp = expandquote(cp, r, buf);
            if(cp == nil){
                fprint(STDERR, "missing closing quote: %s\n", *s);
                Exit();
            }
            break;
        /*x: [[nextword()]] switch rune cases */
        case '$':
            w = varsub(&cp);
            /*s: [[nextword()]] when in variable case, if w is nil */
            if(w == nil){
               /*s: [[nextword()]] when in variable case, if w is nil and no char before */
               if(empty)
                   goto restart;
               /*e: [[nextword()]] when in variable case, if w is nil and no char before */
                break;
            }
            /*e: [[nextword()]] when in variable case, if w is nil */
            empty = false;

            /*s: [[nextword()]] when in variable case, if non-space chars before var */
            if(!isempty(buf)){
                bufcpy(buf, w->s, strlen(w->s));
                insert(buf, '\0');
                free(w->s);
                // adjust the first word
                w->s = strdup(buf->start);
    
                resetbuf(buf);
            }
            /*e: [[nextword()]] when in variable case, if non-space chars before var */
            /*s: [[nextword()]] when in variable case, if head is not empty */
            if(head){
                // merge the last and first words
                bufcpy(buf, lastw->s, strlen(lastw->s));
                bufcpy(buf, w->s, strlen(w->s));
                insert(buf, '\0');
                free(lastw->s);
                lastw->s = strdup(buf->start);

                lastw->next = w->next;
                free(w->s);
                free(w);
                resetbuf(buf);
            }
            /*e: [[nextword()]] when in variable case, if head is not empty */
            else
                head = lastw = w;

            while(lastw->next)
                lastw = lastw->next;
            break;
        /*e: [[nextword()]] switch rune cases */
        default:
            empty = false;
            rinsert(buf, r);
            break;
        }
    }
out:
    *s = cp;
    if(!isempty(buf)){
        /*s: [[nextword()]] when buffer not empty, if there was already an head */
        if(head){
            cp = buf->current;
            bufcpy(buf, lastw->s, strlen(lastw->s));
            bufcpy(buf, buf->start, cp - buf->start);
            insert(buf, '\0');
            free(lastw->s);
            // adjust the last word
            lastw->s = strdup(cp);
        }
        /*e: [[nextword()]] when buffer not empty, if there was already an head */
         else {
            insert(buf, '\0');
            head = newword(buf->start);
        }
    }
    freebuf(buf);
    return head;
}
/*e: function [[nextword]] */

/*s: function [[stow]] */
Word *
stow(char *s)
{
    // list<ref_own<Word>>
    Word *head, *new;
    // option<ref<Word>>
    Word *lastw;

    head = lastw = nil;
    while(*s){
        new = nextword(&s);
        if(new == nil)
            break;

        // head = concat_list(head, new)
        if (lastw)
            lastw->next = new;
        else
            head = lastw = new;

        while(lastw->next)
            lastw = lastw->next;
        
    }
    /*s: [[stow()]] if head still nil */
    if (!head)
        head = newword("");
    /*e: [[stow()]] if head still nil */
    return head;
}
/*e: function [[stow]] */

/*e: mk/varsub.c */
