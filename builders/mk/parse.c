/*s: mk/parse.c */
#include        "mk.h"

void    ipop(void);
void    ipush(void);
static int   rhead(char *, Word **, Word **, int *, char **);
static char* rbody(Biobuf*);

/*s: function [[parse]] */
void
parse(char *f, fdt fd, bool varoverride)
{
    Biobuf in;
    Bufblock *buf;
    char c; // one of : = <
    Word *head, *tail;
    int hline; // head line number
    /*s: [[parse()]] other locals */
    char *body;
    /*x: [[parse()]] other locals */
    // bitset<Rule_attr>
    int attr;
    /*x: [[parse()]] other locals */
    char *p;
    fdt newfd;
    /*x: [[parse()]] other locals */
    bool set = true;
    /*x: [[parse()]] other locals */
    int pid;
    /*x: [[parse()]] other locals */
    char *prog;
    /*e: [[parse()]] other locals */

    /*s: [[parse()]] sanity check fd */
    if(fd < 0){
        perror(f);
        Exit();
    }
    /*e: [[parse()]] sanity check fd */
    /*s: [[parse()]] start, push */
    ipush();
    /*e: [[parse()]] start, push */

    // Initialization
    infile = strdup(f);
    mkinline = 1;
    Binit(&in, fd, OREAD);
    buf = newbuf();

    // Lexing
    while(assline(&in, buf)){
        hline = mkinline;

        // Parsing
        c = rhead(buf->start, &head, &tail,     &attr, &prog);

        // Semantic actions (they may read more lines)
        switch(c)
        {
        /*s: [[parse()]] switch rhead cases */
        default:
            SYNERR(hline);
            fprint(STDERR, "expected one of :<=\n");
            Exit();
            break;
        /*x: [[parse()]] switch rhead cases */
        case ':':
            body = rbody(&in);
            addrules(head, tail, body,   attr, hline, prog);
            break;
        /*x: [[parse()]] switch rhead cases */
        case '<':
            p = wtos(tail, ' ');
            /*s: [[parse()]] when parsing included file, sanity check p */
            if(*p == '\0'){
                SYNERR(-1);
                fprint(STDERR, "missing include file name\n");
                Exit();
            }
            /*e: [[parse()]] when parsing included file, sanity check p */
            newfd = open(p, OREAD);
            /*s: [[parse()]] when parsing included file, sanity check newfd */
            if(newfd < 0){
                fprint(STDERR, "warning: skipping missing include file: ");
                perror(p);
            }
            /*e: [[parse()]] when parsing included file, sanity check newfd */
            else
                // recurse
                parse(p, newfd, false);
            break;
        /*x: [[parse()]] switch rhead cases */
        case '=':
            /*s: [[parse()]] when parsing variable definitions, sanity check head */
            if(head->next){
                SYNERR(-1);
                fprint(STDERR, "multiple vars on left side of assignment\n");
                Exit();
            }
            /*e: [[parse()]] when parsing variable definitions, sanity check head */
            /*s: [[parse()]] when parsing variable definitions, override handling */
            if(symlook(head->s, S_OVERRIDE, nil)){
                set = varoverride;
            } else {
                set = true;
                if(varoverride)
                    symlook(head->s, S_OVERRIDE, (void *)"");
            }
            /*e: [[parse()]] when parsing variable definitions, override handling */
            if(set){
                setvar(head->s, (void *) tail);
                /*s: [[parse()]] when parsing variable definitions, extra setting */
                symlook(head->s, S_WESET, (void *)"");
                /*e: [[parse()]] when parsing variable definitions, extra setting */
            }
            /*s: [[parse()]] when parsing variable definitions, if variable with attr */
            if(attr)
                symlook(head->s, S_NOEXPORT, (void *)"");
            /*e: [[parse()]] when parsing variable definitions, if variable with attr */
            break;
        /*x: [[parse()]] switch rhead cases */
        case '|':
            p = wtos(tail, ' ');
            /*s: [[parse()]] sanity check [[p]] for include program name */
            if(*p == '\0'){
                SYNERR(-1);
                fprint(STDERR, "missing include program name\n");
                Exit();
            }
            /*e: [[parse()]] sanity check [[p]] for include program name */

            initenv();
            pid = pipecmd(p, shellenv, &newfd);
            /*s: [[parse()]] sanity check [[newfd]] */
            if(newfd < 0){
                fprint(STDERR, "warning: skipping missing program file: ");
                perror(p);
            } 
            /*e: [[parse()]] sanity check [[newfd]] */
            else
                // recursive call
                parse(p, newfd, 0);

            while(waitup(EMPTY_CHILDREN_IS_ERROR3, &pid) >= 0)
                ;
            /*s: [[parse()]] sanity check [[pid]] after waitup */
            if(pid != 0){
                fprint(STDERR, "bad include program status\n");
                Exit();
            }
            /*e: [[parse()]] sanity check [[pid]] after waitup */
            break;
        /*e: [[parse()]] switch rhead cases */
        }
    }
    close(fd);
    freebuf(buf);
    /*s: [[parse()]] end, pop */
    ipop();
    /*e: [[parse()]] end, pop */
}
/*e: function [[parse]] */

/*s: function [[addrules]] */
void
addrules(Word *targets, Word *prereqs, char *recipe, 
         int attr, int hline, char *prog)
{
    Word *w;

    assert(/*addrules args*/ targets && recipe);

    /*s: [[addrules()]] set [[target1]] */
    /* tuck away first non-meta rule as default target*/
    if(target1 == nil && !(attr&REGEXP)){
        for(w = targets; w; w = w->next)
            if(charin(w->s, "%&"))
                break;
        if(w == nil) // head does not contain any pattern
            target1 = wdup(targets);
    }
    /*e: [[addrules()]] set [[target1]] */
    for(w = targets; w; w = w->next)
        addrule(w->s, prereqs, recipe, targets, attr, hline, prog);
}
/*e: function [[addrules]] */

/*s: function [[rhead]] */
static int
rhead(char *line, Word **h, Word **t,    int *attr, char **prog)
{
    char *p;
    int sep; // one of : = < 
    /*s: [[rhead()]] other locals */
    Rune r;
    int n;
    /*x: [[rhead()]] other locals */
    char *pp;
    /*e: [[rhead()]] other locals */

    p = charin(line, ":=<");
    if(p == nil)
        return '?';

    sep = *p;
    *p++ = '\0';
    /*s: [[rhead()]] adjust sep if dynamic mkfile [[<|]] */
    if(sep == '<' && *p == '|'){
        sep = '|';
        p++;
    }
    /*e: [[rhead()]] adjust sep if dynamic mkfile [[<|]] */
    /*s: [[rhead()]] adjust [[attr]] and [[prog]] */
    *attr = 0; // Nothing
    *prog = nil;
    // variable attributes
    /*s: [[rhead()]] if sep is [[=]] */
    if(sep == '='){
        pp = charin(p, termchars);      /* termchars is shell-dependent */
        if (pp && *pp == '=') {
            while (p != pp) {
                n = chartorune(&r, p);
                switch(r)
                {
                /*s: [[rhead()]] when parsing variable attributes, switch rune cases */
                case 'U':
                    *attr = 1;
                    break;
                /*e: [[rhead()]] when parsing variable attributes, switch rune cases */
                default:
                    SYNERR(-1);
                    fprint(STDERR, "unknown attribute '%c'\n",*p);
                    Exit();
                }
                p += n;
            }
            p++;                /* skip trailing '=' */
        }
    }
    /*e: [[rhead()]] if sep is [[=]] */
    // rule attributes
    /*s: [[rhead()]] if sep is [[:]] */
    if((sep == ':') && *p && (*p != ' ') && (*p != '\t')){
        while (*p) {
            n = chartorune(&r, p);
            if (r == ':')
                break;
            p += n;
            switch(r)
            {
            /*s: [[rhead()]] when parsing rule attributes, switch rune cases */
            case 'R':
                *attr |= REGEXP;
                break;
            /*x: [[rhead()]] when parsing rule attributes, switch rune cases */
            case 'V':
                *attr |= VIR;
                break;
            /*x: [[rhead()]] when parsing rule attributes, switch rune cases */
            case 'D':
                *attr |= DEL;
                break;
            /*x: [[rhead()]] when parsing rule attributes, switch rune cases */
            case 'Q':
                *attr |= QUIET;
                break;
            /*x: [[rhead()]] when parsing rule attributes, switch rune cases */
            case 'E':
                *attr |= NOMINUSE;
                break;
            /*x: [[rhead()]] when parsing rule attributes, switch rune cases */
            case 'N':
                *attr |= NOREC;
                break;
            /*x: [[rhead()]] when parsing rule attributes, switch rune cases */
            case 'n':
                *attr |= NOVIRT;
                break;
            /*x: [[rhead()]] when parsing rule attributes, switch rune cases */
            case 'P':
                pp = utfrune(p, ':');
                if (pp == nil || *pp == 0)
                    goto eos;
                *pp = 0;
                *prog = strdup(p);
                *pp = ':';
                p = pp;
                break;
            /*e: [[rhead()]] when parsing rule attributes, switch rune cases */
            //PAD: this is an extension in mk-in-ocaml that I ignore here
            case 'I':
                break;
            default:
                SYNERR(-1);
                fprint(STDERR, "unknown attribute '%c'\n", p[-1]);
                Exit();
            }
        }
        if (*p++ != ':') {
    eos:
            SYNERR(-1);
            fprint(STDERR, "missing trailing :\n");
            Exit();
        }
    }
    /*e: [[rhead()]] if sep is [[:]] */
    /*e: [[rhead()]] adjust [[attr]] and [[prog]] */

    // potentially expand variables in head
    *h = stow(line);
    /*s: [[rhead()]] sanity check h */
    if(empty_words(*h) && sep != '<' && sep != '|') {
        SYNERR(mkinline-1);
        fprint(STDERR, 
               "no var (or target) on left side of assignment (or rule)\n");
        Exit();
    }
    /*e: [[rhead()]] sanity check h */
    // potentially expand variables in tail
    *t = stow(p);

    return sep;
}
/*e: function [[rhead]] */

/*s: function [[rbody]] */
static char *
rbody(Biobuf *in)
{
    Bufblock *buf;
    int r, lastr;
    char *p;

    lastr = '\n';
    buf = newbuf();

    for(;;){
        r = Bgetrune(in);
        if (r < 0)
            break; // eof
        // in first column?
        if (lastr == '\n') {
            /*s: [[rbody()]] if comment in first column */
            if (r == '#')
                rinsert(buf, r); // the shell recognize comments too
            /*e: [[rbody()]] if comment in first column */
            else 
              if (r != ' ' && r != '\t') {
                Bungetrune(in);
                break;
            }
        } else
            // not in first column
            rinsert(buf, r);

        lastr = r;
        /*s: [[rbody()]] handle mkinline */
        if (r == '\n')
            mkinline++;
        /*e: [[rbody()]] handle mkinline */
    }

    insert(buf, '\0');
    p = strdup(buf->start);
    freebuf(buf);

    return p;
}
/*e: function [[rbody]] */

/*s: struct [[input]] */
struct Input
{
    char *file;
    int line;

    // Extra
    /*s: [[Input]] extra fields */
    // list<ref_own<Input>> (head = inputs)
    struct Input *next;
    /*e: [[Input]] extra fields */
};
/*e: struct [[input]] */
/*s: global [[inputs]] */
// list<ref_own<Input>> (next = Input.next)
static struct Input *inputs = nil;
/*e: global [[inputs]] */

/*s: function [[ipush]] */
void
ipush(void)
{
    struct Input *in, *me;

    me = (struct Input *)Malloc(sizeof(*me));
    // saving globals
    me->file = infile;
    me->line = mkinline;
    me->next = nil;

    // add_list(me, inputs)
    if(inputs == nil)
        inputs = me;
    else {
        for(in = inputs; in->next; )
            in = in->next;
        in->next = me;
    }
}
/*e: function [[ipush]] */

/*s: function [[ipop]] */
void
ipop(void)
{
    struct Input *in, *me;

    assert(/*pop input list*/ inputs != nil);
    // me = pop_list(inputs)
    if(inputs->next == nil){
        me = inputs;
        inputs = nil;
    } else {
        for(in = inputs; in->next->next; )
            in = in->next;
        me = in->next;
        in->next = nil;
    }
    // restoring globals
    infile = me->file;
    mkinline = me->line;
    free((char *)me);
}
/*e: function [[ipop]] */

/*e: mk/parse.c */
