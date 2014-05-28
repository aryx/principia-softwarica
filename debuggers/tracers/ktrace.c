#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>

#include <mach.h>

// ktrace - interpret kernel stack dumps

static	int	rtrace(uvlong, uvlong, uvlong);
static	int	i386trace(uvlong, uvlong, uvlong);

static	uvlong	getval(uvlong);
static	void	inithdr(int);
static	void	fatal(char*, ...);
static	void	readstack(void);

static	Fhdr	fhdr;
static	int	interactive = 0;

#define	FRAMENAME	".frame"

static void
usage(void)
{
	fprint(2, "usage: ktrace [-i] kernel pc sp [link]\n");
	exits("usage");
}

static void
printaddr(char *addr, uvlong pc)
{
	int i;
	char *p;

	/*
	 * reformat the following.
	 *
	 * foo+1a1 -> src(foo+0x1a1);
	 * 10101010 -> src(0x10101010);
	 */

	if(strlen(addr) == 8 && strchr(addr, '+') == nil){
		for(i=0; i<8; i++)
			if(!isxdigit(addr[i]))
				break;
		if(i == 8){
			print("src(%#.8llux); // 0x%s\n", pc, addr);
			return;
		}
	}

	if(p=strchr(addr, '+')){
		*p++ = 0;
		print("src(%#.8llux); // %s+0x%s\n", pc, addr, p);
	}else
		print("src(%#.8llux); // %s\n", pc, addr);
}

static void (*fmt)(char*, uvlong) = printaddr;

void
main(int argc, char *argv[])
{
	int (*t)(uvlong, uvlong, uvlong);
	uvlong pc, sp, link;
	int fd;

	ARGBEGIN{
	case 'i':
		interactive = 1;
		break;
	default:
		usage();
	}ARGEND

	link = 0;
	t = rtrace;
	switch(argc){
	case 4:
		t = rtrace;
		link = strtoull(argv[3], 0, 16);
		break;
	case 3:
		break;
	default:
		usage();
	}
	pc = strtoull(argv[1], 0, 16);
	sp = strtoull(argv[2], 0, 16);
	if(!interactive)
		readstack();

	fd = open(argv[0], OREAD);
	if(fd < 0)
		fatal("can't open %s: %r", argv[0]);
	inithdr(fd);
	switch(fhdr.magic){
	case I_MAGIC:	/* intel 386 */
		t = i386trace;
		break;
	case V_MAGIC:	/* mips 3000 */
	case M_MAGIC:	/* mips 4000 */
	case E_MAGIC:	/* arm 7-something */
	case N_MAGIC:	/* mips 4000 LE */
		t = rtrace;
		break;
	default:
		fprint(2, "%s: warning: can't tell what type of stack %s uses; assuming it's %s\n",
			argv0, argv[0], argc == 4 ? "risc" : "cisc");
		break;
	}
	(*t)(pc, sp, link);
	exits(0);
}

static void
inithdr(int fd)
{
	seek(fd, 0, 0);
	if(!crackhdr(fd, &fhdr))
		fatal("read text header");

	if(syminit(fd, &fhdr) < 0)
		fatal("%r\n");
}

// for MIPS and ARM
static int
rtrace(uvlong pc, uvlong sp, uvlong link)
{
	Symbol s, f;
	char buf[128];
	uvlong oldpc;
	int i;

	i = 0;
	while(findsym(pc, CTEXT, &s)) {
		if(pc == s.value)	/* at first instruction */
			f.value = 0;
		else if(findlocal(&s, FRAMENAME, &f) == 0)
			break;

		symoff(buf, sizeof buf, pc, CANY);
		fmt(buf, pc);

		oldpc = pc;
		if(s.type == 'L' || s.type == 'l' || pc <= s.value+mach->pcquant){
			if(link == 0)
				fprint(2, "%s: need to supply a valid link register\n", argv0);
			pc = link;
		}else{
			pc = getval(sp);
			if(pc == 0)
				break;
		}

		if(pc == 0 || (pc == oldpc && f.value == 0))
			break;

		sp += f.value;

		if(++i > 40)
			break;
	}
	return i;
}

static int
i386trace(uvlong pc, uvlong sp, uvlong link)
{
	int i;
	uvlong osp;
	Symbol s, f;
	char buf[128];

	USED(link);
	i = 0;
	osp = 0;
	while(findsym(pc, CTEXT, &s)) {

		symoff(buf, sizeof buf, pc, CANY);
		fmt(buf, pc);

		if(pc != s.value) {	/* not at first instruction */
			if(findlocal(&s, FRAMENAME, &f) == 0)
				break;
			sp += f.value-mach->szaddr;
		}else if(strcmp(s.name, "forkret") == 0){
			print("//passing interrupt frame; last pc found at sp=%#llux\n", osp);
			sp +=  15 * mach->szaddr;		/* pop interrupt frame */
		}

		pc = getval(sp);
		if(pc == 0 && strcmp(s.name, "forkret") == 0){
			sp += 3 * mach->szaddr;			/* pop iret eip, cs, eflags */
			print("//guessing call through invalid pointer, try again at sp=%#llux\n", sp);
			s.name = "";
			pc = getval(sp);
		}
		if(pc == 0) {
			print("//didn't find pc at sp=%#llux, last pc found at sp=%#llux\n", sp, osp);
			break;
		}
		osp = sp;

		sp += mach->szaddr;
		if(strcmp(s.name, "forkret") == 0)
			sp += 2 * mach->szaddr;			/* pop iret cs, eflags */

		if(++i > 40)
			break;
	}
	return i;
}

int naddr;
uvlong addr[1024];
uvlong val[1024];

static void
putval(uvlong a, uvlong v)
{
	if(naddr < nelem(addr)){
		addr[naddr] = a;
		val[naddr] = v;
		naddr++;
	}
}

static void
readstack(void)
{
	Biobuf b;
	char *p;
	char *f[64];
	int nf, i;

	Binit(&b, 0, OREAD);
	while(p=Brdline(&b, '\n')){
		p[Blinelen(&b)-1] = 0;
		nf = tokenize(p, f, nelem(f));
		for(i=0; i<nf; i++){
			if(p=strchr(f[i], '=')){
				*p++ = 0;
				putval(strtoull(f[i], 0, 16), strtoull(p, 0, 16));
			}
		}
	}
}

static uvlong
getval(uvlong a)
{
	char buf[256];
	int i, n;
	uvlong r;

	if(interactive){
		print("// data at %#8.8llux? ", a);
		n = read(0, buf, sizeof(buf)-1);
		if(n <= 0)
			return 0;
		buf[n] = '\0';
		r = strtoull(buf, 0, 16);
	}else{
		r = 0;
		for(i=0; i<naddr; i++)
			if(addr[i] == a)
				r = val[i];
	}

	return r;
}

static void
fatal(char *fmt, ...)
{
	char buf[4096];
	va_list arg;

	va_start(arg, fmt);
	vseprint(buf, buf+sizeof(buf), fmt, arg);
	va_end(arg);
	fprint(2, "ktrace: %s\n", buf);
	exits(buf);
}
