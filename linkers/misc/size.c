/*s: linkers/misc/size.c */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>

#include	<mach.h>

/*s: function [[size]] */
int
size(char *file)
{
    fdt fd;
    Fhdr f;

    if((fd = open(file, OREAD)) < 0){
        fprint(2, "size: ");
        perror(file);
        return 1;
    }
    if(crackhdr(fd, &f)) {
        print("%ldt + %ldd + %ldb = %ld\t%s\n", f.txtsz, f.datsz,
            f.bsssz, f.txtsz+f.datsz+f.bsssz, file);
        close(fd);
        return 0;
    }
    fprint(2, "size: %s not an a.out\n", file);
    close(fd);
    return 1;
}
/*e: function [[size]] */

/*s: function main (linkers/misc/size.c) */
void
main(int argc, char *argv[])
{
    char *err;
    int i;

    ARGBEGIN {
    default:
        fprint(2, "usage: size [a.out ...]\n");
        exits("usage");
    } ARGEND;

    err = nil;
    if(argc == 0)
        if(size("8.out"))
            err = "error";
    for(i=0; i<argc; i++)
        if(size(argv[i]))
            err = "error";
    exits(err);
}
/*e: function main (linkers/misc/size.c) */
/*e: linkers/misc/size.c */
