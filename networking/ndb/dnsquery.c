/*s: networking/ndb/dnsquery.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>
#include <ndb.h>
#include "dns.h"
#include "ip.h"

/*s: global [[domount]] */
static int domount;
/*e: global [[domount]] */
static char *mtpt, *dns, *srv;

/*s: function [[setup]]([[(networking/ndb/dnsquery.c)]]) */
static int
setup(int argc, char **argv)
{
    int fd;

    if(argc == 1){
        domount = 0;
        mtpt = argv[0];
    }

    fd = open(dns, ORDWR);
    if(fd < 0){
        if(domount == 0)
            sysfatal("can't open %s: %r", dns);
        fd = open(srv, ORDWR);
        if(fd < 0)
            sysfatal("can't open %s: %r", srv);
        if(mount(fd, -1, mtpt, MBEFORE, "") < 0)
            sysfatal("can't mount(%s, %s): %r", srv, mtpt);
        fd = open(dns, ORDWR);
        if(fd < 0)
            sysfatal("can't open %s: %r", dns);
    }
    return fd;
}
/*e: function [[setup]]([[(networking/ndb/dnsquery.c)]]) */

/*s: function [[querydns]] */
static void
querydns(int fd, char *line, int n)
{
    char buf[1024];

    seek(fd, 0, 0);
    if(write(fd, line, n) != n) {
        print("!%r\n");
        return;
    }
    seek(fd, 0, 0);
    buf[0] = '\0';
    while((n = read(fd, buf, sizeof(buf))) > 0){
        buf[n] = '\0';
        print("%s\n", buf);
    }
}
/*e: function [[querydns]] */

/*s: function [[query]]([[(networking/ndb/dnsquery.c)]]) */
static void
query(int fd)
{
    int n, len;
    char *lp, *p, *np;
    char buf[1024], line[1024];
    Biobuf in;

    Binit(&in, 0, OREAD);
    for(print("> "); lp = Brdline(&in, '\n'); print("> ")){
        n = Blinelen(&in) -1;
        while(isspace(lp[n]))
            lp[n--] = 0;
        n++;
        while(isspace(*lp)){
            lp++;
            n--;
        }
        if(!*lp)
            continue;
        strcpy(line, lp);

        /* default to an "ip" request if alpha, "ptr" if numeric */
        if(strchr(line, ' ') == nil)
            if(strcmp(ipattr(line), "ip") == 0) {
                strcat(line, " ptr");
                n += 4;
            } else {
                strcat(line, " ip");
                n += 3;
            }

        /* inverse queries may need to be permuted */
        if(n > 4 && strcmp(" ptr", &line[n-4]) == 0 &&
            cistrstr(line, ".arpa") == nil){
            /* TODO: reversing v6 addrs is harder */
            for(p = line; *p; p++)
                if(*p == ' '){
                    *p = '.';
                    break;
                }
            np = buf;
            len = 0;
            while(p >= line){
                len++;
                p--;
                if(*p == '.'){
                    memmove(np, p+1, len);
                    np += len;
                    len = 0;
                }
            }
            memmove(np, p+1, len);
            np += len;
            strcpy(np, "in-addr.arpa ptr");	/* TODO: ip6.arpa for v6 */
            strcpy(line, buf);
            n = strlen(line);
        }

        querydns(fd, line, n);
    }
    Bterm(&in);
}
/*e: function [[query]]([[(networking/ndb/dnsquery.c)]]) */

/*s: function [[main]]([[(networking/ndb/dnsquery.c)]]) */
void
main(int argc, char *argv[])
{
    mtpt = "/net";
    dns = "/net/dns";
    srv = "/srv/dns";
    domount = 1;
    ARGBEGIN {
    case 'x':
        mtpt = "/net.alt";
        dns = "/net.alt/dns";
        srv = "/srv/dns_net.alt";
        break;
    default:
        fprint(2, "usage: %s [-x] [dns-mount-point]\n", argv0);
        exits("usage");
    } ARGEND;

    query(setup(argc, argv));
    exits(0);
}
/*e: function [[main]]([[(networking/ndb/dnsquery.c)]]) */
/*e: networking/ndb/dnsquery.c */
