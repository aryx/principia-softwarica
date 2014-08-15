
// this is only for codegraph

// sys.h symlink is also here for that.

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
