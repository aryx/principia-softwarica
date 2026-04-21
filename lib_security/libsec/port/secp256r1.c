/*
 * secp256r1 (NIST P-256) curve parameters.
 *
 * 2026-04-21: principia hand-written substitute for the mpc-generated
 * secp256r1.c that 9front's libsec/mkfile produces from secp256r1.mp.
 * We avoid the mpc code-generation step at build time; these constants
 * are stable (SEC1 / RFC 5480), so checking them in is safe. Replace
 * this file with the mpc output if mpc ever arrives.
 *
 * Curve: y^2 = x^3 + ax + b (mod p)
 *   p = 2^256 - 2^224 + 2^192 + 2^96 - 1
 *   a = p - 3
 * All constants from SEC2 v2 section 2.4.2.
 */
#include "os.h"
#include <mp.h>
#include <libsec.h>

void
secp256r1(mpint *p, mpint *a, mpint *b, mpint *x, mpint *y, mpint *n, mpint *h)
{
	strtomp("FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF", nil, 16, p);
	strtomp("FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC", nil, 16, a);
	strtomp("5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B", nil, 16, b);
	strtomp("6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296", nil, 16, x);
	strtomp("4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5", nil, 16, y);
	strtomp("FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551", nil, 16, n);
	uitomp(1, h);
}
