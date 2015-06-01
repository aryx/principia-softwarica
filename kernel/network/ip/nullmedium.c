/*s: kernel/network/ip/nullmedium.c */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"

#include "ip.h"

/*s: function nullbind */
static void
nullbind(Ipifc*, int, char**)
{
    error("cannot bind null device");
}
/*e: function nullbind */

/*s: function nullunbind */
static void
nullunbind(Ipifc*)
{
}
/*e: function nullunbind */

/*s: function nullbwrite */
static void
nullbwrite(Ipifc*, Block*, int, uchar*)
{
    error("nullbwrite");
}
/*e: function nullbwrite */

/*s: global nullmedium */
Medium nullmedium =
{
.name=      "null",
.bind=      nullbind,
.unbind=    nullunbind,
.bwrite=    nullbwrite,
};
/*e: global nullmedium */

/*s: function nullmediumlink */
void
nullmediumlink(void)
{
    addipmedium(&nullmedium);
}
/*e: function nullmediumlink */
/*e: kernel/network/ip/nullmedium.c */
