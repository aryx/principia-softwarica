/*s: include/bootexec.h */
/*s: struct coffsect */
struct coffsect
{
    char	name[8];
    ulong	phys;
    ulong	virt;
    ulong	size;
    ulong	fptr;
    ulong	fptrreloc;
    ulong	fptrlineno;
    ulong	nrelocnlineno;
    ulong	flags;
};
/*e: struct coffsect */

/*s: struct i386exec */
struct i386exec
{
    struct	i386coff{
        ulong	isectmagic;
        ulong	itime;
        ulong	isyms;
        ulong	insyms;
        ulong	iflags;
    };
    struct	i386hdr{
        ulong	imagic;
        ulong	itextsize;
        ulong	idatasize;
        ulong	ibsssize;
        ulong	ientry;
        ulong	itextstart;
        ulong	idatastart;
    };
    struct coffsect itexts;
    struct coffsect idatas;
    struct coffsect ibsss;
    struct coffsect icomments;
};
/*e: struct i386exec */
/*e: include/bootexec.h */
