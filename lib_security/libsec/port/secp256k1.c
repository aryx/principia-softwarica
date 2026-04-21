/*
 * secp256k1 (Bitcoin/Koblitz curve) parameters.
 *
 * 2026-04-21: principia hand-written substitute for the mpc-generated
 * secp256k1.c that 9front's libsec/mkfile produces from secp256k1.mp.
 * Replace with mpc output if mpc ever arrives.
 *
 * Curve: y^2 = x^3 + 7 (mod p)  (a = 0, b = 7)
 *   p = 2^256 - 2^32 - 2^9 - 2^8 - 2^7 - 2^6 - 2^4 - 1
 * All constants from SEC2 v2 section 2.4.1.
 */
#include "os.h"
#include <mp.h>
#include <libsec.h>

void
secp256k1(mpint *p, mpint *a, mpint *b, mpint *x, mpint *y, mpint *n, mpint *h)
{
	strtomp("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F", nil, 16, p);
	uitomp(0, a);
	uitomp(7, b);
	strtomp("79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798", nil, 16, x);
	strtomp("483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8", nil, 16, y);
	strtomp("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141", nil, 16, n);
	uitomp(1, h);
}
