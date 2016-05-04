/*s: lib_graphics/libdraw/newwindow.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function newwindow */
/* Connect us to new window, if possible */
errorneg1
newwindow(char *str)
{
    fdt fd;
    char *wsys;
    char buf[256];

    wsys = getenv("wsys");
    /*s: [[newwindow()]] sanity check wsys */
    if(wsys == nil)
        return ERROR_NEG1;
    /*e: [[newwindow()]] sanity check wsys */
    fd = open(wsys, ORDWR);
    free(wsys);
    /*s: [[newwindow()]] sanity check fd */
    if(fd < 0)
        return ERROR_NEG1;
    /*e: [[newwindow()]] sanity check fd */
    rfork(RFNAMEG);

    if(str)
        snprint(buf, sizeof buf, "new %s", str);
    else
        strcpy(buf, "new");

    return mount(fd, -1, "/dev", MBEFORE, buf);
}
/*e: function newwindow */

/*e: lib_graphics/libdraw/newwindow.c */
