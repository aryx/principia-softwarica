/*s: local.c */
#include <u.h>
#include <libc.h>
#include "../boot/boot.h"

//void
//configlocal(Method *mp)
//{
//  USED(mp);
//}

int
connectlocal(void)
{
  int fd;
  
  bind_safe("#S", "/dev", MAFTER);

  fd = open_safe("/dev/sdC0/ctl", ORDWR);
  //TODO: use fdisk -p /dev/sdC1/data > /dev/sdC1/ctl
  //for sdC0: #prep -p /dev/sdC1/plan9 > /dev/sdC1/ctl
  print_safe(fd, "part dos 1 1000063");
  close_safe(fd);

  run("/boot/dossrv", nil);
  run("/boot/mount", "-c", "/srv/dos", "/root", "/dev/sdC0/dos", nil);
  
  return 0;
}
/*e: local.c */
