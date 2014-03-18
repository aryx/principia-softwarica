
/*
 *  things saved in the Proc structure during a notify
 */
struct Notsave
{
	ulong	svflags;
	ulong	svcs;
	ulong	svss;
};

/*
 *  routines for things outside the PC model, like power management
 */
// this is actually only used in 386/ code.
struct PCArch
{
	char*	id;
	int	(*ident)(void);		/* this should be in the model */
	void	(*reset)(void);		/* this should be in the model */
	int	(*serialpower)(int);	/* 1 == on, 0 == off */
	int	(*modempower)(int);	/* 1 == on, 0 == off */

	void	(*intrinit)(void);
	int	(*intrenable)(Vctl*);
	int	(*intrvecno)(int);
	int	(*intrdisable)(int);
	void	(*introff)(void);
	void	(*intron)(void);

	void	(*clockenable)(void);
	uvlong	(*fastclock)(uvlong*);
	void	(*timerset)(uvlong);

	void	(*resetothers)(void);	/* put other cpus into reset */
};

extern PCArch	*arch;			/* PC architecture */
