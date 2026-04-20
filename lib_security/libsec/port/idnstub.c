/* Claude Code
 *
 * Copyright (C) 2026 Yoann Padioleau
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License.
 */

/*
 * idn2utf / utf2idn stubs for the 9front libsec merge.
 *
 * 9front's x509.c calls idn2utf() when decoding an email address or DNS
 * name from an X.509 certificate subject, and utf2idn() when going the
 * other way. The real implementations live in 9front's libc (9sys/idn.c)
 * and convert between punycode-encoded IDN labels and UTF-8. Principia's
 * libc does not ship those, so we stub them to always report "not IDN" by
 * returning -1. Callers in x509.c / tlshand.c all have a strncpy fallback
 * that passes the input through verbatim, which is exactly what we want
 * for pure ASCII hostnames (the vast majority of certs in practice).
 *
 * If/when principia's libc grows a real idn.c, this file can be deleted.
 */

#include <u.h>
#include <libc.h>

int
idn2utf(char *name, char *buf, int nbuf)
{
	USED(name); USED(buf); USED(nbuf);
	return -1;
}

int
utf2idn(char *name, char *buf, int nbuf)
{
	USED(name); USED(buf); USED(nbuf);
	return -1;
}
