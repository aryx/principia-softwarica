/*s: sys.h */
#define	NOP		0
/*s: [[sys.h]] process syscalls */
#define	RFORK		1
#define	EXEC		2
#define	EXITS		3
#define	AWAIT		4
/*e: [[sys.h]] process syscalls */
/*s: [[sys.h]] memory syscalls */
#define	BRK		5
/*e: [[sys.h]] memory syscalls */
/*s: [[sys.h]] file syscalls */
#define	OPEN		6
#define	CLOSE		7
#define	PREAD		8
#define	PWRITE		9
#define	SEEK		10
/*e: [[sys.h]] file syscalls */
/*s: [[sys.h]] directory syscalls */
#define	CREATE		11
#define	REMOVE		12
#define	CHDIR		13
#define	FD2PATH		14
#define	STAT		15
#define	FSTAT		16
#define	WSTAT		17
#define	FWSTAT		18
/*e: [[sys.h]] directory syscalls */
/*s: [[sys.h]] namespace syscalls */
#define	BIND		19
#define	MOUNT		20
#define	UNMOUNT		21
/*e: [[sys.h]] namespace syscalls */
/*s: [[sys.h]] time syscalls */
#define	SLEEP		22
#define	ALARM		23
/*e: [[sys.h]] time syscalls */
/*s: [[sys.h]] ipc syscalls */
#define	PIPE		26

#define	NOTIFY		24
#define	NOTED		25

#define	SEGATTACH	27
#define	SEGDETACH	28
#define	SEGFREE		29
#define	SEGFLUSH	30
#define	SEGBRK		31
/*e: [[sys.h]] ipc syscalls */
/*s: [[sys.h]] concurrency syscalls */
#define	RENDEZVOUS	32
#define	SEMACQUIRE	33
#define	SEMRELEASE	34
#define	TSEMACQUIRE	35
/*e: [[sys.h]] concurrency syscalls */
/*s: [[sys.h]] special file syscalls */
#define	DUP		    36
/*e: [[sys.h]] special file syscalls */
/*s: [[sys.h]] security syscalls */
#define	FVERSION	37
#define	FAUTH		38
/*e: [[sys.h]] security syscalls */
#define	ERRSTR		39
/*e: sys.h */
