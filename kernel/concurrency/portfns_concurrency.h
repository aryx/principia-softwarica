/*s: portfns_concurrency.h */
// For protecting kernel syscall code from interrupt kernel code: splhi(), splo()
// For very small critical region: taslock
// For bigger region, or when have lots of contention on a lock: qlock

// taslock.c
int lock(Lock*);
void unlock(Lock*);
void ilock(Lock*);
void iunlock(Lock*);
int canlock(Lock*);

// ref.c
long    incref(Ref*);
long    decref(Ref*);

// qlock.c
void    qlock(QLock*);
void    qunlock(QLock*);
int   canqlock(QLock*);

void    rlock(RWlock*);
void    runlock(RWlock*);
int   canrlock(RWlock*);
void    wlock(RWlock*);
void    wunlock(RWlock*);

// in 386/l.s (but used in port)
//@Scheck: Assembly
int   islo(void);
//@Scheck: Assembly
int   splhi(void);
//@Scheck: Assembly
int   spllo(void);
//@Scheck: Assembly
void    splx(int);
//void    splxpc(int);
//long    lcycles(void);
//test-and-set
//@Scheck: Assembly
int tas(void*);
//@Scheck: Assembly
void    _xinc(long*);
//@Scheck: Assembly
long    _xdec(long*);
/*e: portfns_concurrency.h */
