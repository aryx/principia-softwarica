/*s: arch/arm/io.h */
/*s: enum _anon_ (arch/arm/io.h)(arm) */
// in dat_interrupts.h?
enum {
    IRQtimer0	= 0,
    IRQtimer1	= 1,
    IRQtimer2	= 2,
    IRQtimer3	= 3,
    IRQclock	= IRQtimer3,

    IRQusb		= 9, // also IRQfiq
    IRQdma0		= 16,
    // IRQdma1, IRQdma2, ... via IRQDMA macro below

    IRQaux		= 29,
    IRQi2c		= 53,
    IRQspi		= 54,
    IRQmmc		= 62,

    IRQbasic	= 64,
    IRQtimerArm	= IRQbasic + 0,

    IRQlocal	= 96,
    IRQcntps	= IRQlocal + 0,
    IRQcntpns	= IRQlocal + 1,

    IRQfiq		= IRQusb,	/* only one source can be FIQ */
};
/*e: enum _anon_ (arch/arm/io.h)(arm) */
/*s: macro IRQDMA(arm) */
#define IRQDMA(chan)	(IRQdma0+(chan))
/*e: macro IRQDMA(arm) */

/*s: enum _anon_ (arch/arm/io.h)2(arm) */
enum {
    DmaD2M		= 0,		/* device to memory */
    DmaM2D		= 1,		/* memory to device */
    DmaM2M		= 2,		/* memory to memory */

    DmaChanEmmc	= 4,		/* can only use 2-5, maybe 0 */
    DmaChanSpiTx= 2,
    DmaChanSpiRx= 0,

    DmaDevSpiTx	= 6,
    DmaDevSpiRx	= 7,
    DmaDevEmmc	= 11,
};
/*e: enum _anon_ (arch/arm/io.h)2(arm) */

/*s: enum _anon_ (arch/arm/io.h)3(arm) */
enum {
    PowerSd		= 0,
    PowerUart0,
    PowerUart1,
    PowerUsb,
    PowerI2c0,
    PowerI2c1,
    PowerI2c2,
    PowerSpi,
    PowerCcp2tx,
};
/*e: enum _anon_ (arch/arm/io.h)3(arm) */

/*s: enum _anon_ (arch/arm/io.h)4(arm) */
// in dat_time.h?
enum {
    ClkEmmc		= 1,
    ClkUart,
    ClkArm,
    ClkCore,
    ClkV3d,
    ClkH264,
    ClkIsp,
    ClkSdram,
    ClkPixel,
    ClkPwm,
};
/*e: enum _anon_ (arch/arm/io.h)4(arm) */
/*e: arch/arm/io.h */
