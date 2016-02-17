/*s: lib_graphics/libdraw/openfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function openfont */
Font*
openfont(Display *d, char *name)
{
    Font *fnt;
    fdt fd;
    int i, n;
    char *buf;
    Dir *dir;

    fd = open(name, OREAD);
    /*s: [[openfont()]] sanity check fd */
    if(fd < 0)
        return nil;
    /*e: [[openfont()]] sanity check fd */

    // n = filename_size(fd)
    dir = dirfstat(fd);
    /*s: [[openfont()]] sanity check dir */
    if(dir == nil){
    Err0:
        close(fd);
        return nil;
    }
    /*e: [[openfont()]] sanity check dir */
    n = dir->length;
    free(dir);

    buf = malloc(n+1);
    /*s: [[openfont()]] sanity check buf */
    if(buf == nil)
        goto Err0;
    /*e: [[openfont()]] sanity check buf */
    buf[n] = '\0';
    i = read(fd, buf, n);
    close(fd);
    /*s: [[openfont()]] sanity check i */
    if(i != n){
        free(buf);
        return nil;
    }
    /*e: [[openfont()]] sanity check i */

    fnt = buildfont(d, buf, name);

    free(buf);
    return fnt;
}
/*e: function openfont */
/*e: lib_graphics/libdraw/openfont.c */
