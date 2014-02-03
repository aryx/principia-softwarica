#include	<u.h>
#include	<libc.h>

int
libc_print(char *fmt, ...)
{
	int n;
	va_list args;

	va_start(args, fmt);
	n = vfprint(1, fmt, args);
	va_end(args);
	return n;
}

int (*print)(char *fmt, ...) = &libc_print;
