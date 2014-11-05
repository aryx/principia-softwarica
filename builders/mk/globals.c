/*s: mk/globals.c */
#include	"mk.h"

/*s: global debug */
// bitset<enum<dxxx>>
int debug;
/*e: global debug */
/*s: global rules */
// list<ref_own<Rule>> (next = Rule.next)
Rule *rules;
/*e: global rules */
/*s: global metarules */
// list<ref_own<Rule>>
Rule *metarules;
/*e: global metarules */
/*s: global nflag */
bool nflag = false;
/*e: global nflag */
/*s: global tflag */
bool tflag = false;
/*e: global tflag */
/*s: global iflag */
bool iflag = false;
/*e: global iflag */
/*s: global kflag */
bool kflag = false;
/*e: global kflag */
/*s: global aflag */
bool aflag = false;
/*e: global aflag */
/*s: global explain */
char *explain = nil;
/*e: global explain */
/*s: global nreps */
int nreps = 1;
/*e: global nreps */
/*s: global jobs */
// list<ref_won<jobs>> (next = Job.next)
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
