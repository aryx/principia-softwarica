
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
int		islo(void);
int		splhi(void);
int		spllo(void);
void		splx(int);
void		splxpc(int);
long		lcycles(void);
//test-and-set
void		_xinc(long*);
long		_xdec(long*);
