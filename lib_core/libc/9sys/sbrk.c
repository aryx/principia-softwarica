#include <u.h>
#include <libc.h>

extern	char	end[];

// starting point for the heap, after the text, data, and bss.
static	char	*bloc = { end };

// the syscall
extern	int	brk(void*);

enum
{
	Round	= 7
};

void*
sbrk(ulong n)
{
	uintptr bl;

	bl = ((uintptr)bloc + Round) & ~Round;
	if(brk((void*)(bl+n)) < 0)
		return (void*)-1;
	bloc = (char*)bl + n;
	return (void*)bl;
}
