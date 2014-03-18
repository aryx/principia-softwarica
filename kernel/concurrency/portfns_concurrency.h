
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
void		wlock(RWlock*);
void		wunlock(RWlock*);
int		canrlock(RWlock*);
