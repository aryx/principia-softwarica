/*s: kernel/devices/screen/devdraw.h */

typedef struct Client Client;
typedef struct KDraw KDraw;
typedef struct DImage DImage;
typedef struct DName DName;
typedef struct DScreen DScreen;
typedef struct CScreen CScreen;
typedef struct FChar FChar;
typedef struct Refresh Refresh;
typedef struct Refx Refx;

/*s: constant NHASH bis */
#define NHASH       (1<<5)
/*e: constant NHASH bis */
/*s: constant [[HASHMASK]] */
#define HASHMASK    (NHASH-1)
/*e: constant [[HASHMASK]] */


/*s: struct [[KDraw]] */
struct KDraw
{
    // growing_array<option<Client>> (size = KDraw.nclient)
    Client**    client;
    int     nclient;

    // gensym
    int     clientid;
    
    /*s: [[KDraw]] other fields */
    bool     softscreen;
    /*x: [[KDraw]] other fields */
    // growing_array<DName>, size = KDraw.nname
    DName*  name;
    int     nname;
    /*x: [[KDraw]] other fields */
    int     vers;
    /*x: [[KDraw]] other fields */
    bool	blanked;    /* screen turned off */
    /*x: [[KDraw]] other fields */
    ulong   savemap[3*256];
    /*x: [[KDraw]] other fields */
    ulong   blanktime;  /* time of last operation */
    /*e: [[KDraw]] other fields */
};
/*e: struct [[KDraw]] */

/*s: struct [[Client]] */
struct Client
{
    int     clientid; // /dev/draw/x/

    // hash<Image.id, ref_own<Dimage>> (next = Dimage.next in bucket)
    DImage*     dimage[NHASH];

    /*s: [[Client]] drawing state fields */
    // enum<Drawop>
    int     op;
    /*e: [[Client]] drawing state fields */
    /*s: [[Client]] layer fields */
    // list<ref_own<DScreen>> (next = CScreen.next)
    CScreen*    cscreen;
    /*e: [[Client]] layer fields */
    /*s: [[Client]] other fields */
    // index in KDraw.clients[]
    int     slot;
    /*x: [[Client]] other fields */
    int     infoid;
    /*x: [[Client]] other fields */
    // array<byte> (size >= Client.nreaddata)
    byte*   readdata;
    int     nreaddata;
    /*x: [[Client]] other fields */
    Refresh*    refresh;
    Rendez      refrend;
    int     refreshme;
    /*e: [[Client]] other fields */

    // Extra
    /*s: [[Client]] concurrency fields */
    bool     busy;
    /*e: [[Client]] concurrency fields */
    Ref     r;
};
/*e: struct [[Client]] */


/*s: struct [[DName]] */
struct DName
{
    // key
    char        *name;

    // value
    Client      *client; // the owner
    DImage*     dimage;
    /*s: [[DName]] other fields */
    int     vers;
    /*e: [[DName]] other fields */
};
/*e: struct [[DName]] */


/*s: struct [[DImage]] */
/*
 * Reference counts in DImages:
 *  one per open by original client
 *  one per screen image or fill
 *  one per image derived from this one by name
 */
struct DImage
{
    int     id;
    Memimage*   image;

    /*s: [[DImage]] layer fields */
    DScreen*    dscreen;    /* nil if not a window */
    /*e: [[DImage]] layer fields */
    /*s: [[DImage]] font fields */
    // growing_hash<Rune, ref_own<Fchar>> (size = DImage.nfchar)
    FChar*      fchar;
    int     nfchar;
    /*x: [[DImage]] font fields */
    int     ascent;
    /*e: [[DImage]] font fields */
    /*s: [[DImage]] other fields */
    // option<string>, Some when DImage derives from a named image
    char    *name;
    // option<ref<DImage>>
    DImage*     fromname;   /* image this one is derived from, by name */
    /*x: [[DImage]] other fields */
    int     vers;
    /*e: [[DImage]] other fields */

    // Extra
    /*s: [[DImage]] extra fields */
    DImage*     next;
    /*e: [[DImage]] extra fields */
    int     ref;
};
/*e: struct [[DImage]] */

/*s: struct [[FChar]] */
struct FChar
{
    // Rectangle in Font.cacheimage
    int     minx;   /* left edge of bits */
    int     maxx;   /* right edge of bits */
    uchar   miny;   /* first non-zero scan-line */
    uchar   maxy;   /* last non-zero scan-line + 1 */

    schar   left;   /* offset of baseline */
    uchar   width;  /* width of baseline */
};
/*e: struct [[FChar]] */



/*s: struct [[CScreen]] */
struct CScreen
{
    // ref_shared<DScreen>
    DScreen*    dscreen;

    CScreen*    next;
};
/*e: struct [[CScreen]] */

/*s: struct [[DScreen]] */
struct DScreen
{
    int     id;

    DImage      *dimage;
    DImage      *dfill;

    /*s: [[DScreen]] other fields */
    bool     public;
    /*x: [[DScreen]] other fields */
    Client*     owner;
    /*x: [[DScreen]] other fields */
    // ref_own<Memscreen>
    Memscreen*  screen;
    /*e: [[DScreen]] other fields */

    // Extra
    int     ref;
    /*s: [[DScreen]] extra fields */
    // list<ref<DScreen>> (head = dscreen)
    DScreen*    next;
    /*e: [[DScreen]] extra fields */
};
/*e: struct [[DScreen]] */


/*s: struct [[Refresh]] */
struct Refresh
{
    DImage*     dimage;
    Rectangle   r;
    Refresh*    next;
};
/*e: struct [[Refresh]] */

/*s: struct [[Refx]] */
struct Refx
{
    Client*     client;
    DImage*     dimage;
};
/*e: struct [[Refx]] */

// drawerror.c
extern char Enodrawimage[];
extern char Enodrawscreen[];
extern char Eshortdraw[];
extern char Eshortread[];
extern char Eimageexists[];
extern char Escreenexists[];
extern char Edrawmem[];
extern char Ereadoutside[];
extern char Ewriteoutside[];
extern char Enotfont[];
extern char Eindex[];
extern char Enoclient[];
extern char Enameused[];
extern char Enoname[];
extern char Eoldname[];
extern char Enamed[];
extern char Ewrongname[];

//drawinit.c
extern Memimage    *screenimage;
error0 initscreenimage(void);

//devdraw.c
extern KDraw        sdraw;
void dlock(void);
int candlock(void);
void dunlock(void);
DImage* drawlookup(Client *client, int id, bool checkname);
void addflush(Rectangle r);
void dstflush(int dstid, Memimage *dst, Rectangle r);
void drawflush(void);

//drawalloc.c
DImage* allocdimage(Memimage *i);
void drawfreedimage(DImage *dimage);
Memimage* drawinstall(Client *client, int id, Memimage *i, DScreen *dscreen);
void drawuninstall(Client *client, int id);

//drawname.c
extern char    screenname[];
DName* drawlookupname(int n, char *str);
bool drawgoodname(DImage *d);
void drawaddname(Client *client, DImage *di, int n, char *str);
void drawdelname(DName *name);

//drawwindow.c
DScreen*   drawlookupscreen(Client *client, int id, CScreen **cs);
Memscreen* drawinstallscreen(Client*, DScreen*, int, DImage *, DImage *, bool);
void       drawuninstallscreen(Client*, CScreen*);
DScreen* drawlookupdscreen(int id);
void drawfreedscreen(DScreen*);

//drawmisc.c
int drawrefactive(void *a);
void drawrefresh(Memimage*, Rectangle r, void *v);
void drawrefreshscreen(DImage *l, Client *client);

/*e: kernel/devices/screen/devdraw.h */
