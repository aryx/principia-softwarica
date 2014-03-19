#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"

void (*proctrace)(Proc*, int, vlong) = 0; // was in devproc.c

struct Active active;
