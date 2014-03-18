
struct Schedq
{
	Lock;
	Proc*	head;
	Proc*	tail;
	int	n;
};

// used to be in edf.h
//unused: extern Lock	edftestlock;	/* for atomic admitting/expelling */

#pragma	varargck	type	"t"		long
#pragma	varargck	type	"U"		uvlong
