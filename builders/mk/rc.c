/*s: mk/rc.c */
#include	"mk.h"

/*s: global termchars */
char	*termchars = "'= \t";	/*used in parse.c to isolate assignment attribute*/
/*e: global termchars */
/*s: global shflags */
char	*shflags = "-I";	/* rc flag to force non-interactive mode */
/*e: global shflags */

/*
 *	This file contains functions that depend on rc's syntax.  Most
 *	of the routines extract strings observing rc's escape conventions
 */

/*s: function squote */
/*
 *	skip a token in single quotes.
 */
static char *
squote(char *cp)
{
    Rune r;
    int n;

    while(*cp){
        n = chartorune(&r, cp);
        if(r == '\'') {
            n += chartorune(&r, cp+n);
            if(r != '\'')
                return cp;
        }
        cp += n;
    }
    SYNERR(-1);		/* should never occur */
    fprint(STDERR, "missing closing '\n");
    return 0;
}
/*e: function squote */

/*s: function charin */
/*
 *	search a string for characters in a pattern set
 *	characters in quotes and variable generators are escaped
 */
char*
charin(char *cp, char *pat)
{
    Rune r;
    int n;
    bool vargen = false;

    while(*cp){
        n = chartorune(&r, cp);
        switch(r){
        /*s: [[charin()]] switch rune cases */
        case '\'':			/* skip quoted string */
            cp = squote(cp+1);	/* n must = 1 */
            if(!cp)
                return nil;
            break;
        /*x: [[charin()]] switch rune cases */
        case '$':
            if(*(cp+1) == '{')
                vargen = true;
            break;
        case '}':
            if(vargen)
                vargen = false;
            else if(utfrune(pat, r))
                return cp;
            break;
        /*e: [[charin()]] switch rune cases */
        default:
            if(!vargen && utfrune(pat, r))
                return cp;
            break;
        }
        cp += n;
    }
    /*s: [[charin()]] sanity check vargen */
    if(vargen){
        SYNERR(-1);
        fprint(STDERR, "missing closing } in pattern generator\n");
    }
    /*e: [[charin()]] sanity check vargen */
    return nil;
}
/*e: function charin */

/*s: function expandquote */
/*
 *	extract an escaped token.  Possible escape chars are single-quote,
 *	double-quote,and backslash.  Only the first is valid for rc. the
 *	others are just inserted into the receiving buffer.
 */
char*
expandquote(char *s, Rune r, Bufblock *b)
{
    if (r != '\'') {
        rinsert(b, r);
        return s;
    }

    while(*s){
        s += chartorune(&r, s);
        if(r == '\'') {
            if(*s == '\'')
                s++;
            else
                return s;
        }
        rinsert(b, r);
    }
    return 0;
}
/*e: function expandquote */

/*s: function escapetoken */
/*
 *	Input an escaped token.  Possible escape chars are single-quote,
 *	double-quote and backslash.  Only the first is a valid escape for
 *	rc; the others are just inserted into the receiving buffer.
 */
bool
escapetoken(Biobuf *bp, Bufblock *buf, bool preserve, int esc)
{
    int c;
    int line;

    if(esc != '\'')
        return true;

    line = mkinline;
    while((c = nextrune(bp, 0)) > 0){
        if(c == '\''){
            if(preserve)
                rinsert(buf, c);
            c = Bgetrune(bp);
            if (c < 0)
                break;
            if(c != '\''){
                Bungetrune(bp);
                return true;
            }
        }
        rinsert(buf, c);
    }
    SYNERR(line); 
    fprint(STDERR, "missing closing %c\n", esc);
    return false;
}
/*e: function escapetoken */

/*s: function copysingle */
/*
 *	copy a single-quoted string; s points to char after opening quote
 */
static char *
copysingle(char *s, Bufblock *buf)
{
    Rune r;

    while(*s){
        s += chartorune(&r, s);
        rinsert(buf, r);
        if(r == '\'')
            break;
    }
    return s;
}
/*e: function copysingle */

/*s: function copyq */
/*
 *	check for quoted strings.  backquotes are handled here; single quotes above.
 *	s points to char after opening quote, q.
 */
char *
copyq(char *s, Rune q, Bufblock *buf)
{
    if(q == '\'')				/* copy quoted string */
        return copysingle(s, buf);

    if(q != '`')				/* not quoted */
        return s;
    // else

    while(*s){				/* copy backquoted string */
        s += chartorune(&q, s);
        rinsert(buf, q);
        if(q == '}')
            break;
        if(q == '\'')
            s = copysingle(s, buf);	/* copy quoted string */
    }
    return s;
}
/*e: function copyq */
/*e: mk/rc.c */
