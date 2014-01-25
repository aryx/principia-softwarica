#include <u.h>
#include <libc.h>
#include <draw.h>

// from [9fans] bug report
void
main(int argc, char *argv[])
{
	char buf[12*12+1];

	buf[12*12] = 'X';
	if(read(0, buf, 12*12) != 12*12)
		sysfatal("read: %r");
	if(buf[12*12] != 'X')
		sysfatal("corrupt");
}
