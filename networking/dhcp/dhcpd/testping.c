/*s: networking/ip/dhcpd/testping.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include <bio.h>
#include <ndb.h>
#include "dat.h"

/*s: global [[blog]]([[(networking/ip/dhcpd/testping.c)]]) */
char	*blog = "ipboot";
/*e: global [[blog]]([[(networking/ip/dhcpd/testping.c)]]) */

/*s: function [[main]]([[(networking/ip/dhcpd/testping.c)]]) */
void
main(int argc, char **argv)
{
    fmtinstall('E', eipconv);
    fmtinstall('I', eipconv);

    if(argc < 2)
        exits(0);
    if(icmpecho(argv[1]))
        fprint(2, "%s live\n", argv[1]);
    else
        fprint(2, "%s doesn't answer\n", argv[1]);
}
/*e: function [[main]]([[(networking/ip/dhcpd/testping.c)]]) */
/*e: networking/ip/dhcpd/testping.c */
