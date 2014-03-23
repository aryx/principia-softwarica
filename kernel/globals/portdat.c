#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

// was in init/main.c
Mach *m;
Conf conf;

// was in conf/pcf.c
Dev** devtab = 0;

// was in security/auth.c
char	*eve;
// should be in portfns.c, but then backward deps
iseve(void) { return strcmp(eve, up->user) == 0; }

// was in dat.h
Mach* machp[MAXMACH];
