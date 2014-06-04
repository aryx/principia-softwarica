/*s: l_multiboot.s */
#include "mem.h"
        
TEXT _start(SB), $0

/*s: global _multibootheader */
/*
 * Must be 4-byte aligned.
 */
TEXT _multibootheader(SB), $0
        LONG    $0x1BADB002                     /* magic */
        LONG    $0x00010003                     /* flags */
        LONG    $-(0x1BADB002 + 0x00010003)     /* checksum */
        
        LONG    $_multibootheader-KZERO(SB)     /* header_addr */
        LONG    $_start-KZERO(SB)          /* load_addr */
        LONG    $edata-KZERO(SB)                /* load_end_addr */
        LONG    $end-KZERO(SB)                  /* bss_end_addr */
        
//      !!!entry point specification!!!
        LONG    $_multibootentry-KZERO(SB)              /* entry_addr */
        
        LONG    $0                              /* mode_type */
        LONG    $0                              /* width */
        LONG    $0                              /* height */
        LONG    $0                              /* depth */
/*e: global _multibootheader */

/*s: function _multibootentry */
/* 
 * the kernel expects the data segment to be page-aligned
 * multiboot bootloaders put the data segment right behind text
 */
TEXT _multibootentry(SB), $0
        MOVL    $etext-KZERO(SB), SI
        MOVL    SI, DI
        ADDL    $0xfff, DI
        ANDL    $~0xfff, DI
        MOVL    $edata-KZERO(SB), CX
        SUBL    DI, CX
        ADDL    CX, SI
        ADDL    CX, DI
        STD
        REP; MOVSB
        CLD
        ADDL    $KZERO, BX
        MOVL    BX, multiboot-KZERO(SB)
//      !!! Jump !!!
        MOVL    $_setup_segmentation(SB), AX
        ANDL    $~KZERO, AX
        JMP*    AX
/*e: function _multibootentry */

/*s: global multiboot */
/* multiboot structure pointer */
TEXT multiboot(SB), $0
        LONG    $0
/*e: global multiboot */
/*e: l_multiboot.s */
