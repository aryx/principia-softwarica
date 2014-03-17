
struct Schedq
{
	Lock;
	Proc*	head;
	Proc*	tail;
	int	n;
};
