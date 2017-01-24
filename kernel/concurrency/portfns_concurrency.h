/*s: portfns_concurrency.h */

// taslock.c
int  lock(Lock*);
void unlock(Lock*);
void ilock(Lock*);
void iunlock(Lock*);
int  canlock(Lock*);

// ref.c
long incref(Ref*);
long decref(Ref*);

// qlock.c
void qlock(QLock*);
void qunlock(QLock*);
int  canqlock(QLock*);

void rlock(RWlock*);
void runlock(RWlock*);
int  canrlock(RWlock*);
void wlock(RWlock*);
void wunlock(RWlock*);

// in <arch>/l.s (but used in port)
//@Scheck: Assembly
bool arch_islo(void);
//@Scheck: Assembly
int  arch_splhi(void);
//@Scheck: Assembly
int  arch_spllo(void);
//@Scheck: Assembly
void arch_splx(int);
//test-and-set
//@Scheck: Assembly
int  arch_tas(void*);
//@Scheck: Assembly
void arch_xinc(long*);
//@Scheck: Assembly
long arch_xdec(long*);

// <arch>/??? (called from port but signature not portable across <arch>)
//void  arch_coherence(void);
//int   arch_cmpswap(long*, long, long); 

/*e: portfns_concurrency.h */
