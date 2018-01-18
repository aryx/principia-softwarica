/*s: port/getuser.c */
#include <u.h>
#include <libc.h>

/*s: function [[getuser]] */
char*
getuser(void)
{
    static char user[64];
    fdt fd;
    int n;

    fd = open("/dev/user", OREAD);
    if(fd < 0)
        return "none";
    n = read(fd, user, (sizeof user)-1);
    close(fd);
    if(n <= 0)
        strcpy(user, "none");
    else
        user[n] = '\0';
    return user;
}
/*e: function [[getuser]] */
/*e: port/getuser.c */
