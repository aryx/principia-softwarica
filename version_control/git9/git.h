/*s: git9/git.h */
#include <bio.h>
#include <mp.h>
#include <libsec.h>
#include <flate.h>
#include <regexp.h>

typedef struct Conn Conn;
typedef struct Hash Hash;
typedef struct Delta    Delta;
typedef struct Cinfo    Cinfo;
typedef struct Tinfo    Tinfo;
typedef struct Object   Object;
typedef struct Objset   Objset;
typedef struct Pack Pack;
typedef struct Buf  Buf;
typedef struct Dirent   Dirent;
typedef struct Idxent   Idxent;
typedef struct Objlist  Objlist;
typedef struct Dtab Dtab;
typedef struct Dblock   Dblock;
typedef struct Objq Objq;
typedef struct Qelt Qelt;
typedef struct Idxent   Idxent;

enum {
    Pathmax     = 512,
    /*s: constant [[Npackcache]] */
    Npackcache  = 32,
    /*e: constant [[Npackcache]] */
    Hashsz      = 20,
    Pktmax      = 65536,

    KiB     = 1024,
    MiB     = 1024*KiB,
};

/*s: enum [[Gxxx]] */
enum Gxxx {
    GNone   = 0,

    GCommit = 1,
    GTree   = 2,
    GBlob   = 3,

    /*s: [[Gxxx]] other cases */
    GOdelta = 6,
    GRdelta = 7,
    /*x: [[Gxxx]] other cases */
    GTag    = 4,
    /*e: [[Gxxx]] other cases */
};
/*e: enum [[Gxxx]] */
/*s: enum [[Cxxx]] */
enum Cxxx {
    Cloaded = 1 << 0,
    Cparsed = 1 << 2,
    /*s: [[Cxxx]] other cases */
    Ccache  = 1 << 4,
    /*x: [[Cxxx]] other cases */
    Cexist  = 1 << 5,
    /*x: [[Cxxx]] other cases */
    Cthin   = 1 << 6,
    /*x: [[Cxxx]] other cases */
    Cidx    = 1 << 3,
    /*e: [[Cxxx]] other cases */
};
/*e: enum [[Cxxx]] */

/*s: enum [[Connxxx]] */
enum {
    ConnGit,
    /*s: [[Connxxx]] other cases */
    ConnHttp,
    /*x: [[Connxxx]] other cases */
    ConnSsh,
    /*x: [[Connxxx]] other cases */
    ConnGit9,
    /*e: [[Connxxx]] other cases */
};
/*e: enum [[Connxxx]] */

/*s: struct [[Objlist]] */
struct Objlist {
    int idx;

    // option<fdt> (None = -1)
    fdt fd;
    // ??
    int state;
    int stage;

    // .git/objects/
    // array<ref_own<Dir>> (len = ntop)
    Dir *top;
    int ntop;
    int topidx;

    // ??
    // array<ref_own<Dir>> (len = nloose)
    Dir *loose;
    int nloose;
    int looseidx;

    // .git/objects/pack/
    // option<array<ref_own<Dir>> (len = npack)
    Dir *pack;
    int npack;
    int packidx;

    int nent;
    int entidx;
};
/*e: struct [[Objlist]] */

/*s: struct [[Hash]] */
struct Hash {
    byte h[20];
};
/*e: struct [[Hash]] */

/*s: struct [[Conn]] */
struct Conn {
    // enum<Connxxx>
    int type;

    fdt rfd;
    fdt wfd;

    /*s: [[Conn]] capability fields */
    char    symfrom[256];
    char    symto[256];
    char    multiack;
    char    sideband;
    char    sideband64k;
    char    report;
    /*e: [[Conn]] capability fields */
    /*s: [[Conn]] http fields */
    /* only used by http */
    fdt cfd;
    char *url;  /* note, first GET uses a different url */
    char *dir;
    char *direction;
    /*e: [[Conn]] http fields */
};
/*e: struct [[Conn]] */

/*s: struct [[Dirent]] */
struct Dirent {
    // ref_own<string>
    char *name;
    int mode;
    // hash of GBlob (file) or GTree (dir) but not GCommit
    Hash h;
    /*s: [[Dirent]] other fields */
    bool_byte islink;
    /*x: [[Dirent]] other fields */
    bool_byte ismod;
    /*e: [[Dirent]] other fields */
};
/*e: struct [[Dirent]] */

/*s: struct [[Object]] */
struct Object {
    /* Git data */
    Hash    hash;
    // enum<Gxxx>
    int type;

    /*s: [[Object]] other fields */
    // shared objects
    int refs;
    /*e: [[Object]] other fields */
    /*s: [[Object]] cache fields */
    // enum<Cxxx>
    int flag;
    /*x: [[Object]] cache fields */
    int id;
    /*e: [[Object]] cache fields */
    /*s: [[Object]] indexing fields */
    vlong   off;
    vlong   len;
    u32int  crc;
    /*e: [[Object]] indexing fields */
    /*s: [[Object]] extra fields */
    Object  *next;
    Object  *prev;
    /*e: [[Object]] extra fields */

    /* Everything below here gets cleared */

    // raw object content after decompression
    // option<ref_own<string>>, set after Cloaded
    char    *all;
    // ref_shared<string> point in Object.all after object header (used for GBlob too)
    char    *data;
    /* size excludes header */
    vlong   size;

    // those fields are set after Cparsed
    /* Significant win on memory use */
    union {
        /*s: [[Object]] union fields */
        // when GTree
        Tinfo   *tree;
        /*x: [[Object]] union fields */
        // when GCommit
        Cinfo   *commit;
        /*e: [[Object]] union fields */
    };
};
/*e: struct [[Object]] */

/*s: struct [[Tinfo]] */
struct Tinfo {
    /* Tree */
    // array<ref_own<Dirent>> len = nent
    Dirent  *ent;
    int nent;
};
/*e: struct [[Tinfo]] */
/*s: struct [[Cinfo]] */
struct Cinfo {
    /* Commit */
    // array<Hash> (0, 1, or 2 parents?)
    Hash    *parent;
    int nparent;

    Hash    tree;

    // ref_own<string>
    char    *author;
    // option<ref_own<string>>
    char    *committer;

    // ref<string> (len = nmsg), point in Object.all?
    char    *msg;
    int nmsg;

    vlong   ctime;
    vlong   mtime;
};
/*e: struct [[Cinfo]] */

/*s: struct [[Objset]] */
struct Objset {
    // hash_array<Hash, ref<Object>>, used = nobj (allocated = sz)
    Object  **obj;
    int nobj;
    int sz;
};
/*e: struct [[Objset]] */

/*s: struct [[Qelt]] */
struct Qelt {
    // ref<Object>
    Object  *o;
    vlong   ctime;
    //enum<Qcolor>
    int color;
};
/*e: struct [[Qelt]] */
/*s: struct [[Objq]] */
struct Objq {
    // growing_array<Qelt> (used = nheap, allocated = heapsz)
    Qelt    *heap;
    int nheap;
    int heapsz;
};
/*e: struct [[Objq]] */

/*s: struct [[Dtab]] */
struct Dtab {
    Object  *o;
    uchar   *base;
    int nbase;

    Dblock  *b;
    int nb;
    int sz;
};
/*e: struct [[Dtab]] */
/*s: struct [[Dblock]] */
struct Dblock {
    uchar   *buf;
    int len;
    int off;
    u64int  hash;
};
/*e: struct [[Dblock]] */
/*s: struct [[Delta]] */
struct Delta {
    int cpy;
    int off;
    int len;
};
/*e: struct [[Delta]] */

/*s: struct [[Idxent]] */
struct Idxent {
    // ref_own<string>
    char    *path;
    Qid qid;
    int mode;
    // global counter (used in idxcmp())
    int order;
    // RMAUT?
    char    state;
};
/*e: struct [[Idxent]] */

/*s: macro [[GETBE32]] */
#define GETBE32(b)\
        ((((b)[0] & 0xFFul) << 24) | \
         (((b)[1] & 0xFFul) << 16) | \
         (((b)[2] & 0xFFul) <<  8) | \
         (((b)[3] & 0xFFul) <<  0))
/*e: macro [[GETBE32]] */
/*s: macro [[GETBE64]] */
#define GETBE64(b)\
        ((((b)[0] & 0xFFull) << 56) | \
         (((b)[1] & 0xFFull) << 48) | \
         (((b)[2] & 0xFFull) << 40) | \
         (((b)[3] & 0xFFull) << 32) | \
         (((b)[4] & 0xFFull) << 24) | \
         (((b)[5] & 0xFFull) << 16) | \
         (((b)[6] & 0xFFull) <<  8) | \
         (((b)[7] & 0xFFull) <<  0))
/*e: macro [[GETBE64]] */

/*s: macro [[PUTBE32]] */
#define PUTBE32(b, n)\
    do{ \
        (b)[0] = (n) >> 24; \
        (b)[1] = (n) >> 16; \
        (b)[2] = (n) >> 8; \
        (b)[3] = (n) >> 0; \
    } while(0)
/*e: macro [[PUTBE32]] */
/*s: macro [[PUTBE64]] */
#define PUTBE64(b, n)\
    do{ \
        (b)[0] = (n) >> 56; \
        (b)[1] = (n) >> 48; \
        (b)[2] = (n) >> 40; \
        (b)[3] = (n) >> 32; \
        (b)[4] = (n) >> 24; \
        (b)[5] = (n) >> 16; \
        (b)[6] = (n) >> 8; \
        (b)[7] = (n) >> 0; \
    } while(0)
/*e: macro [[PUTBE64]] */

/*s: macro [[QDIR]] */
#define QDIR(qid)   ((int)(qid)->path & (0xff))
/*e: macro [[QDIR]] */
/*s: macro [[isblank]] */
#define isblank(c) \
    (((c) != '\n') && isspace(c))
/*e: macro [[isblank]] */

extern Reprog   *authorpat;
extern Objset   objcache;
extern vlong    cachemax;
extern Hash Zhash;
extern int  chattygit;
extern int  interactive;
extern int  gitdirmode;

/*s: pragmas varargck */
#pragma varargck type "H" Hash
/*x: pragmas varargck */
#pragma varargck type "T" int
#pragma varargck type "O" Object*
#pragma varargck type "Q" Qid
/*e: pragmas varargck */
int Hfmt(Fmt*);
int Tfmt(Fmt*);
int Ofmt(Fmt*);
int Qfmt(Fmt*);

/*s: signature [[gitinit]] */
void gitinit(char*, int, int*);
/*e: signature [[gitinit]] */

/* object io */
int resolverefs(Hash **, char *);
int resolveref(Hash *, char *);
int listrefs(Hash **, char ***);
Object  *ancestor(Object *, Object *);
int findtwixt(Hash *, int, Hash *, int, Object ***, int *);
Object  *readobject(Hash);
Object  *clearedobject(Hash, int);
int expandprefix(Hash*, Hash, int);
void    parseobject(Object *);
int indexpack(char *, char *, Hash);
int writepack(int, Hash*, int, Hash*, int, Hash*);
int hasheq(Hash *, Hash *);
Object  *ref(Object *);
void    unref(Object *);
void    cache(Object *);
Object  *emptydir(void);
int entcmp(void*, void*);

/* object sets */
void    osinit(Objset *);
void    osclear(Objset *);
void    osadd(Objset *, Object *);
int oshas(Objset *, Hash);
Object  *osfind(Objset *, Hash);

/* object listing */
Objlist *mkols(void);
int olsnext(Objlist *, Hash *);
void    olsfree(Objlist *);

/* util functions */
/*s: macro [[dprint]] */
#define dprint(lvl, ...) \
    if(chattygit >= lvl) _dprint(__VA_ARGS__)
/*e: macro [[dprint]] */
void    _dprint(char *, ...);
void    *eamalloc(ulong, ulong);
void    *emalloc(ulong);
void    *earealloc(void *, ulong, ulong);
void    *erealloc(void *, ulong);
char    *estrdup(char *);
int slurpdir(char *, Dir **);
errorneg1 hparse(Hash *, char *);
char    *strip(char *);
int showprogress(int, int);
u64int  murmurhash2(void*, usize);
Qid parseqid(char*);
int charval(int);

/* packing */
void    dtinit(Dtab *, Object*);
void    dtclear(Dtab*);
Delta*  deltify(Object*, Dtab*, int*);

/* proto handling */
int readpkt(Conn*, char*, int);
int writepkt(Conn*, char*, int);
int fmtpkt(Conn*, char*, ...);
int flushpkt(Conn*);
void    initconn(Conn*, int, int);
int gitconnect(Conn *, char *, char *);
int readphase(Conn *);
int writephase(Conn *);
void    closeconn(Conn *);
void    parsecaps(char *, Conn *);
int okref(char*);

/* queues */
void    qinit(Objq*);
void    qclear(Objq*);
void    qput(Objq*, Object*, int);
int qpop(Objq*, Qelt*);
/*e: git9/git.h */
