#include "os.h"
#include <mp.h>
#include <libsec.h>

static uchar nine[32] = {9};
static uchar zero[32] = {0};

int
x25519(uchar out[32], uchar s[32], uchar u[32])
{
	uchar sf, sl, ul;

	sf = s[0];
	sl = s[31];
	ul = u[31];

	/* clamp */
	s[0] &= ~7;			/* clear bit 0,1,2 */
	s[31] = 0x40 | (s[31] & 0x7f);	/* set bit 254, clear bit 255 */

	/*
		Implementations MUST accept non-canonical values and process them as
   		if they had been reduced modulo the field prime.  The non-canonical
   		values are 2^255 - 19 through 2^255 - 1 for X25519
	*/
	u[31] &= 0x7f;
	
	curve25519(out, s, u);

	s[0] = sf;
	s[31] = sl;
	u[31] = ul;

	return tsmemcmp(out, zero, 32) != 0;
}

void
curve25519_dh_new(uchar x[32], uchar y[32])
{
	/* new public/private key pair */
	genrandom(x, 32);
	uchar b = x[31];

	/* don't check for zero: the scalar is never
		zero because of clamping, and the basepoint is not the identity
		in the prime-order subgroup(s). */
	x25519(y, x, nine);

	/* bit 255 is always 0, so make it random */
	y[31] |= b & 0x80;
}

int
curve25519_dh_finish(uchar x[32], uchar y[32], uchar z[32])
{
	int r;

	/* remove the random bit */
	y[31] &= 0x7f;

	/* calculate dhx key */
	r = x25519(z, x, y);

	memset(x, 0, 32);
	memset(y, 0, 32);

	return r;
}
