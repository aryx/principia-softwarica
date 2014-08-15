#include <u.h>
#include <libc.h>
#include <authsrv.h>

#define	CHAR(x)		*p++ = f->x
//pad: dupe with fcall.h again?
#define	SHORT(x)	do{p[0] = f->x; p[1] = f->x>>8; p += 2;}while(0)
#define	VLONG(q)	do{p[0] = (q); p[1] = (q)>>8; p[2] = (q)>>16; p[3] = (q)>>24; p += 4;}while(0)
#define	LONG(x)		VLONG(f->x)
#define	STRING(x,n)	do{memmove(p, f->x, n); p += n;}while(0)

int
convA2M(Authenticator *f, char *ap, char *key)
{
	int n;
	uchar *p;

	p = (uchar*)ap;
	CHAR(num);
	STRING(chal, CHALLEN);
	LONG(id);
	n = p - (uchar*)ap;
	if(key)
		encrypt(key, ap, n);
	return n;
}
