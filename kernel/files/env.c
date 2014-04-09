#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

// this used to be in devenv.c, but to avoid backward deps I've splitted
// this file in 2

void
closeegrp(Egrp *eg)
{
	int i;
	Evalue *e;

	if(decref(eg) == 0){
		for(i=0; i<eg->nent; i++){
			e = eg->ent[i];
			free(e->name);
			if(e->value)
				free(e->value);
			free(e);
		}
		free(eg->ent);
		free(eg);
	}
}
