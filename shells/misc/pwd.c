/*s: misc/pwd.c */
#include <u.h>
#include <libc.h>
/*s: function main (misc/pwd.c) */
/*
 * Print working (current) directory
 */

void
main(int argc, char *argv[])
{
    char pathname[512];

    USED(argc, argv);
    if(getwd(pathname, sizeof(pathname)) == 0) {
        fprint(2, "pwd: %r\n");
        exits("getwd");
    }
    print("%s\n", pathname);
    exits(0);
}
/*e: function main (misc/pwd.c) */
/*e: misc/pwd.c */
