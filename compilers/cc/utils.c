/*s: cc/utils.c */
#include "cc.h"

/*s: function [[errorexit]] */
/// main | compile | newio | newfile | yylex | ... -> <>
void
errorexit(void)
{
    if(outfile)
        remove(outfile);
    exits("error");
}
/*e: function [[errorexit]] */

/*s: function [[gethunk]] */
/*
 * claude: INVARIANT — gethunk() hands out ZEROED memory.
 *
 * alloc()/allocn() carve chunks out of the `hunk` buffer refilled here
 * without clearing them, so the whole compiler relies on freshly
 * allocated nodes having all fields zero (e.g. Node.left/right/type NULL,
 * Sym links NULL). Original Plan 9 got this for free: sbrk() returns
 * fresh kernel pages, which are always zero-filled. Any replacement
 * allocator MUST preserve that guarantee, hence the memset() below.
 */
void
gethunk(void)
{
    char *h;
    long nh;

    nh = NHUNK;
    if(thunk >= 10L*NHUNK)
        nh = 10L*NHUNK;

    /* claude: was sbrk(nh) in original Plan 9 code.
     *
     * On macOS, sbrk() is deprecated and hard-capped: it satisfies only a
     * ~4MB reservation and then returns -1 for every further request,
     * regardless of free RAM. Small source files compile fine, but once
     * the arena has chewed through that quota gethunk() fails and the
     * compile dies with "out of memory" — this bit us on
     * principia/lib_graphics/libimg/torgbv.c with 8c.
     *
     * We now refill the arena with the real libc malloc(). This is only
     * possible because the old compat.c malloc->alloc wrapper has been
     * removed (see compilers/cc/compat.c); with that wrapper in place this
     * call recursed gethunk->malloc->alloc->gethunk forever. malloc() is
     * uncapped on macOS/Linux/Plan 9, so torgbv.c and friends now build.
     *
     * Two adjustments versus sbrk(): malloc() returns nil (not (char*)-1)
     * on failure, and — unlike sbrk's fresh kernel pages — malloc() does
     * NOT zero its memory, so we memset() the chunk to uphold gethunk's
     * zeroed-memory invariant (see the note above this function).
     */
    /*old: h = (char*)sbrk(nh); */
    h = (char*)malloc(nh);
    if(h == nil) {
        yyerror("out of memory");
        errorexit();
    }
    memset(h, 0, nh);
    hunk = h;
    nhunk = nh;
    thunk += nh;
}
/*e: function [[gethunk]] */

/*s: function [[alloc]] */
/*
 * real allocs
 */
void*
alloc(long n)
{
    void *p;

    while((uintptr)hunk & MAXALIGN) {
        hunk++;
        nhunk--;
    }
    while(nhunk < n)
        gethunk();
    p = hunk;
    nhunk -= n;
    hunk += n;
    return p;
}
/*e: function [[alloc]] */

/*s: function [[allocn]] */
void*
allocn(void *p, long on, long n)
{
    void *q;

    q = (uchar*)p + on;
    if(q != hunk || nhunk < n) {
        while(nhunk < on+n)
            gethunk();
        memmove(hunk, p, on);
        p = hunk;
        hunk += on;
        nhunk -= on;
    }
    hunk += n;
    nhunk -= n;
    return p;
}
/*e: function [[allocn]] */

/*s: function [[yyerror]] */
void
yyerror(char *fmt, ...)
{
    char buf[STRINGSZ];
    va_list arg;

    /*s: [[yyerror()]] when called from yyparse */
    /*
     * hack to intercept message from yaccpar
     */
    if(strcmp(fmt, "syntax error") == 0) {
        yyerror("syntax error, last name: %s", symb);
        return;
    }
    /*e: [[yyerror()]] when called from yyparse */

    va_start(arg, fmt);
    vseprint(buf, buf+sizeof(buf), fmt, arg);
    va_end(arg);

    Bprint(&diagbuf, "%L %s\n", lineno, buf);
    nerrors++;

    if(nerrors > 10) {
        Bprint(&diagbuf, "too many errors\n");
        errorexit();
    }
}
/*e: function [[yyerror]] */

/*e: cc/utils.c */
