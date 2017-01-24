/*s: portfns_files.h */

// allocb.c
Block* allocb(int);
void   freeb(Block*);
Block* iallocb(int);
void   iallocsummary(void);
void   checkb(Block*, char*);

// cache.c
void   cinit(void);
void   copen(Chan*);
int    cread(Chan*, uchar*, int, vlong);
void   cwrite(Chan*, uchar*, int, vlong);
void   cupdate(Chan*, uchar*, int, vlong);

// qio.c
void   freeblist(Block*);
int    blocklen(Block*);
Queue* qopen(int, int, void (*)(void*), void*);
void   qhangup(Queue*, char*);
Block* qget(Queue*);
int    qdiscard(Queue*, int);
void   qreopen(Queue*);
Block* qremove(Queue*);
void   qputback(Queue*, Block*);
Block* packblock(Block*);
Queue* qbypass(void (*)(void*, Block*), void*);
int    pullblock(Block**, int);
Block* bl2mem(uchar*, Block*, int);
void   qnoblock(Queue*, bool);
int    qisclosed(Queue*);
int    qfull(Queue*);
int    qwindow(Queue*);
void   qsetlimit(Queue*, int);
int    qpass(Queue*, Block*);
void   qclose(Queue*);
int    qiwrite(Queue*, void*, int);
int    qproduce(Queue*, void*, int);
Block* padblock(Block*, int);
Block* pullupblock(Block*, int);
int    qpassnolim(Queue*, Block*);
Block* copyblock(Block*, int);
Block* qcopy(Queue*, int, ulong);
void   qflush(Queue*);
long   qbwrite(Queue*, Block*);
int    qconsume(Queue*, void*, int);
//Block*    mem2bl(uchar*, int);
Block* trimblock(Block*, int, int);
void   qaddlist(Queue*, Block*);
Block* concatblock(Block*);
void   ixsummary(void);
Block* pullupqueue(Queue*, int);
void   qfree(Queue*);
int    qwrite(Queue*, void*, int);
Block* adjustblock(Block*, int);
Block* qbread(Queue*, int);
long   qread(Queue*, void*, int);
int    qcanread(Queue*);
int    qlen(Queue*);

// chan.c
Chan*  cclone(Chan*);
void   putmhead(Mhead*);
int    eqchantdqid(Chan*, int, int, Qid, bool);
Mhead* newmhead(Chan*);
void   error_if_not_dir(Chan*);
char*  chanpath(Chan*);
int    emptystr(char*);
void   chandevinit(void);
void   chandevshutdown(void);
void   chandevreset(void);
int    eqqid(Qid, Qid);
int    cmount(Chan*, Chan*, int, char*);
void   cunmount(Chan*, Chan*);
bool   findmount(Chan**, Mhead**, int, int, Qid);
void   pathclose(Path*);
Path*  newpath(char*);
Chan*  newchan(void);
void   chanfree(Chan*);
//cchan_close => cclose
char*  validnamedup(char*, bool);
void   validname(char*, bool);
void   nameerror(char*, char*);
int    walk(Chan**, char**, int, int, int*);
void   ccloseq(Chan*);
Chan*  namec(char*, int, int, ulong);
//long    unionread(Chan*, void*, long);

// file.c
Chan*  fdtochan(int, int, bool, bool);
int    openmode(ulong);
void   fdclose(int, int);
void   validstat(uchar*, int);

// dev.c
void   mkqid(Qid*, vlong, ulong, int);
int    devno(Rune, bool);
void   devdir(Chan*, Qid, char*, vlong, char*, long, DirEntry*);
void   devinit(void);
void   devreset(void);
void   devshutdown(void);
Chan*  devclone(Chan*);
long   devdirread(Chan*, char*, long, Dirtab*, int, Devgen*);
void   devpermcheck(char*, ulong, int);
void   devcreate(Chan*, char*, int, ulong);
Block* devbread(Chan*, long, ulong);
long   devbwrite(Chan*, Block*, ulong);
void   devremove(Chan*);
int    devwstat(Chan*, uchar*, int);
#define DEVDOTDOT -1
Devgen    devgen;//TODO?
//void    devpower(int);
//int   devconfig(int, char *, DevConf *);//TODO? why fp no-deps?
int    devstat(Chan*, uchar*, int, Dirtab*, int, Devgen*);
Chan*  devopen(Chan*, int, Dirtab*, int, Devgen*);
Chan*  devattach(Rune, char*);
Walkqid* devwalk(Chan*, Chan*, char**, int, Dirtab*, int, Devgen*);

// env.c
void   closeegrp(Egrp*);

// mnt.c
void   muxclose(Mnt*);
void   mntfree(Mntrpc*);
//void    mntpntfree(Mnt*);

// sysfile.c
int    newfd(Chan*);
// many sysxxx functions (used in syscalls/ without requiring extern decl)
/*e: portfns_files.h */
