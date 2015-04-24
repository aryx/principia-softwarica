typedef unsigned long ulong;

typedef	struct	Vlong	Vlong;
struct	Vlong
{
	ulong	lo;
	ulong	hi;
};

void
_rshlv(Vlong *r, Vlong a, int b)
{
	ulong t;

	t = a.hi;
	if(b >= 32) {
		r->hi = 0;
		if(b >= 64) {
			/* this is illegal re C standard */
			r->lo = 0;
			return;
		}
		r->lo = t >> (b-32);
		return;
	}
	if(b <= 0) {
		r->hi = t;
		r->lo = a.lo;
		return;
	}
	r->hi = t >> b;
	r->lo = (t << (32-b)) | (a.lo >> b);
}
