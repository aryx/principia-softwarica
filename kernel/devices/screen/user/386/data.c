#include <u.h>
#include <libc.h>
#include <bio.h>

#include "pci.h"
#include "vga.h"

int cflag;					/* do not use hwgc */
int dflag;					/* do the palette */

Ctlr* ctlrs[] = {
	&clgd542x,				/* ctlr */
	&clgd542xhwgc,				/* hwgc */
	&generic,				/* ctlr */
	&palette,				/* ctlr */
	&vesa,					/* ctlr */
	0,
};

/*
 * Lower 2-bits of indirect DAC register
 * addressing.
 */
ushort dacxreg[4] = {
	PaddrW, Pdata, Pixmask, PaddrR
};
