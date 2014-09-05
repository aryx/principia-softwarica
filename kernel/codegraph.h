
// this is only for codegraph

// see also include/*.h and lib_core/libc/9syscall/sys.h

// tos.h, pragma incomplete
struct Plink {
};

// defined by -D in Makefile
#define KERNDATE "today"

// never defined on x86
struct KMap {
};

// defined in user/preboot/ which is skipped because depends on other stack
uchar initcode[]={
    0x00
};

// when add lib_graphics/
#define fprint(x) x
#define strdup(x) x
#define abort(x) x
