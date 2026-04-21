/*
 * secp384r1 (NIST P-384) curve parameters.
 *
 * 2026-04-21: principia hand-written substitute for the mpc-generated
 * secp384r1.c that 9front's libsec/mkfile produces from secp384r1.mp.
 * Replace with mpc output if mpc ever arrives.
 *
 * Curve: y^2 = x^3 + ax + b (mod p)
 *   p = 2^384 - 2^128 - 2^96 + 2^32 - 1
 *   a = p - 3
 * All constants from SEC2 v2 section 2.5.1.
 */
#include "os.h"
#include <mp.h>
#include <libsec.h>

void
secp384r1(mpint *p, mpint *a, mpint *b, mpint *x, mpint *y, mpint *n, mpint *h)
{
	strtomp("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFF",
		nil, 16, p);
	strtomp("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFC",
		nil, 16, a);
	strtomp("B3312FA7E23EE7E4988E056BE3F82D19181D9C6EFE8141120314088F5013875AC656398D8A2ED19D2A85C8EDD3EC2AEF",
		nil, 16, b);
	strtomp("AA87CA22BE8B05378EB1C71EF320AD746E1D3B628BA79B9859F741E082542A385502F25DBF55296C3A545E3872760AB7",
		nil, 16, x);
	strtomp("3617DE4A96262C6F5D9E98BF9292DC29F8F41DBD289A147CE9DA3113B5F0B8C00A60B1CE1D7E819D7A431D7C90EA0E5F",
		nil, 16, y);
	strtomp("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC7634D81F4372DDF581A0DB248B0A77AECEC196ACCC52973",
		nil, 16, n);
	uitomp(1, h);
}
