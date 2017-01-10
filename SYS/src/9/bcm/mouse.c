#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include "io.h"

#define	Image	IMAGE
#include <draw.h>
#include <memdraw.h>
#include <cursor.h>
#include "screen.h"

/*
 *  mouse types
 */
enum
{
	Mouseother=	0,
	Mouseserial=	1,
	MousePS2=	2,
};

extern int mouseshifted;

static QLock mousectlqlock;
static int mousetype;
static int intellimouse;
static int packetsize;
static int resolution;
static int accelerated;
static int mousehwaccel;
static char mouseport[5];

enum
{
	CMaccelerated,
	CMhwaccel,
	CMintellimouse,
	CMlinear,
	CMps2,
	CMps2intellimouse,
	CMres,
	CMreset,
	CMserial,
};

static Cmdtab mousectlmsg[] =
{
	CMaccelerated,		"accelerated",		0,
	CMhwaccel,		"hwaccel",		2,
	CMintellimouse,		"intellimouse",		1,
	CMlinear,		"linear",		1,
	CMps2,			"ps2",			1,
	CMps2intellimouse,	"ps2intellimouse",	1,
	CMres,			"res",			0,
	CMreset,		"reset",		1,
	CMserial,		"serial",		0,
};

/*
 *  ps/2 mouse message is three bytes
 *
 *	byte 0 -	0 0 SDY SDX 1 M R L
 *	byte 1 -	DX
 *	byte 2 -	DY
 *
 *  shift & right button is the same as middle button
 *
 * Intellimouse and AccuPoint with extra buttons deliver
 *	byte 3 -	00 or 01 or FF according to extra button state.
 * extra buttons are mapped in this code to buttons 4 and 5.
 * AccuPoint generates repeated events for these buttons;
*  it and Intellimouse generate 'down' events only, so
 * user-level code is required to generate button 'up' events
 * if they are needed by the application.
 * Also on laptops with AccuPoint AND external mouse, the
 * controller may deliver 3 or 4 bytes according to the type
 * of the external mouse; code must adapt.
 *
 * On the NEC Versa series (and perhaps others?) we seem to
 * lose a byte from the packet every once in a while, which
 * means we lose where we are in the instruction stream.
 * To resynchronize, if we get a byte more than two seconds
 * after the previous byte, we assume it's the first in a packet.
 */
static void
ps2mouseputc(int c, int shift)
{
	static short msg[4];
	static int nb;
	static uchar b[] = {0, 1, 4, 5, 2, 3, 6, 7, 0, 1, 2, 3, 2, 3, 6, 7 };
	static ulong lasttick;
	ulong m;
	int buttons, dx, dy;

	shift |= mouseshifted;
	m = CPUS(0)->ticks;
	if(TK2SEC(m - lasttick) > 2)
		nb = 0;
	lasttick = m;
	if(nb==0 && (c&0xc8)!=0x08)
		if(intellimouse && (c==0x00 || c==0x01 || c==0xFF)){
			packetsize = 4;
			return;
		}

	msg[nb] = c;
	if(++nb == packetsize){
		nb = 0;
		if(msg[0] & 0x10)
			msg[1] |= 0xFF00;
		if(msg[0] & 0x20)
			msg[2] |= 0xFF00;

		buttons = b[(msg[0]&7) | (shift ? 8 : 0)];
		if(intellimouse && packetsize==4){
			if((msg[3]&0xc8) == 0x08){
				packetsize = 3;
				msg[0] = msg[3];
				nb = 1;
			}else{
				if((msg[3] >> 3) & 1)
					buttons |= 1<<3;
				else if(msg[3] & 0x7)
					buttons |= 1<<4;
			}
		}
		dx = msg[1];
		dy = -msg[2];
		mousetrack(dx, dy, buttons, TK2MS(CPUS(0)->ticks));
	}
}

/*
 *  set up a ps2 mouse
 */
static void
ps2mouse(void)
{
	if(mousetype == MousePS2)
		return;

//	i8042auxenable(ps2mouseputc);
//	i8042auxcmd(0xEA);	// TODO
//	i8042auxcmd(0xF4);

	mousetype = MousePS2;
	packetsize = 3;
	mousehwaccel = 1;
}

/*
 * The PS/2 Trackpoint multiplexor on the IBM Thinkpad T23 ignores
 * acceleration commands.  It is supposed to pass them on
 * to the attached device, but my Logitech mouse is simply
 * not behaving any differently.  For such devices, we allow
 * the user to use "hwaccel off" to tell us to back off to
 * software acceleration even if we're using the PS/2 port.
 * (Serial mice are always software accelerated.)
 * For more information on the Thinkpad multiplexor, see
 * http://wwwcssrv.almaden.ibm.com/trackpoint/
 */
static void
setaccelerated(int x)
{
	accelerated = x;
	mouseaccelerate(x);
}

static void
setlinear(void)
{
	accelerated = 0;
	mouseaccelerate(0);
}

static void
setres(int n)
{
	resolution = n;
}

static void
setintellimouse(void)
{
	intellimouse = 1;
	packetsize = 4;
}

static void
resetmouse(void)
{
	packetsize = 3;
}

void
mousectl(Cmdbuf *cb)
{
	Cmdtab *ct;

	qlock(&mousectlqlock);
	if(waserror()){
		qunlock(&mousectlqlock);
		nexterror();
	}

	ct = lookupcmd(cb, mousectlmsg, nelem(mousectlmsg));
	switch(ct->index){
	case CMaccelerated:
		setaccelerated(cb->nf == 1? 1: atoi(cb->f[1]));
		break;
	case CMintellimouse:
		setintellimouse();
		break;
	case CMlinear:
		setlinear();
		break;
	case CMps2:
		intellimouse = 0;
		break;
	case CMps2intellimouse:
		setintellimouse();
		break;
	case CMres:
		if(cb->nf >= 2)
			setres(atoi(cb->f[1]));
		else
			setres(1);
		break;
	case CMreset:
		resetmouse();
		if(accelerated)
			setaccelerated(accelerated);
		if(resolution)
			setres(resolution);
		if(intellimouse)
			setintellimouse();
		break;
	case CMserial:
		error("serial mice not supported");
		break;
	case CMhwaccel:
		if(strcmp(cb->f[1], "on")==0)
			mousehwaccel = 1;
		else if(strcmp(cb->f[1], "off")==0)
			mousehwaccel = 0;
		else
			cmderror(cb, "bad mouse control message");
	}

	qunlock(&mousectlqlock);
	poperror();
}
