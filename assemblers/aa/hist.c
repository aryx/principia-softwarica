/*s: assemblers/aa/hist.c */
#include "aa.h"

/*s: function [[linehist]] */
/// (pinit | macinc -> newfile) | (GETC -> filbuf) | maclin -> <>
void
linehist(char *f, int local_line)
{
    Hist *h;

    /*s: [[linehist()]] debug */
    if(debug['f'])
        if(f) {
            if(local_line)
                print("%4ld: %s (#line %d)\n", lineno, f, local_line);
            else
                print("%4ld: %s\n", lineno, f);
        } else
            print("%4ld: <pop>\n", lineno);
    /*e: [[linehist()]] debug */

    h = alloc(sizeof(Hist));
    h->filename = f;
    h->global_line = lineno;
    h->local_line = local_line;

    //add_list(hist, ehist, h)
    h->link = H;
    if(ehist == H) {
        hist = h;
        ehist = h;
        return;
    }
    ehist->link = h;
    ehist = h;
}
/*e: function [[linehist]] */

/*s: function [[prfile]] */
/// yyerror -> <>
void
prfile(long l)
{
    Hist a[HISTSZ];
    int n = 0;
    Hist *h;
    int i;
    long d;

    /*s: [[prfile()]] compute a and n */
    for(h = hist; h != H; h = h->link) {
        if(l >= h->global_line) {
            if(h->filename) {
                // a #include
                if(h->local_line == 0) {
                    if(n >= 0 && n < HISTSZ)
                        a[n] = *h;
                    n++;
                } 
                /*s: [[prfile()]] compute a and n, when line directive */
                // a #line
                else {
                    if(n > 0 && n < HISTSZ)
                        // previous was a #include
                        if(a[n-1].local_line == 0) {
                            a[n] = *h;
                            n++;
                        } else
                            a[n-1] = *h; // overwrite previous #line
                }
                /*e: [[prfile()]] compute a and n, when line directive */
            }
            // a pop
            else {
                n--;
                /*s: [[prfile()]] compute a and n, when pop, adjust parents */
                if(n >= 0 && n < HISTSZ) {
                    d = h->global_line - a[n].global_line;
                    for(i=0; i<n; i++)
                        a[i].global_line += d;
                }
                /*e: [[prfile()]] compute a and n, when pop, adjust parents */
            }
        }
    }
    /*e: [[prfile()]] compute a and n */
    if(n > HISTSZ)
        n = HISTSZ;
    for(i=0; i<n; i++)
        print("%s:%ld ", a[i].filename, 
                         (long)(l - a[i].global_line + a[i].local_line + 1));
}
/*e: function [[prfile]] */

/*e: assemblers/aa/hist.c */
