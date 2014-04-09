#include <u.h>
#include <libc.h>
#include "../boot/boot.h"

/*
 * we should inherit the standard fds all referring to /dev/cons,
 * but we're being paranoid.
 */
static void
opencons(void)
{
  close(0);
  close(1);
  close(2);
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
  bind("#ec", "/env", MREPL);
  bind("#e", "/env", MBEFORE|MCREATE);
  bind("#s", "/srv/", MREPL|MCREATE);
}

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
  int fd;

  bind_safe("#p", "/proc", MREPL);
  bind_safe("#d", "/fd", MREPL);

  bind_safe("/root", "/", MAFTER|MCREATE);
  bind_safe("/386/bin", "/bin", MREPL);
  bind_safe("/rc/bin", "/bin", MAFTER);

  bind_safe("#v", "/dev", MAFTER);
  bind_safe("#m", "/dev", MAFTER);
  bind_safe("#P", "/dev", MAFTER);

  run("/bin/aux/mouse", "ps2", nil);
  //this just need a regular vga driver
  //run("/bin/aux/vga", "-l", "640x480x8", nil);
  //this need special drivers, such as the clgd424x.c in the kernel
  run("/bin/aux/vga", "-l", "1024x768x8", nil);
  bind_safe("#i", "/dev", MAFTER);

  // for rio
  run("/bin/ramfs", "-m", "/mnt", nil);
  run("/bin/mkdir", "/mnt/temp", nil); // see thread(2), used to create pipes
  run("/bin/mkdir", "/mnt/wsys", nil);
  fd = open_safe("#c/hostowner", OWRITE);
  print_safe(fd, "pad");
  close(fd);

  // network
  bind_safe("#I", "/net", MREPL);
  bind_safe("#l0", "/net", MAFTER);

  run("/bin/rc", nil);
}


// called from bootbcf.c:main()
void
boot(int argc, char *argv[])
{
  USED(argc);
  USED(argv);

  fmtinstall('r', errfmt);

  //At this point we should have #/ and #c setup by the kernel init0

  opencons();
  bindenvsrv();

  print("booooooooting...\n");

  rfork(RFNAMEG);

  connectlocal();

  swapproc();
  execinit();

  exits("failed to exec init");
}
