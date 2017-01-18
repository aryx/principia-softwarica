/*
 * bcm2835 i2c controller
 *
 *	Only i2c1 is supported.
 *	i2c2 is reserved for HDMI.
 *	i2c0 SDA0/SCL0 pins are not routed to P1 connector (except for early Rev 0 boards)
 */

#include	"u.h"
#include	"../port/lib.h"
#include	"../port/error.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"

#define I2CREGS	(VIRTIO+0x804000)
#define SDA0Pin	2
#define	SCL0Pin	3
#define	Alt0	0x4

typedef struct I2c I2c;
typedef struct Bsc Bsc;

/*
 * Registers for Broadcom Serial Controller (i2c compatible)
 */
struct Bsc {
	u32int	ctrl;
	u32int	stat;
	u32int	dlen;
	u32int	addr;
	u32int	fifo;
	u32int	clkdiv;		/* default 1500 => 100 KHz assuming 150Mhz input clock */
	u32int	delay;		/* default (48<<16)|48 falling:rising edge */
	u32int	clktimeout;	/* default 64 */
};

/*
 * Per-controller info
 */
struct I2c {
	QLock	lock;
	Lock	reglock;
	Rendez	r;
	Bsc	*regs;
};

static I2c i2c;

enum {
	/* ctrl */
	I2cen	= 1<<15,
	Intr	= 1<<10,
	Intt	= 1<<9,
	Intd	= 1<<8,
	Start	= 1<<7,
	Clear	= 1<<4,
	Read	= 1<<0,
	Write	= 0<<0,

	/* stat */
	Clkt	= 1<<9,
	Err	= 1<<8,
	Rxf	= 1<<7,
	Txe	= 1<<6,
	Rxd	= 1<<5,
	Txd	= 1<<4,
	Rxr	= 1<<3,
	Txw	= 1<<2,
	Done	= 1<<1,
	Ta	= 1<<0,
};

static void
i2cinterrupt(Ureg*, void*)
{
	Bsc *r;
	int st;

	r = i2c.regs;
	st = 0;
	if((r->ctrl & Intr) && (r->stat & Rxd))
		st |= Intr;
	if((r->ctrl & Intt) && (r->stat & Txd))
		st |= Intt;
	if(r->stat & Done)
		st |= Intd;
	if(st){
		r->ctrl &= ~st;
		wakeup(&i2c.r);
	}
}

static int
i2cready(void *st)
{
	return (i2c.regs->stat & (uintptr)st);
}

static void
i2cinit(void)
{
	i2c.regs = (Bsc*)I2CREGS;
	i2c.regs->clkdiv = 2500;
	gpiosel(SDA0Pin, Alt0);
	gpiosel(SCL0Pin, Alt0);
	gpiopullup(SDA0Pin);
	gpiopullup(SCL0Pin);
	intrenable(IRQi2c, i2cinterrupt, 0, 0, "i2c");
}

static void
i2cio(int rw, uint addr, void *buf, int len)
{
	Bsc *r;
	uchar *p;
	int st;

	qlock(&i2c.lock);
	if(i2c.regs == 0)
		i2cinit();
	r = i2c.regs;
	p = buf;
	r->ctrl = I2cen | Clear;
	r->addr = addr;
	r->dlen = len;
	r->stat = Clkt|Err|Done;
	r->ctrl = I2cen | Start | Intd | rw;
	st = rw == Read? Rxd : Txd;
	while(len > 0){
		while((r->stat & (st|Done)) == 0){
			r->ctrl |= rw == Read? Intr : Intt;
			sleep(&i2c.r, i2cready, (void*)(st|Done));
		}
		if(r->stat & (Err|Clkt)){
			qunlock(&i2c.lock);
			error(Eio);
		}
		if(rw == Read){
			do{
				*p++ = r->fifo;
				len--;
			}while ((r->stat & Rxd) && len > 0);
		}else{
			do{
				r->fifo = *p++;
				len--;
			}while((r->stat & Txd) && len > 0);
		}
	}
	while((r->stat & Done) == 0)
		sleep(&i2c.r, i2cready, (void*)Done);
	if(r->stat & (Err|Clkt)){
		qunlock(&i2c.lock);
		error(Eio);
	}
	r->ctrl = 0;
	qunlock(&i2c.lock);
}

void
i2cread(uint addr, void *buf, int len)
{
	i2cio(Read, addr, buf, len);
}

void
i2cwrite(uint addr, void *buf, int len)
{
	i2cio(Write, addr, buf, len);
}
