/*s: sys.h */
#define	NOP		0
/*s: sys.h process syscalls */
#define	RFORK		1
#define	EXEC		2
#define	EXITS		3
#define	AWAIT		25
/*e: sys.h process syscalls */
/*s: sys.h memory syscalls */
#define	BRK_		32
/*e: sys.h memory syscalls */
/*s: sys.h file syscalls */
#define	OPEN		8
#define	CLOSE		9
#define	PREAD		10
#define	PWRITE		11
#define	SEEK		12
/*e: sys.h file syscalls */
/*s: sys.h directory syscalls */
#define	CREATE		6
#define	REMOVE		7
#define	CHDIR		18
#define	FD2PATH		19
#define	STAT		14
#define	FSTAT		15
#define	WSTAT		16
#define	FWSTAT		17
/*e: sys.h directory syscalls */
/*s: sys.h namespace syscalls */
#define	BIND		22
#define	MOUNT		23
#define	UNMOUNT		24
/*e: sys.h namespace syscalls */
/*s: sys.h time syscalls */
#define	SLEEP		4
#define	ALARM		5
/*e: sys.h time syscalls */
/*s: sys.h ipc syscalls */
#define	NOTIFY		30
#define	NOTED		31
#define	PIPE		21
#define	SEGATTACH	33
#define	SEGDETACH	34
#define	SEGFREE		35
#define	SEGFLUSH	36
#define	SEGBRK		37
/*e: sys.h ipc syscalls */
/*s: sys.h concurrency syscalls */
#define	RENDEZVOUS	26
#define	SEMACQUIRE	27
#define	SEMRELEASE	28
#define	TSEMACQUIRE	29
/*e: sys.h concurrency syscalls */
/*s: sys.h special file syscalls */
#define	DUP		    20
/*e: sys.h special file syscalls */
/*s: sys.h security syscalls */
#define	FVERSION	38
#define	FAUTH		39
/*e: sys.h security syscalls */
#define	ERRSTR		40
/*e: sys.h */
