
struct Lock
{
	ulong	key; // 0 when unset, 0xDEADDEAD when acquired
	ulong	sr; // saved priority level when using ilock() to restore in iunlock()
	ulong	pc; // for debugging
	Proc	*p; // the process locking should be the same unlocking
	Mach	*m; // not that used, only in iprintcanlock apparently
	ushort	isilock; // 0 when from lock(), 1 when from ilock()

#ifdef LOCKCYCLES
	long	lockcycles;
#endif
};
