/*s: webfs/io.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ip.h>
#include <plumb.h>
#include <thread.h>
#include <fcall.h>
#include <9p.h>
#include <mp.h>
#include <libsec.h>
#include "dat.h"
#include "fns.h"

/*s: function [[_iovfprint]](webfs) */
static long
_iovfprint(va_list *arg)
{
    int fd;
    char *fmt;
    va_list arg2;

    fd = va_arg(*arg, int);
    fmt = va_arg(*arg, char*);
    arg2 = va_arg(*arg, va_list);
    return vfprint(fd, fmt, arg2);
}
/*e: function [[_iovfprint]](webfs) */

/*s: function [[iovfprint]](webfs) */
int
iovfprint(Ioproc *io, int fd, char *fmt, va_list arg)
{
    return iocall(io, _iovfprint, fd, fmt, arg);
}
/*e: function [[iovfprint]](webfs) */

/*s: function [[ioprint]](webfs) */
int
ioprint(Ioproc *io, int fd, char *fmt, ...)
{
    int n;
    va_list arg;

    va_start(arg, fmt);
    n = iovfprint(io, fd, fmt, arg);
    va_end(arg);
    return n;
}
/*e: function [[ioprint]](webfs) */

/*s: function [[_iotlsdial]](webfs) */
static long
_iotlsdial(va_list *arg)
{
    char *addr, *local, *dir, *servername;
    int *cfdp, fd, tfd, usetls;
    TLSconn conn;

    addr = va_arg(*arg, char*);
    local = va_arg(*arg, char*);
    dir = va_arg(*arg, char*);
    cfdp = va_arg(*arg, int*);
    usetls = va_arg(*arg, int);
    servername = va_arg(*arg, char*);

    fd = dial(addr, local, dir, cfdp);
    if(fd < 0)
        return -1;
    if(!usetls)
        return fd;

    memset(&conn, 0, sizeof conn);
    /* does no good, so far anyway */
    // conn.chain = readcertchain("/sys/lib/ssl/vsignss.pem");
    /* claude: SNI is required by most modern HTTPS servers */
    if(servername != nil)
        conn.serverName = strdup(servername);

    tfd = tlsClient(fd, &conn);
    close(fd);
    if(tfd < 0)
        fprint(2, "%s: tlsClient: %r\n", argv0);
    else {
        /* BUG: check cert here? */
        if(conn.cert)
            free(conn.cert);
    }
    free(conn.serverName);
    return tfd;
}
/*e: function [[_iotlsdial]](webfs) */

/*s: function [[iotlsdial]](webfs) */
int
iotlsdial(Ioproc *io, char *addr, char *local, char *dir, int *cfdp, int usetls, char *servername)
{
    return iocall(io, _iotlsdial, addr, local, dir, cfdp, usetls, servername);
}
/*e: function [[iotlsdial]](webfs) */
/*e: webfs/io.c */
