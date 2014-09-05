/*s: mk/globals.c */
#include	"mk.h"

/*s: global debug */
int debug;
/*e: global debug */
/*s: global rules */
// list<ref_own<Rule>>
Rule *rules;
/*e: global rules */
/*s: global metarules */
// list<ref_own<Rule>>
Rule *metarules;
/*e: global metarules */
/*s: global nflag */
int nflag = 0;
/*e: global nflag */
/*s: global tflag */
int tflag = 0;
/*e: global tflag */
/*s: global iflag */
int iflag = 0;
/*e: global iflag */
/*s: global kflag */
int kflag = 0;
/*e: global kflag */
/*s: global aflag */
int aflag = 0;
/*e: global aflag */
/*s: global explain */
char *explain = 0;
/*e: global explain */
/*s: global nreps */
int nreps = 1;
/*e: global nreps */
/*s: global jobs */
// list<ref_won<jobs>>
Job *jobs;
/*e: global jobs */
/*s: global bout */
Biobuf bout;
/*e: global bout */
/*s: global patrule */
Rule *patrule;
/*e: global patrule */
// was in main.c, but used also by parse.c
/*s: global target1 */
Word *target1;
/*e: global target1 */
/*e: mk/globals.c */
