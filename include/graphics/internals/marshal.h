/*s: include/marshal.h */
/*s: function BGSHORT */
#define	BGSHORT(p)		(((p)[0]<<0) | ((p)[1]<<8))
/*e: function BGSHORT */
/*s: function BGLONG */
#define	BGLONG(p)		((BGSHORT(p)<<0) | (BGSHORT(p+2)<<16))
/*e: function BGLONG */
/*s: function BPSHORT */
#define	BPSHORT(p, v)		((p)[0]=(v), (p)[1]=((v)>>8))
/*e: function BPSHORT */
/*s: function BPLONG */
#define	BPLONG(p, v)		(BPSHORT(p, (v)), BPSHORT(p+2, (v)>>16))
/*e: function BPLONG */

extern byte*	bufimage(Display*, int);

/*e: include/marshal.h */
