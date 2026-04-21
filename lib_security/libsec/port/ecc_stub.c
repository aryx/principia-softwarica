/* Claude Code
 *
 * Copyright (C) 2026 Yoann Padioleau
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License.
 */

/*
 * Stub implementations of the generic ECC (elliptic-curve) functions that
 * 9front's tlshand.c and x509.c reference. The real implementations live
 * in ecc.c + mpc-generated secp*.c + jacobian.c, none of which we compile
 * here because the mpc tool isn't available in principia yet.
 *
 * principia's tlshand.c was trimmed so that the namedcurves[] table only
 * advertises X25519; during negotiation the non-X25519 branches are
 * therefore unreachable. x509.c still calls X509ecdsaverify* when it
 * encounters an ECDSA-signed server certificate, but the vast majority
 * of real-world servers have RSA certs available and will serve the
 * correct one if our ClientHello offers RSA signature algorithms.
 *
 * If any of these functions gets called at runtime, we panic loudly so
 * the cause is immediately obvious, rather than silently misbehaving.
 *
 * When the mpc tool arrives and ecc.c / secp*.c can be built, this file
 * should be deleted and the real ecc.c added to the libsec mkfile.
 */

#include <u.h>
#include <libc.h>
#include <mp.h>
#include <libsec.h>

static void
eccstub(char *name)
{
	sysfatal("libsec: %s called but ECC is not compiled in "
		"(needs ecc.c + mpc-generated secp*.c)", name);
}

void
ecdominit(ECdomain *dom, void (*init)(mpint*, mpint*, mpint*, mpint*, mpint*, mpint*, mpint*))
{
	USED(dom); USED(init);
	eccstub("ecdominit");
}

void
ecdomfree(ECdomain *dom)
{
	USED(dom);
	/* no-op: nothing ever allocated */
}

void
ecassign(ECdomain *dom, ECpoint *old, ECpoint *new)
{
	USED(dom); USED(old); USED(new);
	eccstub("ecassign");
}

void
ecadd(ECdomain *dom, ECpoint *a, ECpoint *b, ECpoint *s)
{
	USED(dom); USED(a); USED(b); USED(s);
	eccstub("ecadd");
}

void
ecmul(ECdomain *dom, ECpoint *a, mpint *k, ECpoint *s)
{
	USED(dom); USED(a); USED(k); USED(s);
	eccstub("ecmul");
}

ECpoint*
strtoec(ECdomain *dom, char *s, char **end, ECpoint *p)
{
	USED(dom); USED(s); USED(end); USED(p);
	eccstub("strtoec");
	return nil;
}

ECpriv*
ecgen(ECdomain *dom, ECpriv *p)
{
	USED(dom); USED(p);
	eccstub("ecgen");
	return nil;
}

int
ecverify(ECdomain *dom, ECpoint *p)
{
	USED(dom); USED(p);
	eccstub("ecverify");
	return 0;
}

int
ecpubverify(ECdomain *dom, ECpub *p)
{
	USED(dom); USED(p);
	eccstub("ecpubverify");
	return 0;
}

void
ecdsasign(ECdomain *dom, ECpriv *pr, uchar *dig, int ndig, mpint *r, mpint *s)
{
	USED(dom); USED(pr); USED(dig); USED(ndig); USED(r); USED(s);
	eccstub("ecdsasign");
}

int
ecdsaverify(ECdomain *dom, ECpub *pub, uchar *dig, int ndig, mpint *r, mpint *s)
{
	USED(dom); USED(pub); USED(dig); USED(ndig); USED(r); USED(s);
	eccstub("ecdsaverify");
	return 0;
}

ECpub*
ecdecodepub(ECdomain *dom, uchar *p, int n)
{
	USED(dom); USED(p); USED(n);
	eccstub("ecdecodepub");
	return nil;
}

int
ecencodepub(ECdomain *dom, ECpub *pub, uchar *p, int n)
{
	USED(dom); USED(pub); USED(p); USED(n);
	eccstub("ecencodepub");
	return 0;
}

void
ecpubfree(ECpub *p)
{
	USED(p);
	/* no-op: nothing ever allocated */
}

/*
 * NIST curve initializers referenced by x509.c's namedcurves[] table.
 * The real versions are generated from secp256r1.mp / secp384r1.mp via
 * mpc; we stub them so the library links. Unreachable at runtime as
 * long as tlshand's namedcurves[] only advertises X25519 (we trimmed
 * it above) and no ECDSA cert makes it to X509toECpub.
 */
void
secp256r1(mpint *p, mpint *a, mpint *b, mpint *x, mpint *y, mpint *n, mpint *h)
{
	USED(p); USED(a); USED(b); USED(x); USED(y); USED(n); USED(h);
	eccstub("secp256r1");
}

void
secp384r1(mpint *p, mpint *a, mpint *b, mpint *x, mpint *y, mpint *n, mpint *h)
{
	USED(p); USED(a); USED(b); USED(x); USED(y); USED(n); USED(h);
	eccstub("secp384r1");
}

void
secp256k1(mpint *p, mpint *a, mpint *b, mpint *x, mpint *y, mpint *n, mpint *h)
{
	USED(p); USED(a); USED(b); USED(x); USED(y); USED(n); USED(h);
	eccstub("secp256k1");
}
