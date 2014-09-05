#include	"mk.h"

int debug;
Rule *rules, *metarules;
int nflag = 0;
int tflag = 0;
int iflag = 0;
int kflag = 0;
int aflag = 0;
char *explain = 0;
int nreps = 1;
Job *jobs;
Biobuf bout;
Rule *patrule;
