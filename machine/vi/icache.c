#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "mips.h"

void
icacheinit(void)
{
}

void
updateicache(ulong addr)
{
	USED(addr);
}

