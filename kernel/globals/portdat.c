#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

// devcons.c
int		(*print)(char*, ...) = 0;
int		(*iprint)(char*, ...) = 0;
int             (*pprint)(char *fmt, ...) = 0;
void		(*panic)(char*, ...) = 0;
void (*_assert)(char *fmt) = 0;
