/*s: pipe/p.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>

/*s: constant [[DEF]](p.c) */
#define DEF 22  /* lines in chunk: 3*DEF == 66, #lines per nroff page */
/*e: constant [[DEF]](p.c) */
/*s: globals p.c */
Biobuf *cons;
Biobuf bout;

int pglen = DEF;
/*e: globals p.c */

void printfile(fdt);

/*s: function [[main]](p.c) */
void
main(int argc, char *argv[])
{
    int n;
    fdt f;

    if((cons = Bopen("/dev/cons", OREAD)) == 0) {
        fprint(STDERR, "p: can't open /dev/cons\n");
        exits("missing /dev/cons");
    }
    Binit(&bout, STDOUT, OWRITE);
    n = 0;
    while(argc > 1) {
        --argc; argv++;
        if(*argv[0] == '-'){
            pglen = atoi(&argv[0][1]);
            if(pglen <= 0)
                pglen = DEF;
        } else {
            n++;
            f = open(argv[0], OREAD);
            if(f < 0){
                fprint(STDERR, "p: can't open %s - %r\n", argv[0]);
                continue;
            }
            printfile(f);
            close(f);
        }
    }
    if(n == 0)
        printfile(STDIN);
    exits(nil);
}
/*e: function [[main]](p.c) */
/*s: function [[printfile]](p.c) */
void
printfile(fdt f)
{
    int i, j, n;
    char *s, *cmd;
    Biobuf *b;

    b = malloc(sizeof(Biobuf));
    Binit(b, f, OREAD);
    for(;;){
        for(i=1; i <= pglen; i++) {
            s = Brdline(b, '\n');
            if(s == 0){
                n = Blinelen(b);
                if(n > 0)   /* line too long for Brdline */
                    for(j=0; j<n; j++)
                        Bputc(&bout, Bgetc(b));
                else{       /* true EOF */
                    free(b);
                    return;
                }
            }else{
                Bwrite(&bout, s, Blinelen(b)-1);
                if(i < pglen)
                    Bwrite(&bout, "\n", 1);
            }
        }
        Bflush(&bout);
        getcmd:
        cmd = Brdline(cons, '\n');
        if(cmd == nil || *cmd == 'q')
            exits(nil);
        cmd[Blinelen(cons)-1] = 0;
        if(*cmd == '!'){
            if(fork() == 0){
                dup(Bfildes(cons), 0);
                execl("/bin/rc", "rc", "-c", cmd+1, nil);
            }
            waitpid();
            goto getcmd;
        }
    }
}
/*e: function [[printfile]](p.c) */
/*e: pipe/p.c */
