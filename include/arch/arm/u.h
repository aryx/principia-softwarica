/*s: include/arch/arm/u.h */

/*s: type uxxx */
typedef unsigned short  ushort;
typedef unsigned char   uchar;
typedef unsigned long   ulong;
typedef unsigned int    uint;
/*e: type uxxx */

/*s: type uxxxint */
typedef unsigned char u8int;
typedef unsigned short u16int;
typedef unsigned int    u32int;
typedef unsigned long long u64int;
/*e: type uxxxint */

/*s: type xxxvlong */
typedef long long   vlong;
typedef unsigned long long uvlong;
/*e: type xxxvlong */

typedef signed char schar;
/*s: type usize */
typedef unsigned long   usize;
/*e: type usize */

/*s: constant nil */
#define nil     ((void*)0)
/*e: constant nil */
/*s: type uintptr */
typedef unsigned long   uintptr;
/*e: type uintptr */

/*s: type Rune */
typedef uint        Rune;
/*e: type Rune */
/*s: type mpdigit */
typedef unsigned int    mpdigit;    /* for /sys/include/mp.h */
/*e: type mpdigit */

/*s: type jmpbufxxx */
#define JMPBUFSP    0
#define JMPBUFPC    1
#define JMPBUFDPC   0
/*e: type jmpbufxxx */
/*s: type jmp_buf */
typedef long    jmp_buf[2];
/*e: type jmp_buf */

/*s: type FPxxx */
/* VFP FCR */
#define FPINEX  (1<<12)         /* trap enables for exceptions */
#define FPUNFL  (1<<11)
#define FPOVFL  (1<<10)
#define FPZDIV  (1<<9)
#define FPINVAL (1<<8)
#define FPRNR   (0<<22)
#define FPRZ    (1<<22)
#define FPRPINF (2<<22)
#define FPRNINF (3<<22)
#define FPRMASK (3<<22)
#define FPPEXT  0
#define FPPSGL  0
#define FPPDBL  0
#define FPPMASK 0
/* FSR */
#define FPAINEX (1<<4)          /* accrued exceptions */
#define FPAUNFL (1<<3)
#define FPAOVFL (1<<2)
#define FPAZDIV (1<<1)
#define FPAINVAL    (1<<0)
/*e: type FPxxx */
/*s: type FPdbleword */
union FPdbleword
{
    double  x;

    struct {    /* little endian */
        ulong lo;
        ulong hi;
    };
};
/*e: type FPdbleword */
typedef     union FPdbleword FPdbleword;

/*s: type va_list */
typedef char*   va_list;
/*e: type va_list */

/*s: macro va_start */
#define va_start(list, start) list =\
    (sizeof(start) < 4?\
        (char*)((int*)&(start)+1):\
        (char*)(&(start)+1))
/*e: macro va_start */
/*s: macro va_end */
#define va_end(list)\
    USED(list)
/*e: macro va_end */
/*s: macro va_arg */
#define va_arg(list, mode)\
    ((sizeof(mode) == 1)?\
        ((list += 4), (mode*)list)[-4]:\
    (sizeof(mode) == 2)?\
        ((list += 4), (mode*)list)[-2]:\
        ((list += sizeof(mode)), (mode*)list)[-1])
/*e: macro va_arg */

/*e: include/arch/arm/u.h */
