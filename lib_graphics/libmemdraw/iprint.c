#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

int
memdraw_iprint(char*,...)
{
	return -1;
}

int		(*iprint)(char*, ...) = &memdraw_iprint;

