/*s: 8l/compat.c */
#include	"l.h"

/*
 * claude: same fix as compilers/cc/compat.c — the "fake malloc" below
 * (a zeroed bump arena over hunk/nhunk, plus a no-op free and no-op
 * setmalloctag) is DISABLED via #if 0. It made malloc() unusable inside
 * gethunk(): gethunk()->malloc()->gethunk() recursed forever. Original
 * Plan 9 avoided that because gethunk() used sbrk(), but macOS sbrk() is
 * hard-capped at ~4MB and then returns -1, so large links (e.g. 8.jpg)
 * died with "out of memory". We now use the real libc malloc/free
 * directly: gethunk() refills the arena with the real malloc() (see
 * utils.c), the malloc() call sites get honest heap allocation, and the
 * free() call sites (pass.c, obj.c, span.c) become real frees instead of
 * leaks. (The syncweb chunk markers are kept; only the code is disabled.)
 */

/*s: function [[malloc]] */
/*
 * fake malloc
 */
//void*
//malloc(ulong n)
//{
//    void *p;
//
//    // upper_round(n, 8)
//    while(n & 7)
//        n++;
//
//    while(nhunk < n)
//        gethunk();
//    p = hunk;
//    nhunk -= n;
//    hunk += n;
//    return p;
//}
/*e: function [[malloc]] */

/*s: function [[free]] */
//void
//free(void *p)
//{
//    USED(p);
//}
/*e: function [[free]] */

/*s: function [[setmalloctag]] */
////@Scheck: looks dead, but because we redefine malloc/free we must also redefine that
//void setmalloctag(void *v, ulong pc)
//{
//    USED(v); USED(pc);
//}
/*e: function [[setmalloctag]] */

/*s: function [[fileexists]] */
//old: now in libc
//int
//fileexists(char *s)
//{
//    byte dirbuf[400];
//
//    /* it's fine if stat result doesn't fit in dirbuf, since even then the file exists */
//    return stat(s, dirbuf, sizeof(dirbuf)) >= 0;
//}
/*e: function [[fileexists]] */
/*e: 8l/compat.c */
