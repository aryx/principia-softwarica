/*s: lib_networking/libip/tests/testreadipifc.c */
#include <u.h>
#include <libc.h>
#include <ip.h>

/*s: function [[main]] */
void
main(void)
{
    Ipifc *ifc, *list;
    Iplifc *lifc;
    int i;

    fmtinstall('I', eipfmt);
    fmtinstall('M', eipfmt);

    list = readipifc("/net", nil, -1);
    for(ifc = list; ifc; ifc = ifc->next){
        print("ipifc %s %d\n", ifc->dev, ifc->mtu);
        for(lifc = ifc->lifc; lifc; lifc = lifc->next)
            print("\t%I %M %I\n", lifc->ip, lifc->mask, lifc->net);
    }
}
/*e: function [[main]] */
/*e: lib_networking/libip/tests/testreadipifc.c */
