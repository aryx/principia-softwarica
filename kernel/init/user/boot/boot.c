/*s: boot.c */
#include <u.h>
#include <libc.h>
#include "../boot/boot.h"

// Note that most of this code is now superseded by $objtype/boot.rc

/*
 * we should inherit the standard fds referring to /dev/cons,
 * but we're being paranoid.
 */
static void
opencons(void)
{
  close(STDIN);
  close(STDOUT);
  close(STDERR);
  open("/dev/cons", OREAD);
  open("/dev/cons", OWRITE);
  open("/dev/cons", OWRITE);
}

/*
 * init will reinitialize its namespace.
 * #ec gets us plan9.ini settings (*var variables).
 */
static void
bindenvsrv(void)
{
  bind("#ec", "/env", MREPL); // ec? 2 chars? #e and pass 'c' to device?
  bind("#e", "/env", MBEFORE|MCREATE); // devenv
  bind("#s", "/srv/", MREPL|MCREATE); // devsrv
}

//TODO: use open_safe, write_safe
static void
swapproc(void)
{
    int fd;

    fd = open("#c/swap", OWRITE);
    if(fd < 0){
        warning("opening #c/swap");
        return;
    }
    if(write(fd, "start", 5) <= 0)
        warning("starting swap kproc");
    close(fd);
}

static void
execinit(void)
{
  fdt fd;

  // basics

  bind_safe("#p", "/proc", MREPL); //devproc
  // used by rc and many programs, e.g. via open("#d/0")
  bind_safe("#d", "/fd", MREPL); //devdup
  // can't use sys, because too much code assumes /sys/ have an include/, src/
  bind_safe("#k", "/ksys", MREPL); //devsys

  bind_safe("/root", "/", MAFTER|MCREATE);

  bind_safe("/386/bin", "/bin", MREPL); // X86

  bind_safe("/rc/bin", "/bin", MAFTER);

  bind_safe("#P", "/dev", MAFTER); //devarch

  bind_safe("#m", "/dev", MAFTER); //devmouse
  run("/bin/mouse", "ps2", nil);


  // for draw

  bind_safe("#v", "/dev", MAFTER); //devvga
  //this just need a regular vga driver
  run("/bin/vga", "-l", "640x480x8", nil);
  //this need special drivers, such as the clgd424x.c in the kernel
  //run("/bin/vga", "-l", "1024x768x8", nil); // can add -V to debug vga
  bind_safe("#i", "/dev", MAFTER); // devdraw

  // for rio

  run("/bin/ramfs", "-m", "/mnt", nil);
  run("/bin/mkdir", "/mnt/temp", nil); // see thread(2), used to create pipes
  run("/bin/mkdir", "/mnt/wsys", nil);
  fd = open_safe("#k/hostowner", OWRITE);
  print_safe(fd, "pad");
  close(fd);

  // network

  bind_safe("#I", "/net", MREPL); // devip
  bind_safe("#l0", "/net", MAFTER); // ether (and dev 0)

  // for 8c, 8a, 8l

  putenv("objtype", "386");

  // I have a bug in the kernel where I can't get the current
  // date from qemu. At boot time there is some lapic clock error.
  // So then the date is set to 0sec since epoch which is 1970
  // which is bad for tools like mk because many files are in the futur.
  // So here I set the date to 2033 so mk should be happy
  fd = open_safe("#c/time", OWRITE);
  print_safe(fd, "2000000000");
  close(fd);

  // should normally run /root/init, but prefer to simply run rc for now
  run("/bin/rc", nil);
}


// called from boot$CONF.c:main()
void
boot(int argc, char *argv[])
{
  USED(argc);
  USED(argv);

  fmtinstall('r', errfmt);

  // at this point we should have #/ and #c setup by the kernel init0

  opencons();
  bindenvsrv();

  print("booooooooting...\n");

  rfork(RFNAMEG);

  connectlocal();

  swapproc();
  execinit();

  exits("failed to exec init");
}
/*e: boot.c */
