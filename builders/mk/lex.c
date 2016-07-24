/*s: mk/lex.c */
#include	"mk.h"

static	int	bquote(Biobuf*, Bufblock*);

/*s: function assline */
/*
 *	Assemble a line skipping blank lines, comments, and eliding
 *	escaped newlines
 */
bool
assline(Biobuf *bp, Bufblock *buf)
{
    int c;
    /*s: [[assline()]] other locals */
    int lastc;
    /*e: [[assline()]] other locals */

    // reset buf
    buf->current = buf->start;

    while ((c = nextrune(bp, true)) >= 0){
        switch(c)
        {
        case '\n':
            if (buf->current != buf->start) {
                insert(buf, '\0');
                return true;
            }
            break;		/* skip empty lines */
        /*s: [[assline()]] switch character cases */
        case '#':
            lastc = '#';
            // skil all characters in comment until newline
            while ((c = Bgetc(bp)) != '\n') {
                if (c < 0)
                    goto eof;
                if(c != '\r')
                    lastc = c;
            }
            mkinline++;
            if (lastc == '\\')
                break;		/* propagate escaped newlines??*/

           // like '\n' case 
           if (buf->current != buf->start) {
                insert(buf, '\0');
                return true;
            }
            // skip lines with only a comment
            break;
        /*x: [[assline()]] switch character cases */
        case '\\':
        case '\'':
        case '"':
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
    return *buf->start != '\0';
}
/*e: function assline */

/*s: function bquote */
/*
 *	assemble a back-quoted shell command into a buffer
 */
static error0
bquote(Biobuf *bp, Bufblock *buf)
{
    int c, line, term;
    int start;

    line = mkinline;
    while((c = Bgetrune(bp)) == ' ' || c == '\t')
            ;
    if(c == '{'){
        term = '}';		/* rc style */
        while((c = Bgetrune(bp)) == ' ' || c == '\t')
            ;
    } else
        term = '`';		/* sh style */

    start = buf->current - buf->start;
    for(;c > 0; c = nextrune(bp, false)){
        if(c == term){
            insert(buf, '\n');
            insert(buf, '\0');
            buf->current = buf->start + start;

            execinit();
            // running the command, passing a buf argument
            execsh(nil, buf->current, buf, envy);

            return OK_1;
        }
        if(c == '\n')
            break;
        if(c == '\'' || c == '"' || c == '\\'){
            insert(buf, c);
            if(!escapetoken(bp, buf, 1, c))
                return ERROR_0;
            continue;
        }
        rinsert(buf, c);
    }
    SYNERR(line);
    fprint(STDERR, "missing closing %c after `\n", term);
    return ERROR_0;
}
/*e: function bquote */

/*s: function nextrune */
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
        if (c == '\\') {
            if (Bgetrune(bp) == '\n') {
                mkinline++;
                if (elide)
                    continue;
                // else
                return ' ';
            }
            Bungetrune(bp);
        }
        if (c == '\n')
            mkinline++;
        return c;
    }
}
/*e: function nextrune */
/*e: mk/lex.c */
