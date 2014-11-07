/*s: mk/job.c */
#include	"mk.h"

/*s: constructor newjob */
Job*
newjob(Rule *r, Node *nlist, char *stem, char **match, Word *pre, Word *npre, Word *tar, Word *atar)
{
    Job *j;

    j = (Job *)Malloc(sizeof(Job));
    j->r = r;
    j->n = nlist;
    j->stem = stem;
    j->match = match;
    j->p = pre;
    j->np = npre;
    j->t = tar;
    j->at = atar;
    j->nproc = -1;
    j->next = nil;
    return j;
}
/*e: constructor newjob */

/*s: dumper dumpj */
void
dumpj(char *s, Job *j, int all)
{
    Bprint(&bout, "%s\n", s);
    while(j){
        Bprint(&bout, "job@%p: r=%p n=%p stem='%s' nproc=%d\n",
            j, j->r, j->n, j->stem, j->nproc);
        Bprint(&bout, "\ttarget='%s' alltarget='%s' prereq='%s' nprereq='%s'\n",
            wtos(j->t, ' '), wtos(j->at, ' '), wtos(j->p, ' '), wtos(j->np, ' '));
        j = all? j->next : nil;
    }
}
/*e: dumper dumpj */
/*e: mk/job.c */
