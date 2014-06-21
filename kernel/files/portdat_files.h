/*s: portdat_files.h */

/*s: enum blockflag */
/* flag values */
enum
{
  BINTR = (1<<0),
  BFREE = (1<<1),
  Bipck = (1<<2),   /* ip checksum */
  Budpck  = (1<<3),   /* udp checksum */
  Btcpck  = (1<<4),   /* tcp checksum */
  Bpktck  = (1<<5),   /* packet checksum */
};
/*e: enum blockflag */

/*s: struct Block */
struct Block
{
  long  ref;
  Block*  next;
  Block*  list;
  uchar*  rp;     /* first unconsumed byte */
  uchar*  wp;     /* first empty byte */
  uchar*  lim;      /* 1 past the end of the buffer */
  uchar*  base;     /* start of the buffer */
  void  (*free)(Block*);
  ushort  flag;
  ushort  checksum;   /* IP checksum of complete packet (minus media header) */
};
/*e: struct Block */

/*s: function BLEN */
#define BLEN(s) ((s)->wp - (s)->rp)
/*e: function BLEN */
/*s: function BALLOC */
#define BALLOC(s) ((s)->lim - (s)->base)
/*e: function BALLOC */


/*s: enum queuestate */
/* queue state bits,  Qmsg, Qcoalesce, and Qkick can be set in qopen */
enum
{
  /* Queue.state */
  Qstarve   = (1<<0), /* consumer starved */
  Qmsg    = (1<<1), /* message stream */
  Qclosed   = (1<<2), /* queue has been closed/hungup */
  Qflow   = (1<<3), /* producer flow controlled */
  Qcoalesce = (1<<4), /* coalesce packets on read */
  Qkick   = (1<<5), /* always call the kick routine after qwrite */
};
/*e: enum queuestate */

// defined in qio.c
extern  uint  qiomaxatomic;

/*
 *  IO queues
 */
// was in qio.c
/*s: struct Queue */
struct Queue
{
  Lock;

  Block*  bfirst;   /* buffer */
  Block*  blast;

  int len;    /* bytes allocated to queue */
  int dlen;   /* data bytes in queue */
  int limit;    /* max bytes in queue */
  int inilim;   /* initial limit */
  int state;
  int noblock;  /* true if writes return immediately when q full */
  int eof;    /* number of eofs read by user */

  void  (*kick)(void*); /* restart output */
  void  (*bypass)(void*, Block*); /* bypass queue altogether */
  void* arg;    /* argument to kick */

  QLock rlock;    /* mutex for reading processes */
  Rendez  rr;   /* process waiting to read */
  QLock wlock;    /* mutex for writing processes */
  Rendez  wr;   /* process waiting to write */

  char  err[ERRMAX];
};
/*e: struct Queue */


// was in cache.c
/*s: struct Extent */
struct Extent
{
  int bid;
  ulong start;
  int len;
  Page  *cache;
  Extent  *next;
};
/*e: struct Extent */

// was in cache.c
/*s: struct Mntcache */
struct Mntcache
{
  Qid qid;
  int dev;
  int type;
  QLock;
  Extent   *list;
  Mntcache *hash;
  Mntcache *prev;
  Mntcache *next;
};
/*e: struct Mntcache */


/*s: struct Mount */
struct Mount
{
  ulong mountid;
  Mount*  next;
  Mhead*  head;
  Mount*  copy;
  Mount*  order;
  Chan* to;     /* channel replacing channel */
  int mflag;
  char  *spec;
};
/*e: struct Mount */

/*s: struct Mhead */
struct Mhead
{
  Ref;
  RWlock  lock;
  Chan* from;     /* channel mounted upon */
  Mount*  mount;      /* what's mounted upon it */
  Mhead*  hash;     /* Hash chain */
};
/*e: struct Mhead */



#include <fcall.h>

// was in devmnt.c
/*s: struct Mntrpc */
struct Mntrpc
{
  Chan* c;    /* Channel for whom we are working */
  Mntrpc* list;   /* Free/pending list */
  Fcall request;  /* Outgoing file system protocol message */
  Fcall   reply;    /* Incoming reply */
  Mnt*  m;    /* Mount device during rpc */
  Rendez  r;    /* Place to hang out */
  uchar*  rpc;    /* I/O Data buffer */
  uint  rpclen;   /* len of buffer */
  Block *b;   /* reply blocks */
  char  done;   /* Rpc completed */
  uvlong  stime;    /* start time for mnt statistics */
  ulong reqlen;   /* request length for mnt statistics */
  ulong replen;   /* reply length for mnt statistics */
  Mntrpc* flushed;  /* message this one flushes */
};
/*e: struct Mntrpc */

/*s: struct Mnt */
struct Mnt
{
  Lock;
  /* references are counted using c->ref; channels on this mount point incref(c->mchan) == Mnt.c */
  Chan  *c;   /* Channel to file service */
  Proc  *rip;   /* Reader in progress */
  Mntrpc  *queue;   /* Queue of pending requests on this channel */
  ulong id;   /* Multiplexer id for channel check */
  Mnt *list;    /* Free list */
  int flags;    /* cache */
  int msize;    /* data + IOHDRSZ */
  char  *version; /* 9P version */
  Queue *q;   /* input queue */
};
/*e: struct Mnt */



/*s: struct Path */
struct Path
{
  char  *s;
  int len;      /* strlen(s) */
  int alen;     /* allocated length of s */

  Chan  **mtpt;     /* mtpt history */
  int mlen;     /* number of path elements */
  int malen;      /* allocated length of mtpt */

  // extra
  Ref;
};
/*e: struct Path */

/*s: enum accessnamec */
/*
 * Access types in namec
 */
enum
{
  Aaccess,      /* as in stat, wstat */
  Abind,        /* for left-hand-side of bind */
  Atodir,       /* as in chdir */
  Aopen,        /* for i/o */
  Amount,       /* to be mounted or mounted upon */
  Acreate,      /* is to be created */
  Aremove,      /* will be removed by caller */
};
/*e: enum accessnamec */

/*s: enum channelflag */
/*
 * channel flags
 */
enum 
{
  COPEN = 0x0001,   /* for i/o */
  CFREE = 0x0010,   /* not in use */
  /*s: enum channelflag cases */
  CCEXEC  = 0x0008,   /* close on exec */
  /*x: enum channelflag cases */
  CRCLOSE = 0x0020,   /* remove on close */
  /*x: enum channelflag cases */
  CMSG  = 0x0002,   /* the message channel for a mount */
  /*x: enum channelflag cases */
  CCACHE  = 0x0080,   /* client cache */
  /*e: enum channelflag cases */
};
/*e: enum channelflag */

/*s: struct Chan */
struct Chan
{
    ushort type; // idx in devtab
    ulong dev;
    Qid qid;

    Path* path;

    vlong offset;     /* in fd */
    //enum<open>
    ushort mode;     /* read/write */

    bool ismtpt; // is a mount point

    union {
       void* aux; // generic pointer, for specific usages
       /*s: [[Chan]] union other fields */
       /*Pipe*/void* chanpipe; // for pipes
       /*x: [[Chan]] union other fields */
       Qid pgrpid;   /* for #p/notepg */
       /*x: [[Chan]] union other fields */
       ulong mid;    /* for ns in devproc */
       /*e: [[Chan]] union other fields */
    };
    /*s: [[Chan]] other fields */
    // enum<channelflag>
    ushort  flag;
    /*x: [[Chan]] other fields */
    vlong devoffset;    /* in underlying device; see read */
    /*x: [[Chan]] other fields */
    Mhead*  umh;      /* mount point that derived Chan; used in unionread */
    Chan* umc;      /* channel in union; held for union read */
    QLock umqlock;    /* serialize unionreads */
    int uri;      /* union read index */
    /*x: [[Chan]] other fields */
    uchar*  dirrock;    /* directory entry rock for translations */
    int nrock;
    int mrock;
    QLock rockqlock;
    /*x: [[Chan]] other fields */
    Chan* mchan;      /* channel to mounted server */
    /*x: [[Chan]] other fields */
    int dri;      /* devdirread index */
    /*x: [[Chan]] other fields */
    int fid;      /* for devmnt */
    ulong iounit;     /* chunk size for i/o; 0==default */

    Mnt*  mux;      /* Mnt for clients using me for messages */

    Qid mqid;     /* qid of root of mount point */
    /*x: [[Chan]] other fields */
    Mntcache* mcp;      /* Mount cache pointer */
    /*e: [[Chan]] other fields */

    // extra
    Ref; /* the Lock in this Ref is also Chan's lock */
    /*s: [[Chan]] extra fields */
    Chan* next;     /* allocation */
    Chan* link;
    /*e: [[Chan]] extra fields */
};
/*e: struct Chan */


/*s: struct Evalue */
struct Evalue
{
  char  *name;
  char  *value;
  int len;
  Evalue  *link;
  Qid qid;
};
/*e: struct Evalue */

/*s: struct Egrp */
struct Egrp
{
  Ref;
  RWlock;
  Evalue  **ent;
  int nent;
  int ment;
  ulong path; /* qid.path of next Evalue to be allocated */
  ulong vers; /* of Egrp */
};
/*e: struct Egrp */


// internals

/*s: struct Walkqid */
struct Walkqid
{
  Chan  *clone;
  int nqid;
  Qid qid[1];
};
/*e: struct Walkqid */

/*s: struct Dev */
struct Dev
{
    Rune dc; // dev character code, e.g. '/' (devroot), 'e' (devenv), ...
    char* name;

    /*s: [[Dev]] methods */
    void  (*reset)(void); // done once at boot time
    /*x: [[Dev]] methods */
    void  (*init)(void);
    /*x: [[Dev]] methods */
    Chan* (*open)(Chan*, int);
    /*x: [[Dev]] methods */
    void  (*close)(Chan*);
    /*x: [[Dev]] methods */
    long  (*read)(Chan*, void*, long, vlong);
    /*x: [[Dev]] methods */
    long  (*write)(Chan*, void*, long, vlong);
    /*x: [[Dev]] methods */
    void  (*create)(Chan*, char*, int, ulong);
    /*x: [[Dev]] methods */
    Walkqid*(*walk)(Chan*, Chan*, char**, int);
    /*x: [[Dev]] methods */
    void  (*remove)(Chan*);
    /*x: [[Dev]] methods */
    int (*stat)(Chan*, uchar*, int);
    /*x: [[Dev]] methods */
    int (*wstat)(Chan*, uchar*, int);
    /*x: [[Dev]] methods */
    Chan* (*attach)(char*);
    /*x: [[Dev]] methods */
    Block* (*bread)(Chan*, long, ulong);
    long  (*bwrite)(Chan*, Block*, ulong);
    /*x: [[Dev]] methods */
    void  (*shutdown)(void);
    /*x: [[Dev]] methods */
    int (*config)(int, char*, DevConf*);  /* returns nil on error */
    /*x: [[Dev]] methods */
    void  (*power)(bool);  /* power mgt: power(1) => on, power (0) => off */
    /*e: [[Dev]] methods */
};
/*e: struct Dev */


// array<Dev>, it looks like an allocated array<ref<dev>> but
// it is really a static array put here to avoid backward deps on conf_devtab,
// and it is not really a <ref<dev>> because it's pointers to static
// structures (e.g. mousedevtab, vgadevtab, etc).
extern Dev** devtab;

/*s: struct Dirtab */
struct Dirtab
{
  char  name[KNAMELEN];
  Qid qid;
  vlong length;
  long  perm;
};
/*e: struct Dirtab */



//*****************************************************************************
// Internal to memory/
//*****************************************************************************

/*s: constants tags */
enum
{
    TAGSHIFT = 5,           /* ulong has to be 32 bits */
    TAGMASK = (1<<TAGSHIFT)-1,
    NMASK = (64*1024)>>TAGSHIFT,
};
/*e: constants tags */

// actually internal to devmnt.c and mnt.c
/*s: struct Mntalloc */
/*
 * References are managed as follows:
 * The channel to the server - a network connection or pipe - has one
 * reference for every Chan open on the server.  The server channel has
 * c->mux set to the Mnt used for muxing control to that server.  Mnts
 * have no reference count; they go away when c goes away.
 * Each channel derived from the mount point has mchan set to c,
 * and increfs/decrefs mchan to manage references on the server
 * connection.
 */
struct Mntalloc
{
    Mnt*    list;       /* Mount devices in use */
    Mnt*    mntfree;    /* Free list */
    Mntrpc* rpcfree;
    int nrpcfree;
    int nrpcused;
    ulong   id;
    ulong   tagmask[NMASK];

    // extra
    Lock;

};
/*e: struct Mntalloc */
extern struct Mntalloc mntalloc;

// TODO: mv in errstr.c?
extern char Esbadstat[];
extern char Enoversion[];
/*e: portdat_files.h */
