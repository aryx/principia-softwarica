/*s: misc/tprof.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

/*s: constant PCRES (misc/tprof.c) */
#define	PCRES	8
/*e: constant PCRES (misc/tprof.c) */

/*s: struct COUNTER (misc/tprof.c) */
struct COUNTER
{
    char 	*name;		/* function name */
    long	time;		/* ticks spent there */
};
/*e: struct COUNTER (misc/tprof.c) */

/*s: function error (misc/tprof.c) */
static void
error(bool perr, char *s)
{
    fprint(STDERR, "tprof: %s", s);
    if(perr){
        fprint(STDERR, ": ");
        perror(0);
    }else
        fprint(STDERR, "\n");
    exits(s);
}
/*e: function error (misc/tprof.c) */

/*s: function compar (misc/tprof.c) */
int
compar(void *va, void *vb)
{
    struct COUNTER *a, *b;

    a = va;
    b = vb;
    if(a->time < b->time)
        return -1;
    if(a->time == b->time)
        return 0;
    return 1;
}
/*e: function compar (misc/tprof.c) */
/*s: function main (misc/tprof.c) */
void
main(int argc, char *argv[])
{
    // for symbol table
    fdt fd; // /proc/<pid>/text and then /proc/<pid>/profile
    Fhdr f;
    // for profile data
    char filebuf[128], *file; // /proc/<pid>/profile
    Dir *d;
    ulong *data; // profile content
    // for output
    Biobuf outbuf;
    /*s: [[main()]](tprof.c) other locals */
    long i, j, k, n;
    char *name;
    ulong tbase, sum;
    long delta;
    Symbol s;
    struct COUNTER *cp;
    /*e: [[main()]](tprof.c) other locals */

    /*s: [[main()]](tprof.c) sanity check argc and print usage */
    if(argc != 2 && argc != 3)
        error(false, "usage: tprof pid [binary]");
    /*e: [[main()]](tprof.c) sanity check argc and print usage */

    /*
     * Read symbol table
     */
    /*s: [[main()]](tprof.c) read symbol table */
    if(argc == 2){
        file = filebuf;
        snprint(filebuf, sizeof filebuf, "/proc/%s/text", argv[1]);
    }else
        file = argv[2];

    fd = open(file, OREAD);
    /*s: [[main()]](tprof.c) sanity check [[fd]] */
    if(fd < 0)
        error(true, file);
    /*e: [[main()]](tprof.c) sanity check [[fd]] */

    if (!crackhdr(fd, &f))
        error(true, "read text header");
    /*s: [[main()]](tprof.c) sanity check [[f.type]] */
    if (f.type == FNONE)
        error(false, "text file not an a.out");
    /*e: [[main()]](tprof.c) sanity check [[f.type]] */

    machbytype(f.type);
    if (syminit(fd, &f) < 0)
        error(true, "syminit");
    close(fd);
    /*e: [[main()]](tprof.c) read symbol table */

    /*
     * Read timing data
     */
    /*s: [[main()]](tprof.c) read timing data */
    file = smprint("/proc/%s/profile", argv[1]);
    fd = open(file, OREAD);
    /*s: [[main()]](tprof.c) sanity check [[fd]] */
    if(fd < 0)
        error(true, file);
    /*e: [[main()]](tprof.c) sanity check [[fd]] */
    free(file);

    d = dirfstat(fd);
    /*s: [[main()]](tprof.c) sanity check [[d]] */
    if(d == nil)
        error(true, "stat");
    /*e: [[main()]](tprof.c) sanity check [[d]] */

    n = d->length/sizeof(data[0]);
    /*s: [[main()]](tprof.c) sanity check [[n]] */
    if(n < 2)
        error(false, "data file too short");
    /*e: [[main()]](tprof.c) sanity check [[n]] */
    data = malloc(d->length);
    /*s: [[main()]](tprof.c) sanity check [[data]] */
    if(data == nil)
        error(true, "malloc");
    /*e: [[main()]](tprof.c) sanity check [[data]] */
    if(read(fd, data, d->length) < 0)
        error(true, "text read");
    close(fd);

    for(i=0; i<n; i++)
        data[i] = machdata->swal(data[i]);
    /*e: [[main()]](tprof.c) read timing data */

    /*s: [[main()]](tprof.c) displaying profile */
    delta = data[0]-data[1];
    print("total: %ld\n", data[0]);
    if(data[0] == 0)
        exits(nil);
    // else
    if (!textsym(&s, 0))
        error(0, "no text symbols");

    tbase = s.value & ~(mach->pgsize-1);	/* align down to page */

    print("TEXT %.8lux\n", tbase);

    /*
     * Accumulate counts for each function
     */
    cp = 0;
    k = 0;
    for (i = 0, j = (s.value-tbase)/PCRES+2; j < n; i++) {
        name = s.name;		/* save name */
        if (!textsym(&s, i))	/* get next symbol */
            break;
        sum = 0;
        while (j < n && j*PCRES < s.value-tbase)
            sum += data[j++];
        if (sum) {
            cp = realloc(cp, (k+1)*sizeof(struct COUNTER));
            if (cp == 0)
                error(1, "realloc");
            cp[k].name = name;
            cp[k].time = sum;
            k++;
        }
    }
    if (!k)
        error(0, "no counts");
    cp[k].time = 0;			/* "etext" can take no time */

    /*
     * Sort by time and print
     */
    qsort(cp, k, sizeof(struct COUNTER), compar);
    Binit(&outbuf, 1, OWRITE);
    Bprint(&outbuf, "    ms      %%   sym\n");
    while(--k>=0)
        Bprint(&outbuf, "%6ld\t%3lld.%lld\t%s\n",
                cp[k].time,
                100LL*cp[k].time/delta,
                (1000LL*cp[k].time/delta)%10,
                cp[k].name);
    /*e: [[main()]](tprof.c) displaying profile */

    exits(nil);
}
/*e: function main (misc/tprof.c) */
/*e: misc/tprof.c */
