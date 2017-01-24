#include	"u.h"
#include	"../port/lib.h"
#include	"../port/error.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"

// callback for portkbd.c
void
arch_setleds(Kbscan *kbscan)
{
  USED(kbscan);
  return;
}
