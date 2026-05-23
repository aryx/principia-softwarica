/*s: git9/proto.c */
#include <u.h>
#include <libc.h>
#include <ctype.h>

#include "git.h"

/*s: constant [[Useragent]] */
#define Useragent   "useragent git/2.24.1"
/*e: constant [[Useragent]] */
/*s: constant [[Contenthdr]] */
#define Contenthdr  "headers Content-Type: application/x-git-%s-pack-request"
/*e: constant [[Contenthdr]] */
/*s: constant [[Accepthdr]] */
#define Accepthdr   "headers Accept: application/x-git-%s-pack-result"
/*e: constant [[Accepthdr]] */

enum {
    Nproto  = 16,
    Nport   = 16,
    Nhost   = 256,
    Npath   = 128,
    Nbranch = 32,
};

/*s: function [[matchcap]] */
char *
matchcap(char *s, char *cap, int full)
{
    if(strncmp(s, cap, strlen(cap)) == 0)
        if(!full || strlen(s) == strlen(cap))
            return s + strlen(cap);
    return nil;
}
/*e: function [[matchcap]] */

/*s: function [[parsecaps]] */
void
parsecaps(char *caps, Conn *c)
{
    char *p, *n, *s, *t;

    for(p = caps; p != nil; p = n){
        n = strchr(p, ' ');
        if(n != nil)
            *n++ = 0;
        if(matchcap(p, "report-status", 1) != nil)
            c->report = 1;
        if(matchcap(p, "multi_ack", 1) != nil)
            c->multiack = 1;
        else if(matchcap(p, "side-band", 1) != nil)
            c->sideband = 1;
        else if(matchcap(p, "side-band-64k", 1) != nil)
            c->sideband64k = 1;
        else if((s = matchcap(p, "symref=", 0)) != nil){
            if((t = strchr(s, ':')) == nil)
                continue;
            *t++ = '\0';
            snprint(c->symfrom, sizeof(c->symfrom), s);
            snprint(c->symto, sizeof(c->symto), t);
        }
    }
}
/*e: function [[parsecaps]] */


/*s: function [[tracepkt]] */
void
tracepkt(int v, char *pfx, char *b, int n)
{
    char *f;
    int o, i;

    if(chattygit < v)
        return;
    o = 0;
    f = emalloc(n*4 + 1);
    for(i = 0; i < n; i++){
        if(isprint(b[i])){
            f[o++] = b[i];
            continue;
        }
        f[o++] = '\\';
        switch(b[i]){
        case '\\':  f[o++] = '\\';  break;
        case '\n':  f[o++] = 'n';   break;
        case '\r':  f[o++] = 'r';   break;
        case '\v':  f[o++] = 'v';   break;
        case '\0':  f[o++] = '0';   break;
        default:
            f[o++] = 'x';
            f[o++] = "0123456789abcdef"[(b[i]>>4)&0xf];
            f[o++] = "0123456789abcdef"[(b[i]>>0)&0xf];
            break;
        }
    }
    f[o] = '\0';
    fprint(2, "%s %04x:\t%s\n", pfx, n, f);
    free(f);
}
/*e: function [[tracepkt]] */

/*s: function [[readpkt]] */
int
readpkt(Conn *c, char *buf, int nbuf)
{
    char len[5];
    char *e;
    int n;

    if(readn(c->rfd, len, 4) != 4){
        werrstr("pktline: short read from transport");
        return -1;
    }
    len[4] = 0;
    n = strtol(len, &e, 16);
    if(n == 0){
        dprint(1, "=r=> 0000\n");
        return 0;
    }
    if(e != len + 4 || n <= 4)
        sysfatal("pktline: bad length '%s'", len);
    n  -= 4;
    if(n >= nbuf)
        abort();//sysfatal("pktline: undersize buffer");
    if(readn(c->rfd, buf, n) != n)
        return -1;
    if(n > 4 && strncmp(buf, "ERR ", 4) == 0){
        if((e = strrchr(buf, '\n')) != nil)
            *e = '\0';
        werrstr("%s", buf + 4);
        return -1;
    }
    buf[n] = 0;
    tracepkt(1, "=r=>", buf, n);
    return n;
}
/*e: function [[readpkt]] */
/*s: function [[writepkt]] */
int
writepkt(Conn *c, char *buf, int nbuf)
{
    char len[5];


    snprint(len, sizeof(len), "%04x", nbuf + 4);
    if(write(c->wfd, len, 4) != 4)
        return -1;
    if(write(c->wfd, buf, nbuf) != nbuf)
        return -1;
    tracepkt(1, "<=w=", buf, nbuf);
    return 0;
}
/*e: function [[writepkt]] */

/*s: function [[fmtpkt]] */
int
fmtpkt(Conn *c, char *fmt, ...)
{
    char pkt[Pktmax];
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = vsnprint(pkt, sizeof(pkt), fmt, ap);
    n = writepkt(c, pkt, n);
    va_end(ap);
    return n;
}
/*e: function [[fmtpkt]] */

/*s: function [[flushpkt]] */
int
flushpkt(Conn *c)
{
    dprint(1, "<=w= 0000\n");
    return write(c->wfd, "0000", 4);
}
/*e: function [[flushpkt]] */

/*s: function [[grab]] */
static void
grab(char *dst, int n, char *p, char *e)
{
    int l;

    l = e - p;
    if(l >= n)
        sysfatal("overlong component");
    memcpy(dst, p, l);
    dst[l] = 0;
}
/*e: function [[grab]] */

/*s: function [[parseuri]] */
static int
parseuri(char *uri, char *proto, char *host, char *port, char *path)
{
    char *s, *p, *q;
    int hasport;

    print("uri: \"%s\"\n", uri);

    p = strstr(uri, "://");
    if(p == nil)
        snprint(proto, Nproto, "ssh");
    else if(strncmp(uri, "git+", 4) == 0)
        grab(proto, Nproto, uri + 4, p);
    else
        grab(proto, Nproto, uri, p);
    *port = 0;
    hasport = 1;
    if(strcmp(proto, "git") == 0)
        snprint(port, Nport, "9418");
    else if(strncmp(proto, "https", 5) == 0)
        snprint(port, Nport, "443");
    else if(strncmp(proto, "http", 4) == 0)
        snprint(port, Nport, "80");
    else if(strncmp(proto, "hjgit", 5) == 0)
        snprint(port, Nport, "17021");
    else if(strncmp(proto, "gits", 5) == 0)
        snprint(port, Nport, "9419");
    else
        hasport = 0;
    s = (p != nil) ? p + 3 : uri;
    p = nil;
    if(!hasport){
        p = strstr(s, ":");
        if(p != nil)
            p++;
    }
    if(p == nil)
        p = strstr(s, "/");
    if(p == nil || strlen(p) == 1){
        werrstr("missing path");
        return -1;
    }

    q = memchr(s, ':', p - s);
    if(q){
        grab(host, Nhost, s, q);
        grab(port, Nport, q + 1, p);
    }else{
        grab(host, Nhost, s, p);
    }

    snprint(path, Npath, "%s", p);
    return 0;
}
/*e: function [[parseuri]] */

/*s: function [[webclone]] */
static int
webclone(Conn *c, char *url)
{
    char buf[16];
    int n, conn;

    if((c->cfd = open("/mnt/web/clone", ORDWR)) < 0)
        goto err;
    if((n = read(c->cfd, buf, sizeof(buf)-1)) == -1)
        goto err;
    buf[n] = 0;
    conn = atoi(buf);

    /* github will behave differently based on useragent */
    if(write(c->cfd, Useragent, sizeof(Useragent)) == -1)
        return -1;
    dprint(1, "open url %s\n", url);
    if(fprint(c->cfd, "url %s", url) == -1)
        goto err;
    free(c->dir);
    c->dir = smprint("/mnt/web/%d", conn);
    return 0;
err:
    if(c->cfd != -1)
        close(c->cfd);
    return -1;
}
/*e: function [[webclone]] */

/*s: function [[webopen]] */
static int
webopen(Conn *c, char *file, int mode)
{
    char path[128];
    int fd;

    snprint(path, sizeof(path), "%s/%s", c->dir, file);
    if((fd = open(path, mode)) == -1)
        return -1;
    return fd;
}
/*e: function [[webopen]] */

/*s: function [[issmarthttp]] */
static int
issmarthttp(Conn *c, char *direction)
{
    char buf[Pktmax+1], svc[128];
    int fd, n;

    if((fd = webopen(c, "contenttype", OREAD)) == -1)
        return -1;
    n = readn(fd, buf, sizeof(buf) - 1);
    close(fd);
    if(n == -1)
        return -1;
    buf[n] = '\0';
    snprint(svc, sizeof(svc), "application/x-git-%s-pack-advertisement", direction);
    if(strcmp(svc, buf) != 0){
        werrstr("dumb http protocol not supported");
        return -1;
    }

    if((n = readpkt(c, buf, sizeof(buf))) == -1)
        sysfatal("http read: %r");
    buf[n] = 0;
    snprint(svc, sizeof(svc), "# service=git-%s-pack\n", direction);
    if(strncmp(svc, buf, n) != 0){
        werrstr("invalid initial packet line");
        return -1;
    }
    if(readpkt(c, buf, sizeof(buf)) != 0){
        werrstr("protocol garble: expected flushpkt");
        return -1;
    }
    return 0;
}
/*e: function [[issmarthttp]] */

/*s: function [[dialhttp]] */
static int
dialhttp(Conn *c, char *host, char *port, char *path, char *direction)
{
    char *geturl, *suff, *hsep, *psep, *isep;

    suff = "";
    hsep = "";
    psep = "";
    isep = "";
    if(port && strlen(port) != 0)
        hsep = ":";
    if(path && path[0] != '/')
        psep = "/";
    if(path && path[0] && path[strlen(path)-1] != '/')
        isep = "/";
    memset(c, 0, sizeof(*c));
    geturl = smprint("https://%s%s%s%s%s%s%sinfo/refs?service=git-%s-pack",
        host, hsep, port, psep, path, suff, isep, direction);
    c->type = ConnHttp;
    c->url = smprint("https://%s%s%s%s%s%s%sgit-%s-pack",
        host, hsep, port, psep, path, suff, isep, direction);
    c->cfd = webclone(c, geturl);
    free(geturl);
    if(c->cfd == -1)
        return -1;
    c->rfd = webopen(c, "body", OREAD);
    c->wfd = -1;
    if(c->rfd == -1)
        return -1;
    if(issmarthttp(c, direction) == -1)
        return -1;
    c->direction = estrdup(direction);
    return 0;
}
/*e: function [[dialhttp]] */
/*s: function [[dialssh]] */
static int
dialssh(Conn *c, char *host, char *, char *path, char *direction)
{
    int pid, pfd[2];
    char cmd[64];

    if(pipe(pfd) == -1)
        sysfatal("unable to open pipe: %r");
    pid = fork();
    if(pid == -1)
        sysfatal("unable to fork");
    if(pid == 0){
        close(pfd[1]);
        dup(pfd[0], 0);
        dup(pfd[0], 1);
        snprint(cmd, sizeof(cmd), "git-%s-pack", direction);
        dprint(1, "exec ssh '%s' '%s' %s\n", host, cmd, path);
        execl("/bin/ssh", "ssh", host, cmd, path, nil);
        sysfatal("exec: %r");
    }
    close(pfd[0]);
    c->type = ConnSsh;
    c->rfd = pfd[1];
    c->wfd = dup(pfd[1], -1);
    return 0;
}
/*e: function [[dialssh]] */

/*s: function [[githandshake]] */
/// dialgit | servelocal -> <>
static errorneg1
githandshake(Conn *c, char *host, char *path, char *direction)
{
    char *p, *e, cmd[512];

    p = cmd;
    e = cmd + sizeof(cmd);
    p = seprint(p, e - 1, "git-%s-pack %s", direction, path);
    if(host != nil)
        p = seprint(p + 1, e, "host=%s", host);
    if(writepkt(c, cmd, p - cmd + 1) == -1){
        fprint(STDERR, "failed to write message\n");
        closeconn(c);
        return ERROR_NEG1;
    }
    return OK_0;
}
/*e: function [[githandshake]] */

/*s: function [[dialhjgit]] */
static int
dialhjgit(Conn *c, char *host, char *port, char *path, char *direction, int auth)
{
    char *ds;
    int pid, pfd[2];

    if((ds = netmkaddr(host, "tcp", port)) == nil)
        return -1;
    if(pipe(pfd) == -1)
        sysfatal("unable to open pipe: %r");
    pid = fork();
    if(pid == -1)
        sysfatal("unable to fork");
    if(pid == 0){
        close(pfd[1]);
        dup(pfd[0], 0);
        dup(pfd[0], 1);
        dprint(1, "exec tlsclient -a %s\n", ds);
        if(auth)
            execl("/bin/tlsclient", "tlsclient", "-a", ds, nil);
        else
            execl("/bin/tlsclient", "tlsclient", ds, nil);
        sysfatal("exec: %r");
    }
    close(pfd[0]);
    c->type = ConnGit9;
    c->rfd = pfd[1];
    c->wfd = dup(pfd[1], -1);
    return githandshake(c, host, path, direction);
}
/*e: function [[dialhjgit]] */

/*s: function [[initconn]] */
void
initconn(Conn *c, int rd, int wr)
{
    c->type = ConnGit;
    c->rfd = rd;
    c->wfd = wr;
}
/*e: function [[initconn]] */

/*s: function [[dialgit]] */
/// (main(get.c) | main(send.c)) -> gitconnect -> <>
static errorneg1
dialgit(Conn *c, char *host, char *port, char *path, char *direction)
{
    char *ds; // dial string
    fdt fd;

    ds = netmkaddr(host, "tcp", port);
    /*s: [[dialgit()]] sanity check [[ds]] */
    if(ds == nil)
        return ERROR_NEG1;
    /*e: [[dialgit()]] sanity check [[ds]] */
    dprint(1, "dial %s git-%s-pack %s\n", ds, direction, path);
    fd = dial(ds, nil, nil, nil);
    /*s: [[dialgit()]] sanity check [[fd]] */
    if(fd == -1)
        return ERROR_NEG1;
    /*e: [[dialgit()]] sanity check [[fd]] */
    c->type = ConnGit;
    c->rfd = fd;
    c->wfd = dup(fd, -1);
    return githandshake(c, host, path, direction);
}
/*e: function [[dialgit]] */

/*s: function [[servelocal]] */
/// gitconnect -> <>
static int
servelocal(Conn *c, char *path, char *direction)
{
    int pid;
    fdt pfd[2];

    if(pipe(pfd) == ERROR_NEG1)
        sysfatal("unable to open pipe: %r");
    pid = fork();
    /*s: [[servelocal()]] sanity check [[pid]] */
    if(pid == -1)
        sysfatal("unable to fork");
    /*e: [[servelocal()]] sanity check [[pid]] */
    if(pid == 0){
        // children
        close(pfd[1]);
        dup(pfd[0], 0);
        dup(pfd[0], 1);
        execl("/bin/git/serve", "serve", "-w", nil);
        sysfatal("exec: %r");
    }
    // else parent
    close(pfd[0]);
    c->type = ConnGit;
    c->rfd = pfd[1];
    c->wfd = dup(pfd[1], -1);
    return githandshake(c, nil, path, direction);
}
/*e: function [[servelocal]] */

/*s: function [[localrepo]] */
static errorneg1
localrepo(char *uri, char *path, int npath)
{
    fdt fd;

    snprint(path, npath, "%s/.git/../", uri);
    fd = open(path, OREAD);
    if(fd < 0)
        return ERROR_NEG1;
    if(fd2path(fd, path, npath) != 0){
        close(fd);
        return ERROR_NEG1;
    }
    close(fd);
    return OK_0;
}
/*e: function [[localrepo]] */

/*s: function [[gitconnect]] */
/// main(get.c) | main(send.c) -> <>
errorneg1
gitconnect(Conn *c, char *uri, char *direction)
{
    char path[Npath];
    /*s: [[gitconnect()]] other locals */
    char proto[Nproto], host[Nhost], port[Nport];
    /*e: [[gitconnect()]] other locals */

    memset(c, 0, sizeof(Conn));
    c->rfd = c->wfd = c->cfd = -1;

    if(localrepo(uri, path, sizeof(path)) == OK_0)
        return servelocal(c, path, direction);
    // else
    /*s: [[gitconnect()]] when [[uri]] not local repo */
    if(parseuri(uri, proto, host, port, path) == ERROR_NEG1){
        werrstr("bad uri %s", uri);
        return ERROR_NEG1;
    }
    if(strcmp(proto, "git") == 0)
        return dialgit(c, host, port, path, direction);
    /*s: [[gitconnect()]] when [[uri]] not local repo, if series on [[proto]] cases */
    else if(strcmp(proto, "http") == 0 || strcmp(proto, "https") == 0)
        return dialhttp(c, host, port, path, direction);
    /*x: [[gitconnect()]] when [[uri]] not local repo, if series on [[proto]] cases */
    else if(strcmp(proto, "ssh") == 0)
        return dialssh(c, host, port, path, direction);
    /*x: [[gitconnect()]] when [[uri]] not local repo, if series on [[proto]] cases */
    else if(strcmp(proto, "hjgit") == 0)
        return dialhjgit(c, host, port, path, direction, 1);
    /*x: [[gitconnect()]] when [[uri]] not local repo, if series on [[proto]] cases */
    else if(strcmp(proto, "gits") == 0)
        return dialhjgit(c, host, port, path, direction, 0);
    /*e: [[gitconnect()]] when [[uri]] not local repo, if series on [[proto]] cases */
    // else
    werrstr("unknown protocol %s", proto);
    return ERROR_NEG1;
    /*e: [[gitconnect()]] when [[uri]] not local repo */
}
/*e: function [[gitconnect]] */

/*s: function [[writephase]] */
int
writephase(Conn *c)
{
    char hdr[128];
    int n;

    dprint(1, "start write phase\n");
    if(c->type != ConnHttp)
        return 0;

    if(c->wfd != -1)
        close(c->wfd);
    if(c->cfd != -1)
        close(c->cfd);
    if((c->cfd = webclone(c, c->url)) == -1)
        return -1;
    n = snprint(hdr, sizeof(hdr), Contenthdr, c->direction);
    if(write(c->cfd, hdr, n) == -1)
        return -1;
    n = snprint(hdr, sizeof(hdr), Accepthdr, c->direction);
    if(write(c->cfd, hdr, n) == -1)
        return -1;
    if((c->wfd = webopen(c, "postbody", OWRITE)) == -1)
        return -1;
    c->rfd = -1;
    return 0;
}
/*e: function [[writephase]] */
/*s: function [[readphase]] */
int
readphase(Conn *c)
{
    dprint(1, "start read phase\n");
    if(c->type != ConnHttp)
        return 0;
    if(close(c->wfd) == -1)
        return -1;
    if((c->rfd = webopen(c, "body", OREAD)) == -1)
        return -1;
    c->wfd = -1;
    return 0;
}
/*e: function [[readphase]] */

/*s: function [[closeconn]] */
void
closeconn(Conn *c)
{
    close(c->rfd);
    close(c->wfd);
    switch(c->type){
    /*s: [[closeconn()]] switch [[c->type]] cases */
    case ConnGit:
        break;
    /*x: [[closeconn()]] switch [[c->type]] cases */
    case ConnHttp:
        close(c->cfd);
        break;
    /*x: [[closeconn()]] switch [[c->type]] cases */
    case ConnGit9:
    case ConnSsh:
        free(wait());
        break;
    /*e: [[closeconn()]] switch [[c->type]] cases */
    }
}
/*e: function [[closeconn]] */
/*e: git9/proto.c */
