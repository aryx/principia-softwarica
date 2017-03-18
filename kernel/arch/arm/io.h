/*s: arch/arm/io.h */
// could be in dat_interrupts.h?
/*s: enum IRQ(arm) */
enum IRQ {
    IRQtimer0   = 0, // reserved by GPU?
    IRQtimer1   = 1, // reserved by GPU?
    IRQtimer2   = 2, // reserved by GPU?
    IRQtimer3   = 3,

    IRQclock    = IRQtimer3,

    IRQusb      = 9,  // also IRQfiq
    IRQdma0     = 16, // IRQdma1, IRQdma2, ... via IRQDMA() macro

    IRQaux      = 29,
    IRQi2c      = 53,
    IRQspi      = 54,
    IRQmmc      = 62,

    /*s: [[IRQ]] basic cases(arm) */
    IRQbasic    = 64,
    IRQtimerArm = IRQbasic + 0,
    /*e: [[IRQ]] basic cases(arm) */
    /*s: [[IRQ]] local cases(arm) */
    IRQlocal    = 96,
    IRQcntps    = IRQlocal + 0,
    IRQcntpns   = IRQlocal + 1,
    /*e: [[IRQ]] local cases(arm) */
    /*s: [[IRQ]] other cases(arm) */
    IRQfiq      = IRQusb,   /* only one source can be FIQ */
    /*e: [[IRQ]] other cases(arm) */
};
/*e: enum IRQ(arm) */
/*s: macro IRQDMA(arm) */
#define IRQDMA(chan)    (IRQdma0+(chan))
/*e: macro IRQDMA(arm) */

/*s: enum Dma(arm) */
enum DmaFlags {
    DmaD2M      = 0,        /* device to memory */
    DmaM2D      = 1,        /* memory to device */
    DmaM2M      = 2,        /* memory to memory */

    DmaChanEmmc = 4,        /* can only use 2-5, maybe 0 */
    DmaChanSpiTx= 2,
    DmaChanSpiRx= 0,

    DmaDevSpiTx = 6,
    DmaDevSpiRx = 7,
    DmaDevEmmc  = 11,
};
/*e: enum Dma(arm) */

/*s: enum Power(arm) */
enum {
    PowerSd     = 0,
    PowerUart0,
    PowerUart1,
    PowerUsb,
    PowerI2c0,
    PowerI2c1,
    PowerI2c2,
    PowerSpi,
    PowerCcp2tx,
};
/*e: enum Power(arm) */

// could be in dat_time.h?
/*s: enum Clock(arm) */
enum {
    ClkEmmc     = 1,
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
/*e: enum Clock(arm) */
/*e: arch/arm/io.h */
