/*s: time/date.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

/*s: global flags(date.c) */
// UTC time, which is mostly the same than GMT
bool uflg;
// number of seconds since the (UNIX) epoch, 1/1/1970:00:00
bool nflg;
/*e: global flags(date.c) */

/*s: function [[main]](date.c) */
void
main(int argc, char *argv[])
{
    ulong now;

    ARGBEGIN{
    case 'n':   nflg = true; break;
    case 'u':   uflg = true; break;
    default:    fprint(STDERR, "usage: date [-un] [seconds]\n"); exits("usage");
    }ARGEND

    if(argc == 1)
        now = strtoul(*argv, 0, 0);
    else
        now = time(0);

    if(nflg)
        print("%ld\n", now);
    else if(uflg)
        print("%s", asctime(gmtime(now)));
    else
        print("%s", ctime(now));
    
    exits(nil);
}
/*e: function [[main]](date.c) */
/*e: time/date.c */
