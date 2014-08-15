#include <u.h>
#include <libc.h>
#include <authsrv.h>

#define	CHAR(x)		f->x = *p++
//pad: dupe with fcall.h again?
#define	SHORT(x)	do{f->x = (p[0] | (p[1]<<8)); p += 2;}while(0)
#define	VLONG(q)	do{q = (p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24)); p += 4;}while(0)
#define	LONG(x)		VLONG(f->x)
#define	STRING(x,n)	do{memmove(f->x, p, n); p += n;}while(0)

void
convM2A(char *ap, Authenticator *f, char *key)
{
	uchar *p;

	if(key)
		decrypt(key, ap, AUTHENTLEN);
	p = (uchar*)ap;
	CHAR(num);
	STRING(chal, CHALLEN);
	LONG(id);
	USED(p);
}
