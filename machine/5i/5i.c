#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>
#include "arm.h"

#include <tos.h>

char*	file = "5.out";
Biobuf	bp, bi;
Fhdr	fhdr;

void
main(int argc, char **argv)
{

	argc--;
	argv++;

	bioout = &bp;
	bin = &bi;
	Binit(bioout, 1, OWRITE);
	Binit(bin, 0, OREAD);

	tlb.on = 1;
	tlb.tlbsize = 24;

	if(argc)
		file = argv[0];
	argc--;
	argv++;

	text = open(file, OREAD);
	if(text < 0)
		fatal(1, "open text '%s'", file);

	Bprint(bioout, "5i\n");
	inithdr(text);
	initstk(argc, argv);

	cmd();
}

void
initmap()
{
	ulong t, d, b, bssend;
	Segment *s;

	t = (fhdr.txtaddr+fhdr.txtsz+(BY2PG-1)) & ~(BY2PG-1);
	d = (t + fhdr.datsz + (BY2PG-1)) & ~(BY2PG-1);
	bssend = t + fhdr.datsz + fhdr.bsssz;
	b = (bssend + (BY2PG-1)) & ~(BY2PG-1);

	s = &memory.seg[Text];
	s->type = Text;
	s->base = fhdr.txtaddr - fhdr.hdrsz;
	s->end = t;
	s->fileoff = fhdr.txtoff - fhdr.hdrsz;
	s->fileend = s->fileoff + fhdr.txtsz;
	s->table = emalloc(((s->end-s->base)/BY2PG)*sizeof(uchar*));

	iprof = emalloc(((s->end-s->base)/PROFGRAN)*sizeof(long));
	textbase = s->base;

	s = &memory.seg[Data];
	s->type = Data;
	s->base = t;
	s->end = t+(d-t);
	s->fileoff = fhdr.datoff;
	s->fileend = s->fileoff + fhdr.datsz;
	datasize = fhdr.datsz;
	s->table = emalloc(((s->end-s->base)/BY2PG)*sizeof(uchar*));

	s = &memory.seg[Bss];
	s->type = Bss;
	s->base = d;
	s->end = d+(b-d);
	s->table = emalloc(((s->end-s->base)/BY2PG)*sizeof(uchar*));

	s = &memory.seg[Stack];
	s->type = Stack;
	s->base = STACKTOP-STACKSIZE;
	s->end = STACKTOP;
	s->table = emalloc(((s->end-s->base)/BY2PG)*sizeof(uchar*));

	reg.r[REGPC] = fhdr.entry;
}

void
inithdr(int fd)
{
	Symbol s;

	extern Machdata armmach;

	seek(fd, 0, 0);
	if (!crackhdr(fd, &fhdr))
		fatal(0, "read text header");

	if(fhdr.type != FARM )
		fatal(0, "bad magic number: %d %d", fhdr.type, FARM);

	if (syminit(fd, &fhdr) < 0)
		fatal(0, "%r\n");

	symmap = loadmap(symmap, fd, &fhdr);
	if (mach->sbreg && lookup(0, mach->sbreg, &s))
		mach->sb = s.value;
	machdata = &armmach;
}


void
initstk(int argc, char *argv[])
{
	ulong size;
	ulong sp, ap, tos;
	int i;
	char *p;

	initmap();
	tos = STACKTOP - sizeof(Tos)*2;	/* we'll assume twice the host's is big enough */
	sp = tos;
	for (i = 0; i < sizeof(Tos)*2; i++)
		putmem_b(tos + i, 0);

	/*
	 * pid is second word from end of tos and needs to be set for nsec().
	 * we know arm is a 32-bit cpu, so we'll assume knowledge of the Tos
	 * struct for now, and use our pid.
	 */
	putmem_w(tos + 4*4 + 2*sizeof(ulong) + 3*sizeof(uvlong), getpid());

	/* Build exec stack */
	size = strlen(file)+1+BY2WD+BY2WD+BY2WD;	
	for(i = 0; i < argc; i++)
		size += strlen(argv[i])+BY2WD+1;

	sp -= size;
	sp &= ~7;
	reg.r[0] = tos;
	reg.r[13] = sp;
	reg.r[1] = STACKTOP-4;	/* Plan 9 profiling clock (why & why in R1?) */

	/* Push argc */
	putmem_w(sp, argc+1);
	sp += BY2WD;

	/* Compute sizeof(argv) and push argv[0] */
	ap = sp+((argc+1)*BY2WD)+BY2WD;
	putmem_w(sp, ap);
	sp += BY2WD;
	
	/* Build argv[0] string into stack */
	for(p = file; *p; p++)
		putmem_b(ap++, *p);

	putmem_b(ap++, '\0');

	/* Loop through pushing the arguments */
	for(i = 0; i < argc; i++) {
		putmem_w(sp, ap);
		sp += BY2WD;
		for(p = argv[i]; *p; p++)
			putmem_b(ap++, *p);
		putmem_b(ap++, '\0');
	}
	/* Null terminate argv */
	putmem_w(sp, 0);

}
