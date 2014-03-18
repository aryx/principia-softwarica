#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

// init/main.c
Mach *m;
Conf conf;

// conf/pcf.c
Dev** devtab = 0;

// security/auth.c
char	*eve;
// should be in portfns.c, but then backward deps
iseve(void) { return strcmp(eve, up->user) == 0; }
