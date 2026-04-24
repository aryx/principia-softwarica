/*s: mothra/mothra.h */
enum{
    NWWW=64,    /* # of pages we hold in the log */
    NXPROC=5,   /* # of parallel procs loading the pix */
    NPIXMB=8,   /* megabytes of image data to keep arround */
    NNAME=512,
    NLINE=256,
    NAUTH=128,
    NTITLE=81,  /* length of title (including nul at end) */
    NLABEL=50,  /* length of option name in forms */
    NREDIR=10,  /* # of redirections we'll tolerate before declaring a loop */
};

typedef struct Action Action;
typedef struct Url Url;
typedef struct Www Www;
typedef struct Field Field;

/*s: struct [[Action]](mothra) */
struct Action{

    // for <img src=... >
    char *image;
    // for <input ...>
    Field *field;
    // for <a href=... >
    char *link;
    // anchor #fragment
    char *name;

    int ismap;

    int width;
    int height;
};
/*e: struct [[Action]](mothra) */

/*s: struct [[Url]](mothra) */
struct Url{
    char *basename;
    char *reltext;
    char fullname[NNAME];
    char tag[NNAME];
    char contenttype[NNAME];
    int map;            /* is this an image map? */
};
/*e: struct [[Url]](mothra) */

/*s: struct [[Www]](mothra) */
struct Www{

    //ref_own<Url>
    Url *url;
    // list<Rtext>, next= ?
    Rtext *text;
    char title[NTITLE];

    // list<Pix>, next = Pix.next, image cache
    void *pix;
    // form state, type=??
    void *form;

    bool changed;               /* reader sets this every time it updates page */
    int finished;               /* reader sets this when done */
    int alldone;                /* page will not change further -- used to adjust cursor */

    /*s: [[Www]] other fields */
    int yoffs;
    int gottitle;               /* title got drawn */
    /*e: [[Www]] other fields */
};
/*e: struct [[Www]](mothra) */

/*s: enum [[XKind]](mothra) */
enum{
    PLAIN,
    HTML,

    GIF,
    JPEG,
    PNG,
    BMP,
    ICO,

    PAGE,
};
/*e: enum [[XKind]](mothra) */

/*s: enum [[Axxx]](mothra) */
/*
 *  authentication types
 */
enum{
    ANONE,
    ABASIC,
};
/*e: enum [[Axxx]](mothra) */

/*s: global images (mothra) */
Image *hrule, *bullet, *linespace;
/*e: global images (mothra) */
/*s: global [[chrwidth]](mothra) */
int chrwidth;           /* nominal width of characters in font */
/*e: global [[chrwidth]](mothra) */

/*s: global [[text]](mothra) */
Panel *text;            /* Panel displaying the current www page */
/*e: global [[text]](mothra) */

/*s: global [[debug]](mothra) */
bool debug;             /* command line flag */
/*e: global [[debug]](mothra) */

/*s: enum [[HttpMethod]](mothra) */
/*
 * HTTP methods
 */
enum{
    GET=1,
    POST,
};
/*e: enum [[HttpMethod]](mothra) */

void finish(Www *w);
void plrdhtml(char *, int, Www *, int);
void plrdplain(char *, int, Www *);
void htmlerror(char *, int, char *, ...);       /* user-supplied routine */
void seturl(Url *, char *, char *);
void freeurl(Url *);
Url *selurl(char *);
void getpix(Rtext *, Www *);
ulong countpix(void *p);
void freepix(void *p);
void dupfds(int fd, ...);
int pipeline(int fd, char *fmt, ...);
void getfonts(void);
void *emalloc(int);
void nstrcpy(char *to, char *from, int len);
void freeform(void *p);
int Ufmt(Fmt *f);
#pragma varargck type "U" char*
void message(char *, ...);
int filetype(int, char *, int);
int mimetotype(char *);
int snooptype(int);
void mkfieldpanel(Rtext *);
void geturl(char *, int, int, int);
char *urlstr(Url *);
int urlpost(Url*, char*);
int urlget(Url*, int);
int urlresolve(Url *);

/*s: global [[mouse]](mothra) */
Mouse mouse;
/*e: global [[mouse]](mothra) */
/*e: mothra/mothra.h */
