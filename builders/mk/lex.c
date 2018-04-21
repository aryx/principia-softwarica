/*s: mk/lex.c */
#include	"mk.h"

static	int	bquote(Biobuf*, Bufblock*);

/*s: function [[assline]] */
/*
 *	Assemble a line skipping blank lines, comments, and eliding
 *	escaped newlines
 */
bool
assline(Biobuf *bp, Bufblock *buf)
{
    int c;
    /*s: [[assline()]] other locals */
    int prevc;
    /*e: [[assline()]] other locals */

    resetbuf(buf);
    while ((c = nextrune(bp, true)) >= 0){
        switch(c)
        {
        case '\n':
            if (!isempty(buf)) {
                insert(buf, '\0');
                return true;
            }
            break;		/* skip empty lines */
        /*s: [[assline()]] switch character cases */
        case '#':
            prevc = '#';
            // skip all characters in comment until newline
            while ((c = Bgetc(bp)) != '\n') {
                if (c < 0)
                    goto eof;
                prevc = c;
            }
            mkinline++;
           /*s: [[assline()]] when processing comments, if escaped newline */
           if (prevc == '\\')
               break;		/* propagate escaped newlines??*/
           /*e: [[assline()]] when processing comments, if escaped newline */
           /*s: [[assline()]] when processing comments, if not only comment on the line */
           if (!isempty(buf)) {
                insert(buf, '\0');
                return true;
            }
           /*e: [[assline()]] when processing comments, if not only comment on the line */
            // else, skip lines with only a comment
            break;
        /*x: [[assline()]] switch character cases */
        case '\'':
        case '"':
        case '\\':
            rinsert(buf, c);
            if (escapetoken(bp, buf, true, c) == ERROR_0)
                Exit();
            break;
        /*x: [[assline()]] switch character cases */
        case '`':
            if (bquote(bp, buf) == ERROR_0)
                Exit();
            break;
        /*e: [[assline()]] switch character cases */
        default:
            rinsert(buf, c);
            break;
        }
    }
eof:
    insert(buf, '\0');
    return *(bufcontent(buf)) != '\0';
}
/*e: function [[assline]] */

/*s: function [[bquote]] */
/*
 *	assemble a back-quoted shell command into a buffer
 */
static error0
bquote(Biobuf *bp, Bufblock *buf)
{
    int line;
    int c, term;
    int start;

    line = mkinline;
    /*s: [[bquote()]] skip spaces */
    while((c = Bgetrune(bp)) == ' ' || c == '\t')
            ;
    /*e: [[bquote()]] skip spaces */
    if(c == '{'){
        term = '}';		/* rc style */
        /*s: [[bquote()]] skip spaces */
        while((c = Bgetrune(bp)) == ' ' || c == '\t')
                ;
        /*e: [[bquote()]] skip spaces */
    } else
        term = '`';		/* sh style */

    start = buf->current - buf->start;
    for(;c > 0; c = nextrune(bp, false)){
        if(c == term){
            insert(buf, '\n');
            insert(buf, '\0');
            // prepare to overwrite the command string with its output
            buf->current = buf->start + start;

            /*s: [[bquote()]] execute shell command in [[buf]] */
            initenv();
            // running the command, passing a buf argument
            execsh(nil, buf->current, buf, shellenv);
            /*e: [[bquote()]] execute shell command in [[buf]] */
            return OK_1;
        }
        if(c == '\n')
            break; // go to error
        /*s: [[bquote()]] handle quotes and escape characters */
        if(c == '\'' || c == '"' || c == '\\'){
            insert(buf, c);
            if(!escapetoken(bp, buf, true, c))
                return ERROR_0;
            continue;
        }
        /*e: [[bquote()]] handle quotes and escape characters */
        rinsert(buf, c);
    }
    SYNERR(line);
    fprint(STDERR, "missing closing %c after `\n", term);
    return ERROR_0;
}
/*e: function [[bquote]] */

/*s: function [[nextrune]] */
/*
 *	get next character stripping escaped newlines
 *	the flag specifies whether escaped newlines are to be elided or
 *	replaced with a blank.
 */
int
nextrune(Biobuf *bp, bool elide)
{
    int c;

    for (;;) {
        c = Bgetrune(bp);
        /*s: [[nextrune()]] if escape character */
        if (c == '\\') {
            if (Bgetrune(bp) == '\n') {
                // an escaped newline!
                mkinline++;
                if (elide)
                    continue;
                // else
                return ' ';
            }
            // else, it was just \
            Bungetrune(bp);
        }
        /*e: [[nextrune()]] if escape character */
        /*s: [[nextrune()]] handle mkinline */
        if (c == '\n')
            mkinline++;
        /*e: [[nextrune()]] handle mkinline */
        return c;
    }
}
/*e: function [[nextrune]] */
/*e: mk/lex.c */
