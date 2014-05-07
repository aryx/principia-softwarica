/*s: portdat.c */
/*s: kernel basic includes */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
/*e: kernel basic includes */

// was in init/main.c
/*s: global conf */
Conf conf;
/*e: global conf */
// bool cpuserver; // defined in $CONF.c

/*s: globals confname and confvar */
// conf (boot) parameters *e.g. { "*kernelpercent*" => "60" }
// hash<string, string>
char *confname[MAXCONF];
char *confval[MAXCONF];
// Hashtbl.length(confname)
int nconf;
/*e: globals confname and confvar */

/*s: function getconf */
char* getconf(char *name)
{
        int i;
        for(i = 0; i < nconf; i++)
                if(cistrcmp(confname[i], name) == 0)
                        return confval[i];
        return nil;
}
/*e: function getconf */

// was in init/main.c
/*s: global m */
Mach *m;
/*e: global m */
// was in dat.h
/*s: global machp */
/*
 * Each processor sees its own Mach structure at address MACHADDR.
 * However, the Mach structures must also be available via the per-processor
 * MMU information array machp, mainly for disambiguation and access to
 * the clock which is only maintained by the bootstrap processor (0).
 */
// array<ref<Mach>>, MAXMACH is defined in 386/mem.h
Mach* machp[MAXMACH];
/*e: global machp */

// was in security/auth.c
char    *eve;
// should be in portfns.c, but then backward deps
/*
 *  return true if current user is eve
 */
bool iseve(void) { 
  return strcmp(eve, up->user) == 0; 
}

// was in console/devcons.c, but used also by edf.c
int panicking;
/*e: portdat.c */
