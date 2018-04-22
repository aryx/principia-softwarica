/*s: networking/ip/httpfile.c */
/* contributed by 20h@r-36.net, September 2005 */

#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ndb.h>
#include <thread.h>
#include <fcall.h>
#include <9p.h>
#include <mp.h>
#include <libsec.h>

/*s: enum [[_anon_ (networking/ip/httpfile.c)]] */
enum
{
    Blocksize = 64*1024,
    Stacksize = 8192,
};
/*e: enum [[_anon_ (networking/ip/httpfile.c)]] */

/*s: global [[host]] */
char *host;
/*e: global [[host]] */
/*s: global [[file]] */
char *file;
/*e: global [[file]] */
/*s: global [[port]] */
char *port;
/*e: global [[port]] */
/*s: global [[url]] */
char *url;
/*e: global [[url]] */
/*s: global [[get]] */
char *get;
/*e: global [[get]] */
/*s: global [[user]]([[(networking/ip/httpfile.c)]]) */
char *user;
/*e: global [[user]]([[(networking/ip/httpfile.c)]]) */
/*s: global [[net]]([[(networking/ip/httpfile.c)]]) */
char *net = "net";
/*e: global [[net]]([[(networking/ip/httpfile.c)]]) */

/*s: global [[size]] */
vlong size;
/*e: global [[size]] */
/*s: global [[usetls]]([[(networking/ip/httpfile.c)]]) */
int usetls;
/*e: global [[usetls]]([[(networking/ip/httpfile.c)]]) */
/*s: global [[debug]]([[(networking/ip/httpfile.c)]]) */
int debug;
/*e: global [[debug]]([[(networking/ip/httpfile.c)]]) */
/*s: global [[ncache]] */
int ncache;
/*e: global [[ncache]] */
/*s: global [[mcache]] */
int mcache;
/*e: global [[mcache]] */

/*s: function [[usage]]([[(networking/ip/httpfile.c)]]) */
void
usage(void)
{
    fprint(2, "usage: httpfile [-Dd] [-c count] [-f file] [-m mtpt] [-s srvname] [-x net] url\n");
    exits("usage");
}
/*e: function [[usage]]([[(networking/ip/httpfile.c)]]) */

/*s: enum [[_anon_ (networking/ip/httpfile.c)2]] */
enum
{
    Qroot,
    Qfile,
};
/*e: enum [[_anon_ (networking/ip/httpfile.c)2]] */

/*s: macro [[PATH]] */
#define PATH(type, n)		((type)|((n)<<8))
/*e: macro [[PATH]] */
/*s: macro [[TYPE]]([[(networking/ip/httpfile.c)]]) */
#define TYPE(path)			((int)(path) & 0xFF)
/*e: macro [[TYPE]]([[(networking/ip/httpfile.c)]]) */
/*s: macro [[NUM]] */
#define NUM(path)			((uint)(path)>>8)
/*e: macro [[NUM]] */

/*s: global [[reqchan]] */
Channel *reqchan;
/*e: global [[reqchan]] */
/*s: global [[httpchan]] */
Channel *httpchan;
/*e: global [[httpchan]] */
/*s: global [[finishchan]] */
Channel *finishchan;
/*e: global [[finishchan]] */
/*s: global [[time0]] */
ulong time0;
/*e: global [[time0]] */

typedef struct Block Block;
/*s: struct [[Block]]([[(networking/ip/httpfile.c)]]) */
struct Block
{
    uchar *p;
    vlong off;
    vlong len;
    Block *link;
    long lastuse;
    Req *rq;
    Req **erq;
};
/*e: struct [[Block]]([[(networking/ip/httpfile.c)]]) */

typedef struct Blocklist Blocklist;
/*s: struct [[Blocklist]] */
struct Blocklist
{
    Block *first;
    Block **end;
};
/*e: struct [[Blocklist]] */

/*s: global [[cache]] */
Blocklist cache;
/*e: global [[cache]] */
/*s: global [[inprogress]] */
Blocklist inprogress;
/*e: global [[inprogress]] */

/*s: function [[queuereq]] */
void
queuereq(Block *b, Req *r)
{
    if(b->rq==nil)
        b->erq = &b->rq;
    *b->erq = r;
    r->aux = nil;
    b->erq = (Req**)&r->aux;
}
/*e: function [[queuereq]] */

/*s: function [[addblock]] */
void
addblock(Blocklist *l, Block *b)
{
    if(debug)
        print("adding: %p %lld\n", b, b->off);

    if(l->first == nil)
        l->end = &l->first;
    *l->end = b;
    b->link = nil;
    l->end = &b->link;
    b->lastuse = time(0);
}
/*e: function [[addblock]] */

/*s: function [[delreq]] */
void
delreq(Block *b, Req *r)
{
    Req **l;

    for(l = &b->rq; *l; l = (Req**)&(*l)->aux){
        if(*l == r){
            *l = r->aux;
            if(*l == nil)
                b->erq = l;
            free(r);
            return;
        }
    }
}
/*e: function [[delreq]] */

/*s: function [[evictblock]] */
void
evictblock(Blocklist *cache)
{
    Block **l, **oldest, *b;

    if(cache->first == nil)
        return;

    oldest = nil;
    for(l=&cache->first; *l; l=&(*l)->link)
        if(oldest == nil || (*oldest)->lastuse > (*l)->lastuse)
            oldest = l;

    b = *oldest;
    *oldest = (*oldest)->link;
    if(*oldest == nil)
        cache->end = oldest;
    free(b->p);
    free(b);
    ncache--;
}
/*e: function [[evictblock]] */

/*s: function [[findblock]] */
Block *
findblock(Blocklist *s, vlong off)
{
    Block *b;

    for(b = s->first; b != nil; b = b->link){
        if(b->off <= off && off < b->off + Blocksize){
            if(debug)
                print("found: %lld -> %lld\n", off, b->off);
            b->lastuse = time(0);
            return b;
        }
    }

    return nil;
}
/*e: function [[findblock]] */

/*s: function [[readfrom]] */
void
readfrom(Req *r, Block *b)
{
    int d, n;

    b->lastuse = time(0);

    n = r->ifcall.count;
    d = r->ifcall.offset - b->off;
    if(b->off + d + n > b->off + b->len)
        n = b->len - d;
    if(debug)
        print("Reading from: %p %d %d\n", b->p, d, n);
    memmove(r->ofcall.data, b->p + d, n);
    r->ofcall.count = n;

    respond(r, nil);
}
/*e: function [[readfrom]] */

/*s: function [[hangupclient]] */
void
hangupclient(Srv*)
{
    if(debug)
        print("Hangup.\n");

    threadexitsall("done");
}
/*e: function [[hangupclient]] */

/*s: function [[dotls]] */
int
dotls(int fd)
{
    TLSconn conn;

    if((fd=tlsClient(fd, &conn)) < 0)
        sysfatal("tlsclient: %r");

    if(conn.cert != nil)
        free(conn.cert);

    return fd;
}
/*e: function [[dotls]] */

/*s: function [[nocr]] */
char*
nocr(char *s)
{
    char *r, *w;

    for(r=w=s; *r; r++)
        if(*r != '\r')
            *w++ = *r;
    *w = 0;
    return s;
}
/*e: function [[nocr]] */

/*s: function [[readhttphdr]] */
char*
readhttphdr(Biobuf *netbio, vlong *size)
{
    char *s, *stat;

    stat = nil;
    while((s = Brdstr(netbio, '\n', 1)) != nil && s[0] != '\r'
            && s[0] != '\0'){
        if(stat == nil)
            stat = estrdup9p(s);
        if(strncmp(s, "Content-Length: ", 16) == 0 && size != nil)
            *size = atoll(s + 16);
        free(s);
    }
    if(stat)
        nocr(stat);

    return stat;
}
/*e: function [[readhttphdr]] */

/*s: function [[dialhttp]] */
int
dialhttp(Biobuf *netbio)
{
    int netfd;

    netfd = dial(netmkaddr(host, net, port), 0, 0, 0);
    if(netfd < 0)
        sysfatal("dial: %r");
    if(usetls)
        netfd = dotls(netfd);
    Binit(netbio, netfd, OREAD);

    return netfd;
}
/*e: function [[dialhttp]] */

/*s: function [[getrange]] */
uchar*
getrange(Block *b)
{
    uchar *data;
    char *status;
    int netfd;
    static Biobuf netbio;

    b->len = Blocksize;
    if(b->off + b->len > size)
        b->len = size - b->off;

    if(debug)
        print("getrange: %lld %lld\n", b->off, b->len);

    netfd = dialhttp(&netbio);

    fprint(netfd, 
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Accept-Encoding:\r\n"
        "Range: bytes=%lld-%lld\r\n"
        "\r\n",
        get, host, b->off, b->off+b->len);
    Bflush(&netbio);

    status = readhttphdr(&netbio, nil);
    if(status == nil)
        return nil;

    /*
     * Some servers (e.g., www.google.com) return 200 OK
     * when you ask for the entire page in one range.
     */
    if(strstr(status, "206 Partial Content")==nil
    && (b->off!=0 || b->len!=size || strstr(status, "200 OK")==nil)){
        free(status);
        close(netfd);
        werrstr("did not get requested range");
        return nil;
    }
    free(status);

    data = emalloc9p(b->len);
    if(Bread(&netbio, data, b->len) != b->len){
        free(data);
        close(netfd);
        werrstr("not enough bytes read");
        return nil;
    }

    b->p = data;

    close(netfd);
    return data;
}
/*e: function [[getrange]] */

/*s: function [[httpfilereadproc]] */
void
httpfilereadproc(void*)
{
    Block *b;

    threadsetname("httpfilereadproc");

    for(;;){
        b = recvp(httpchan);
        if(b == nil)
            continue;
        if(getrange(b) == nil)
            sysfatal("getrange: %r");
        sendp(finishchan, b);
    }
}
/*e: function [[httpfilereadproc]] */

typedef struct Tab Tab;
/*s: struct [[Tab]] */
struct Tab
{
    char *name;
    ulong mode;
};
/*e: struct [[Tab]] */

/*s: global [[tab]] */
Tab tab[] =
{
    "/",		DMDIR|0555,
    nil,		0444,
};
/*e: global [[tab]] */

/*s: function [[fillstat]] */
static void
fillstat(Dir *d, uvlong path)
{
    Tab *t;

    memset(d, 0, sizeof(*d));
    d->uid = estrdup9p(user);
    d->gid = estrdup9p(user);
    d->qid.path = path;
    d->atime = d->mtime = time0;
    t = &tab[TYPE(path)];
    d->name = estrdup9p(t->name);
    d->length = size;
    d->qid.type = t->mode>>24;
    d->mode = t->mode;
}
/*e: function [[fillstat]] */

/*s: function [[fsattach]] */
static void
fsattach(Req *r)
{
    if(r->ifcall.aname && r->ifcall.aname[0]){
        respond(r, "invalid attach specifier");
        return;
    }
    r->fid->qid.path = PATH(Qroot, 0);
    r->fid->qid.type = QTDIR;
    r->fid->qid.vers = 0;
    r->ofcall.qid = r->fid->qid;
    respond(r, nil);
}
/*e: function [[fsattach]] */

/*s: function [[fsstat]]([[(networking/ip/httpfile.c)]]) */
static void
fsstat(Req *r)
{
    fillstat(&r->d, r->fid->qid.path);
    respond(r, nil);
}
/*e: function [[fsstat]]([[(networking/ip/httpfile.c)]]) */

/*s: function [[rootgen]] */
static int
rootgen(int i, Dir *d, void*)
{
    i += Qroot + 1;
    if(i <= Qfile){
        fillstat(d, i);
        return 0;
    }
    return -1;
}
/*e: function [[rootgen]] */

/*s: function [[fswalk1]] */
static char*
fswalk1(Fid *fid, char *name, Qid *qid)
{
    int i;
    ulong path;

    path = fid->qid.path;
    if(!(fid->qid.type & QTDIR))
        return "walk in non-directory";

    if(strcmp(name, "..") == 0){
        switch(TYPE(path)){
        case Qroot:
            return nil;
        default:
            return "bug in fswalk1";
        }
    }

    i = TYPE(path) + 1;
    while(i < nelem(tab)){
        if(strcmp(name, tab[i].name) == 0){
            qid->path = PATH(i, NUM(path));
            qid->type = tab[i].mode>>24;
            return nil;
        }
        if(tab[i].mode & DMDIR)
            break;
        i++;
    }
    return "directory entry not found";
}
/*e: function [[fswalk1]] */

/*s: function [[getfilesize]] */
vlong
getfilesize(void)
{
    char *status;
    vlong size;
    int netfd;
    static Biobuf netbio;

    netfd = dialhttp(&netbio);

    fprint(netfd, 
        "HEAD %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Accept-Encoding:\r\n"
        "\r\n",
        get, host);

    status = readhttphdr(&netbio, &size);
    if(strstr(status, "200 OK") == nil){
        werrstr("%s", status);
        size = -1;
    }
    free(status);

    close(netfd);
    return size;
}
/*e: function [[getfilesize]] */

/*s: function [[fileread]]([[(networking/ip/httpfile.c)]]) */
void
fileread(Req *r)
{
    Block *b;

    if(r->ifcall.offset > size){
        respond(r, nil);
        return;
    }

    if((b = findblock(&cache, r->ifcall.offset)) != nil){
        readfrom(r, b);
        return;
    }
    if((b = findblock(&inprogress, r->ifcall.offset)) == nil){
        b = emalloc9p(sizeof(Block));
        b->off = r->ifcall.offset - (r->ifcall.offset % Blocksize);
        addblock(&inprogress, b);
        if(inprogress.first == b)
            sendp(httpchan, b);
    }
    queuereq(b, r);
}
/*e: function [[fileread]]([[(networking/ip/httpfile.c)]]) */

/*s: function [[fsopen]] */
static void
fsopen(Req *r)
{
    if(r->ifcall.mode != OREAD){
        respond(r, "permission denied");
        return;
    }
    respond(r, nil);
}
/*e: function [[fsopen]] */

/*s: function [[finishthread]] */
void
finishthread(void*)
{
    Block *b;
    Req *r, *nextr;

    threadsetname("finishthread");

    for(;;){
        b = recvp(finishchan);
        assert(b == inprogress.first);
        inprogress.first = b->link;
        ncache++;
        if(ncache >= mcache)
            evictblock(&cache);
        addblock(&cache, b);
        for(r=b->rq; r; r=nextr){
            nextr = r->aux;
            readfrom(r, b);
        }
        b->rq = nil;
        if(inprogress.first)
            sendp(httpchan, inprogress.first);
    }
}
/*e: function [[finishthread]] */

/*s: function [[fsnetproc]] */
void
fsnetproc(void*)
{
    Req *r;
    Block *b;

    threadcreate(finishthread, nil, 8192);

    threadsetname("fsnetproc");

    for(;;){
        r = recvp(reqchan);
        switch(r->ifcall.type){
        case Tflush:
            b = findblock(&inprogress, r->ifcall.offset);
            delreq(b, r->oldreq);
            respond(r->oldreq, "interrupted");
            respond(r, nil);
            break;
        case Tread:
            fileread(r);
            break;
        default:
            respond(r, "bug in fsthread");
            break;
        }
    }
}
/*e: function [[fsnetproc]] */

/*s: function [[fsflush]] */
static void
fsflush(Req *r)
{
    sendp(reqchan, r);
}
/*e: function [[fsflush]] */

/*s: function [[fsread]]([[(networking/ip/httpfile.c)]]) */
static void
fsread(Req *r)
{
    char e[ERRMAX];
    ulong path;

    path = r->fid->qid.path;
    switch(TYPE(path)){
    case Qroot:
        dirread9p(r, rootgen, nil);
        respond(r, nil);
        break;
    case Qfile:
        sendp(reqchan, r);
        break;
    default:
        snprint(e, sizeof(e), "bug in fsread path=%lux", path);
        respond(r, e);
        break;
    }
}
/*e: function [[fsread]]([[(networking/ip/httpfile.c)]]) */

/*s: global [[fs]] */
Srv fs = 
{
.attach=		fsattach,
.walk1=		fswalk1,
.open=		fsopen,
.read=		fsread,
.stat=		fsstat,
.flush=		fsflush,
.end=		hangupclient,
};
/*e: global [[fs]] */

/*s: function [[threadmain]] */
void
threadmain(int argc, char **argv)
{
    char *defport, *mtpt, *srvname, *p;

    mtpt = nil;
    srvname = nil;
    ARGBEGIN{
    case 'D':
        chatty9p++;
        break;
    case 'd':
        debug++;
        break;
    case 's':
        srvname = EARGF(usage());
        break;
    case 'm':
        mtpt = EARGF(usage());
        break;
    case 'c':
        mcache = atoi(EARGF(usage()));
        break;
    case 'f':
        file = EARGF(usage());
        break;
    case 'x':
        net = smprint("%s/net", EARGF(usage()));
        break;
    default:
        usage();
    }ARGEND;

    if(srvname == nil && mtpt == nil)
        mtpt = ".";

    if(argc < 1)
        usage();
    if(mcache <= 0)
        mcache = 32;

    time0 = time(0);
    host = url = estrdup9p(argv[0]);

    defport = nil;
    if(!cistrncmp(url, "https://", 8)){
        host += 8;
        usetls = 1;
        defport = "https";
    }else if(!cistrncmp(url, "http://", 7)){
        host += 7;
        defport = "http";
    }else
        sysfatal("unsupported url: %s", url);

    if((p = strchr(host, '/')) != nil){
        get = estrdup9p(p);
        *p = '\0';
    }else
        get = "/";

    port = strchr(host, ':');
    if(port != nil)
        *port++ = '\0';
    else
        port = defport;

    if(file == nil){
        file = strrchr(get, '/')+1;
        if(*file == 0)
            file = "index";
    }

    tab[Qfile].name = file;
    user = getuser();
    size = getfilesize();
    if(size < 0)
        sysfatal("getfilesize: %r");

    reqchan = chancreate(sizeof(Req*), 0);
    httpchan = chancreate(sizeof(Block*), 0);
    finishchan = chancreate(sizeof(Block*), 0);

    procrfork(fsnetproc, nil, Stacksize, RFNAMEG|RFNOTEG);
    procrfork(httpfilereadproc, nil, Stacksize, RFNAMEG|RFNOTEG);

    threadpostmountsrv(&fs, srvname, mtpt, MBEFORE);
    threadexits(0);
}
/*e: function [[threadmain]] */
/*e: networking/ip/httpfile.c */
