
// taslock.c
int	lock(Lock*);
void unlock(Lock*);
void ilock(Lock*);
void iunlock(Lock*);
int	canlock(Lock*);

// ref.c
long		incref(Ref*);
long		decref(Ref*);

// qlock.c
void		qlock(QLock*);
void		qunlock(QLock*);
int		canqlock(QLock*);

void		rlock(RWlock*);
void		runlock(RWlock*);
int		canrlock(RWlock*);
void		wlock(RWlock*);
void		wunlock(RWlock*);

// in 386/l.s (but used in port)
//@Scheck: Assembly
int		islo(void);
//@Scheck: Assembly
int		splhi(void);
//@Scheck: Assembly
int		spllo(void);
//@Scheck: Assembly
void		splx(int);
//@Scheck: Assembly
//void		splxpc(int);
//@Scheck: Assembly
//long		lcycles(void);
//test-and-set
//@Scheck: Assembly
void		_xinc(long*);
//@Scheck: Assembly
long		_xdec(long*);
