/*s: lib_networking/libip/readipifc.c */
#include <u.h>
#include <libc.h>
#include <ctype.h>
#include <ip.h>

/*s: function [[findfield]] */
static char*
findfield(char *name, char **f, int n)
{
    int i;

    for(i = 0; i < n-1; i++)
        if(strcmp(f[i], name) == 0)
            return f[i+1];
    return "";
}
/*e: function [[findfield]] */

/*s: function [[_readipifc]] */
static Ipifc**
_readipifc(char *file, Ipifc **l, int index)
{
    int i, n, fd, lines;
    char buf[4*1024];
    char *line[32];
    char *f[64];
    Ipifc *ifc, **l0;
    Iplifc *lifc, **ll;

    /* read the file */
    fd = open(file, OREAD);
    if(fd < 0)
        return l;
    n = 0;
    while((i = read(fd, buf+n, sizeof(buf)-1-n)) > 0 && n < sizeof(buf) - 1)
        n += i;
    buf[n] = 0;
    close(fd);

    //if(strncmp(buf, "device", 6) != 0)
    //    return _readoldipifc(buf, l, index);

    /* ignore ifcs with no associated device */
    if(strncmp(buf+6, "  ", 2) == 0)
        return l;
    /* allocate new interface */
    *l = ifc = mallocz(sizeof(Ipifc), 1);
    if(ifc == nil)
        return l;
    l0 = l;
    l = &ifc->next;
    ifc->index = index;

    lines = getfields(buf, line, nelem(line), 1, "\n");

    /* pick off device specific info(first line) */
    n = tokenize(line[0], f, nelem(f));
    if(n%2 != 0)
        goto lose;
    strncpy(ifc->dev, findfield("device", f, n), sizeof(ifc->dev));
    ifc->dev[sizeof(ifc->dev)-1] = 0;
    if(ifc->dev[0] == 0){
lose:
        free(ifc);
        *l0 = nil;
        return l;
    }
    ifc->mtu          = strtoul(findfield("maxtu", f, n), nil, 10);
    ifc->sendra6      = atoi(findfield("sendra", f, n));
    ifc->recvra6      = atoi(findfield("recvra", f, n));
    ifc->rp.mflag     = atoi(findfield("mflag", f, n));
    ifc->rp.oflag     = atoi(findfield("oflag", f, n));
    ifc->rp.maxraint  = atoi(findfield("maxraint", f, n));
    ifc->rp.minraint  = atoi(findfield("minraint", f, n));
    ifc->rp.linkmtu   = atoi(findfield("linkmtu", f, n));
    ifc->rp.reachtime = atoi(findfield("reachtime", f, n));
    ifc->rp.rxmitra   = atoi(findfield("rxmitra", f, n));
    ifc->rp.ttl       = atoi(findfield("ttl", f, n));
    ifc->rp.routerlt  = atoi(findfield("routerlt", f, n));
    ifc->pktin        = strtoul(findfield("pktin", f, n), nil, 10);
    ifc->pktout       = strtoul(findfield("pktout", f, n), nil, 10);
    ifc->errin        = strtoul(findfield("errin", f, n), nil, 10);
    ifc->errout       = strtoul(findfield("errout", f, n), nil, 10);

    /* now read the addresses */
    ll = &ifc->lifc;
    for(i = 1; i < lines; i++){
        n = tokenize(line[i], f, nelem(f));
        if(n < 5)
            break;

        /* allocate new local address */
        *ll = lifc = mallocz(sizeof(Iplifc), 1);
        ll = &lifc->next;

        parseip(lifc->ip, f[0]);
        parseipmask(lifc->mask, f[1]);
        parseip(lifc->net, f[2]);

        lifc->validlt = strtoul(f[3], nil, 10);
        lifc->preflt = strtoul(f[4], nil, 10);
    }

    return l;
}
/*e: function [[_readipifc]] */

/*s: function [[_freeifc]] */
static void
_freeifc(Ipifc *ifc)
{
    Ipifc *next;
    Iplifc *lnext, *lifc;

    if(ifc == nil)
        return;
    for(; ifc; ifc = next){
        next = ifc->next;
        for(lifc = ifc->lifc; lifc; lifc = lnext){
            lnext = lifc->next;
            free(lifc);
        }
        free(ifc);
    }
}
/*e: function [[_freeifc]] */

/*s: function [[readipifc]] */
Ipifc*
readipifc(char *net, Ipifc *ifc, int index)
{
    int fd, i, n;
    Dir *dir;
    char directory[128];
    char buf[128];
    Ipifc **l;

    _freeifc(ifc);

    l = &ifc;
    ifc = nil;

    if(net == nil)
        net = "/net";
    snprint(directory, sizeof(directory), "%s/ipifc", net);

    if(index >= 0){
        snprint(buf, sizeof(buf), "%s/%d/status", directory, index);
        _readipifc(buf, l, index);
    } else {
        fd = open(directory, OREAD);
        if(fd < 0)
            return nil;
        n = dirreadall(fd, &dir);
        close(fd);

        for(i = 0; i < n; i++){
            if(strcmp(dir[i].name, "clone") == 0)
                continue;
            if(strcmp(dir[i].name, "stats") == 0)
                continue;
            snprint(buf, sizeof(buf), "%s/%s/status", directory, dir[i].name);
            l = _readipifc(buf, l, atoi(dir[i].name));
        }
        free(dir);
    }

    return ifc;
}
/*e: function [[readipifc]] */
/*e: lib_networking/libip/readipifc.c */
