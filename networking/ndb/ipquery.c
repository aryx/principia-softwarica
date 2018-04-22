/*s: networking/ndb/ipquery.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ndb.h>
#include <ip.h>

/*s: function [[usage]]([[(networking/ndb/ipquery.c)]]) */
/*
 *  search the database for matches
 */

void
usage(void)
{
    fprint(2, "usage: ipquery attr value rattribute\n");
    exits("usage");
}
/*e: function [[usage]]([[(networking/ndb/ipquery.c)]]) */

/*s: function [[search]] */
void
search(Ndb *db, char *attr, char *val, char **rattr, int nrattr)
{
    Ndbtuple *t, *tt;

    tt = ndbipinfo(db, attr, val, rattr, nrattr);
    for(t = tt; t; t = t->entry)
        print("%s=%s ", t->attr, t->val);
    print("\n");
    ndbfree(tt);
}
/*e: function [[search]] */

/*s: function [[main]]([[(networking/ndb/ipquery.c)]]) */
void
main(int argc, char **argv)
{
    Ndb *db;
    char *dbfile = 0;

    ARGBEGIN{
    case 'f':
        dbfile = ARGF();
        break;
    }ARGEND;

    if(argc < 3)
        usage();

    db = ndbopen(dbfile);
    if(db == 0){
        fprint(2, "no db files\n");
        exits("no db");
    }
    search(db, argv[0], argv[1], argv+2, argc-2);
    ndbclose(db);

    exits(0);
}
/*e: function [[main]]([[(networking/ndb/ipquery.c)]]) */
/*e: networking/ndb/ipquery.c */
