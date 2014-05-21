/*s: portdat.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
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
// ref<Cpu>, assigned to MACHADDR in _clearbss
Cpu *cpu;
/*e: global m */
// was in dat.h
/*s: global machp */
/*
 * Each processor sees its own Cpu structure at address MACHADDR.
 * However, the Cpu structures must also be available via the per-processor
 * MMU information array machp, mainly for disambiguation and access to
 * the clock which is only maintained by the bootstrap processor (0).
 */
// array<ref<Cpu>>, MAXMACH is defined in 386/mem.h
Cpu* machp[MAXMACH];
/*e: global machp */

// was in security/auth.c
/*s: global eve */
char    *eve;
/*e: global eve */
// should be in portfns.c, but then backward deps
/*
 *  return true if current user is eve
 */
/*s: function iseve */
bool iseve(void) { 
  return strcmp(eve, up->user) == 0; 
}
/*e: function iseve */

/*e: portdat.c */
