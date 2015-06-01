/*s: networking/ip/wol.c */
/* send wake-on-lan magic ethernet packet */
#include <u.h>
#include <libc.h>
#include <ip.h>

/*s: enum _anon_ (networking/ip/wol.c) */
enum {
    Eaddrlen = 6,	/* 48 bits */
};
/*e: enum _anon_ (networking/ip/wol.c) */

typedef struct Wolpack Wolpack;
/*s: struct Wolpack */
struct Wolpack{
    uchar	magic[6];
    uchar	macs[16][Eaddrlen];
    char	pass[6+1];
};
/*e: struct Wolpack */

/*s: global verbose */
int verbose;
/*e: global verbose */

/*s: function usage (networking/ip/wol.c) */
void
usage(void)
{
    fprint(2, "usage: wol [-v] [-a dialstr] [-c password] macaddr\n");
    exits("usage");
}
/*e: function usage (networking/ip/wol.c) */

/*s: function fillmac */
void
fillmac(Wolpack *w, uchar *mac)
{
    int i;

    for(i = 0; i < nelem(w->macs); i++)
        memmove(w->macs[i], mac, Eaddrlen);
}
/*e: function fillmac */

/*s: function dumppack */
void
dumppack(Wolpack *w)
{
    int i;

    print("packet: [\n");
    print("\t%E\n", w->magic);
    for(i = 0; i < nelem(w->macs); i++)
        print("\t%E\n", w->macs[i]);
    print("\t%6s\n", w->pass);
    print("]\n");
}
/*e: function dumppack */

/*s: function main (networking/ip/wol.c) */
void
main(int argc, char* argv[])
{
    int fd, nw;
    char *argmac, *pass, *address;
    uchar mac[Eaddrlen];
    static Wolpack w = {
        .magic = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, }
    };

    address = pass = nil;
    fmtinstall('E', eipfmt);

    ARGBEGIN{
    case 'a':
        address = EARGF(usage());
        break;
    case 'c':
        pass = EARGF(usage());
        break;
    case 'v':
        verbose++;
        break;
    default:
        usage();
    }ARGEND

    if(argc != 1)
        usage();
    argmac = argv[0];
    if(verbose)
        print("mac is %s, pass is %s\n", argmac, pass);

    parseether(mac, argmac);
    fillmac(&w, mac);
    if(pass){
        if(strlen(pass) > 6)
            sysfatal("password greater than 6 bytes");
        strcpy(w.pass, pass);
    }
    if(verbose)
        dumppack(&w);

    if(!address)
        address = "udp!255.255.255.255!0";

    fd = dial(address, nil, nil, nil);
    if(fd < 0)
        sysfatal("%s: %r", address);
    nw = write(fd, &w, sizeof w);
    if(nw != sizeof w)
        sysfatal("error sending: %r");
    exits(0);
}
/*e: function main (networking/ip/wol.c) */
/*e: networking/ip/wol.c */
