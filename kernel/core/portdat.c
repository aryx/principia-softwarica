#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

// was in init/main.c
Conf conf;
// bool cpuserver; // defined in conf/pcf.c

// conf (boot) parameters *e.g. { "*kernelpercent*" => "60" }
// hash<string, string>
char *confname[MAXCONF];
char *confval[MAXCONF];
// Hashtbl.length(confname)
int nconf;

char* getconf(char *name)
{
        int i;
        for(i = 0; i < nconf; i++)
                if(cistrcmp(confname[i], name) == 0)
                        return confval[i];
        return nil;
}

// was in init/main.c
Mach *m;
// was in dat.h
Mach* machp[MAXMACH];

// was in security/auth.c
char	*eve;
// should be in portfns.c, but then backward deps
iseve(void) { 
  return strcmp(eve, up->user) == 0; 
}

// was in console/devcons.c, but used also by edf.c
int	panicking;

