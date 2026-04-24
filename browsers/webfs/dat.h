/*s: webfs/dat.h */
typedef struct Client Client;
typedef struct Ctl Ctl;
typedef struct Ibuf Ibuf;
typedef struct Url Url;

/*s: struct [[Ibuf]](webfs) */
/* simple buffered i/o for network connections; shared by http, ftp */
struct Ibuf
{
    fdt fd;
    Ioproc *io;
    char buf[4096];
    // read and write pointer in buf
    char *rp, *wp;
};
/*e: struct [[Ibuf]](webfs) */

/*s: struct [[Ctl]](webfs) */
struct Ctl
{
    bool acceptcookies;
    bool sendcookies;
    int redirectlimit;
    char *useragent;
};
/*e: struct [[Ctl]](webfs) */
/*s: struct [[Client]](webfs) */
struct Client
{
    // URls and control settings
    Url *url;
    Url *baseurl;
    Ctl ctl;

    // filled once the response arrives
    char *contenttype;
    char *postbody;
    char *redirect;
    char *authenticate;

    // Plumbing
    Channel *creq;      /* chan(Req*) */
    Ioproc *io;
    int iobusy;
    int ref;

    /*s: [[Client]](webfs) other fields */
    int num;
    int plumbed;

    char *ext;
    int npostbody;
    int havepostbody;
    int bodyopened;
    void *aux;
    /*e: [[Client]](webfs) other fields */
};
/*e: struct [[Client]](webfs) */

/*s: enum [[UScheme]] */
/*
 * If ischeme is USunknown, then the given URL is a relative
 * URL which references the "current document" in the context of the base.
 * If this is the case, only the "fragment" and "url" members will have
 * meaning, and the given URL structure may not be used as a base URL itself.
 */
enum UScheme
{
    USunknown,

    UShttp,
    UShttps,
    USftp,
    USfile,

    UScurrent,
};
/*e: enum [[UScheme]] */
/*s: struct [[Url]](webfs) */
struct Url
{
    // ref_own<string>, the full URL string
    char*       url;

    // the scheme://[user[:password]@]host[:port]/path[?query][#fragment] components
    char*       scheme;
    char*       user;
    char*       passwd;
    char*       host;
    char*       port;
    char*       path;
    char*       query;
    char*       fragment;

    // enum<USCheme>
    int         ischeme;
    /*s: [[Url]](webfs) per-scheme methods */
    int         (*open)(Client*, Url*);
    int         (*read)(Client*, Req*);
    void        (*close)(Client*);
    /*e: [[Url]](webfs) per-scheme methods */
    union {
        /*s: [[Url]](webfs) union members */
        struct {
            char *page_spec;
        } http;
        /*x: [[Url]](webfs) union members */
        struct {
            char *path_spec;
            char *type;
        } ftp;
        /*e: [[Url]](webfs) union members */
    };

    /*s: [[Url]](webfs) other fields */
    char*       schemedata;
    char*       authority;
    /*e: [[Url]](webfs) other fields */
};
/*e: struct [[Url]](webfs) */

enum
{
    STACK = 32*1024,  /* was 16*1024; there are big arrays on the stack */
};

extern  Client**        client;
extern  int             cookiedebug;
extern  Srv             fs;
extern  int             fsdebug;
extern  Ctl             globalctl;
extern  int             nclient;
extern  int             urldebug;
extern  int             httpdebug;
extern  char*   status[];

/*e: webfs/dat.h */
