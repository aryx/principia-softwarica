
// this is only for codegraph or syncweb's indexer

// see also include/*.h and lib_core/libc/9syscall/sys.h

// tos.h, pragma incomplete defined in lib_core/libc/port/profile.c
struct Plink {
};

// defined by -D in Makefile
#define KERNDATE "today"

// never defined on x86
struct Arch_KMap {
};

// defined in user/preboot/ which is skipped because depends on other stack
uchar initcode[]={
    0x00
};

// when add lib_graphics/
//#define fprint(x) x
//#define strdup(x) x
//#define abort(x) x
