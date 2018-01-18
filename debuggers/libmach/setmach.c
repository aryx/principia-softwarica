/*s: libmach/setmach.c */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>
#include	<mach.h>
        /* table for selecting machine-dependent parameters */

typedef	struct machtab Machtab;

/*s: struct [[machtab]] */
struct machtab
{
    char	*name;			/* machine name */
    short	type;			/* executable type */
    short	boottype;		/* bootable type */
    int		asstype;		/* disassembler code */
    Mach	*mach;			/* machine description */
    Machdata	*machdata;		/* machine functions */
};
/*e: struct [[machtab]] */

extern	Mach		mi386, marm;
extern	Machdata	i386mach, armmach;

/*s: global [[machines]] */
/*
 *	machine selection table.  machines with native disassemblers should
 *	follow the plan 9 variant in the table; native modes are selectable
 *	only by name.
 */
Machtab	machines[] =
{
    {	"386",				/*plan 9 386*/
        FI386,
        FI386B,
        AI386,
        &mi386,
        &i386mach,	},
    {	"arm",				/*ARM*/
        FARM,
        FARMB,
        AARM,
        &marm,
        &armmach,	},
    {	0		},		/*the terminator*/
};
/*e: global [[machines]] */

/*s: function [[machbytype]] */
/*
 *	select a machine by executable file type
 */
void
machbytype(int type)
{
    Machtab *mp;

    for (mp = machines; mp->name; mp++){
        if (mp->type == type || mp->boottype == type) {
            asstype = mp->asstype;
            machdata = mp->machdata;
            break;
        }
    }
}
/*e: function [[machbytype]] */
/*s: function [[machbyname]] */
/*
 *	select a machine by name
 */
int
machbyname(char *name)
{
    Machtab *mp;

    if (!name) {
        asstype = AARM;
        machdata = &armmach;
        mach = &marm;
        return 1;
    }
    for (mp = machines; mp->name; mp++){
        if (strcmp(mp->name, name) == 0) {
            asstype = mp->asstype;
            machdata = mp->machdata;
            mach = mp->mach;
            return 1;
        }
    }
    return 0;
}
/*e: function [[machbyname]] */
/*e: libmach/setmach.c */
