
/*
 * bitmap of how to replicate n bits to fill 8, for 1 ≤ n ≤ 8.
 * the X's are where to put the bottom (ones) bit of the n-bit pattern.
 * only the top 8 bits of the result are actually used.
 * (the lower 8 bits are needed to get bits in the right place
 * when n is not a divisor of 8.)
 *
 * Should check to see if its easier to just refer to replmul than
 * use the precomputed values in replbit.  On PCs it may well
 * be; on machines with slow multiply instructions it probably isn't.
 */
#define a ((((((((((((((((0
#define X *2+1)
#define _ *2)
//static 
int replmul[1+8] = {
	0,
	a X X X X X X X X X X X X X X X X,
	a _ X _ X _ X _ X _ X _ X _ X _ X,
	a _ _ X _ _ X _ _ X _ _ X _ _ X _,
	a _ _ _ X _ _ _ X _ _ _ X _ _ _ X,
	a _ _ _ _ X _ _ _ _ X _ _ _ _ X _,
	a _ _ _ _ _ X _ _ _ _ _ X _ _ _ _, 
	a _ _ _ _ _ _ X _ _ _ _ _ _ X _ _,
	a _ _ _ _ _ _ _ X _ _ _ _ _ _ _ X,
};
#undef a
#undef X
#undef _
