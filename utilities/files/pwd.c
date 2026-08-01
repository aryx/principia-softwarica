/*s: files/pwd.c */
#include <u.h>
#include <libc.h>

/*s: function [[main]](pwd.c) */
void
main(int argc, char *argv[])
{
    char pathname[512];

    USED(argc, argv);
    if(getwd(pathname, sizeof(pathname)) == ERROR_0) {
        fprint(STDERR, "pwd: %r\n");
        exits("getwd");
    }
    // else
    print("%s\n", pathname);
    exits(nil);
}
/*e: function [[main]](pwd.c) */
/*e: files/pwd.c */
