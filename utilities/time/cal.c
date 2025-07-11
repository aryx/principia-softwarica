/*s: time/cal.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>

/*s: constants cal.c */
char    dayw[] =
{
    " S  M Tu  W Th  F  S"
};
char    *smon[] =
{
    "January", "February", "March", "April",
    "May", "June", "July", "August",
    "September", "October", "November", "December",
};
char    mon[] =
{
    0,
    31, 29, 31, 30,
    31, 30, 31, 31,
    30, 31, 30, 31,
};
/*e: constants cal.c */
/*s: globals cal.c */
char    string[432];
Biobuf  bout;
/*e: globals cal.c */

// forward decls
int number(char *str);
void pstr(char *str, int n);
void cal(int m, int y, char *p, int w);
int jan1(int yr);
int curmo(void);
int curyr(void);

/*s: function [[main]](cal.c) */
void
main(int argc, char *argv[])
{
    int y, m;
    int i, j;

    if(argc > 3) {
        fprint(STDERR, "usage: cal [month] [year]\n");
        exits("usage");
    }
    Binit(&bout, STDOUT, OWRITE);

/*
 * no arg, print current month
 */
    if(argc == 1) {
        m = curmo();
        y = curyr();
        goto xshort;
    }

/*
 * one arg
 *  if looks like a month, print month
 *  else print year
 */
    if(argc == 2) {
        y = number(argv[1]);
        if(y < 0)
            y = -y;
        if(y >= 1 && y <= 12) {
            m = y;
            y = curyr();
            goto xshort;
        }
        goto xlong;
    }

/*
 * two arg, month and year
 */
    m = number(argv[1]);
    if(m < 0)
        m = -m;
    y = number(argv[2]);
    goto xshort;

/*
 *  print out just month
 */
xshort:
    if(m < 1 || m > 12)
        goto badarg;
    if(y < 1 || y > 9999)
        goto badarg;
    Bprint(&bout, "   %s %ud\n", smon[m-1], y);
    Bprint(&bout, "%s\n", dayw);
    cal(m, y, string, 24);
    for(i=0; i<6*24; i+=24)
        pstr(string+i, 24);
    exits(nil);

/*
 *  print out complete year
 */
xlong:
    y = number(argv[1]);
    if(y<1 || y>9999)
        goto badarg;
    Bprint(&bout, "\n\n\n");
    Bprint(&bout, "                                %ud\n", y);
    Bprint(&bout, "\n");
    for(i=0; i<12; i+=3) {
        for(j=0; j<6*72; j++)
            string[j] = '\0';
        Bprint(&bout, "         %.3s", smon[i]);
        Bprint(&bout, "                    %.3s", smon[i+1]);
        Bprint(&bout, "                    %.3s\n", smon[i+2]);
        Bprint(&bout, "%s   %s   %s\n", dayw, dayw, dayw);
        cal(i+1, y, string, 72);
        cal(i+2, y, string+23, 72);
        cal(i+3, y, string+46, 72);
        for(j=0; j<6*72; j+=72)
            pstr(string+j, 72);
    }
    Bprint(&bout, "\n\n\n");
    exits(nil);

badarg:
    Bprint(&bout, "cal: bad argument\n");
}
/*e: function [[main]](cal.c) */

/*s: struct [[dict]](cal.c) */
struct
{
    char*   word;
    int val;
} dict[] =
{
    "jan",      1,
    "january",  1,
    "feb",      2,
    "february", 2,
    "mar",      3,
    "march",    3,
    "apr",      4,
    "april",    4,
    "may",      5,
    "jun",      6,
    "june",     6,
    "jul",      7,
    "july",     7,
    "aug",      8,
    "august",   8,
    "sep",      9,
    "sept",     9,
    "september",    9,
    "oct",      10,
    "october",  10,
    "nov",      11,
    "november", 11,
    "dec",      12,
    "december", 12,
    0
};
/*e: struct [[dict]](cal.c) */
/*s: function [[number]](cal.c) */
/*
 * convert to a number.
 * if its a dictionary word,
 * return negative  number
 */
int
number(char *str)
{
    int n, c;
    char *s;

    for(n=0; s=dict[n].word; n++)
        if(strcmp(s, str) == 0)
            return -dict[n].val;
    n = 0;
    s = str;
    while(c = *s++) {
        if(c<'0' || c>'9')
            return 0;
        n = n*10 + c-'0';
    }
    return n;
}
/*e: function [[number]](cal.c) */
/*s: function [[pstr]](cal.c) */
void
pstr(char *str, int n)
{
    int i;
    char *s;

    s = str;
    i = n;
    while(i--)
        if(*s++ == '\0')
            s[-1] = ' ';
    i = n+1;
    while(i--)
        if(*--s != ' ')
            break;
    s[1] = '\0';
    Bprint(&bout, "%s\n", str);
}
/*e: function [[pstr]](cal.c) */

/*s: function [[cal]] */
void
cal(int m, int y, char *p, int w)
{
    int d, i;
    char *s;

    s = p;
    d = jan1(y);
    mon[2] = 29;
    mon[9] = 30;

    switch((jan1(y+1)+7-d)%7) {

    /*
     *  non-leap year
     */
    case 1:
        mon[2] = 28;
        break;

    /*
     *  1752
     */
    default:
        mon[9] = 19;
        break;

    /*
     *  leap year
     */
    case 2:
        ;
    }
    for(i=1; i<m; i++)
        d += mon[i];
    d %= 7;
    s += 3*d;
    for(i=1; i<=mon[m]; i++) {
        if(i==3 && mon[m]==19) {
            i += 11;
            mon[m] += 11;
        }
        if(i > 9)
            *s = i/10+'0';
        s++;
        *s++ = i%10+'0';
        s++;
        if(++d == 7) {
            d = 0;
            s = p+w;
            p = s;
        }
    }
}
/*e: function [[cal]] */

/*s: function [[jan1]](cal.c) */
/*
 *  return day of the week
 *  of jan 1 of given year
 */
int
jan1(int yr)
{
    int y, d;

/*
 *  normal gregorian calendar
 *  one extra day per four years
 */

    y = yr;
    d = 4+y+(y+3)/4;

/*
 *  julian calendar
 *  regular gregorian
 *  less three days per 400
 */

    if(y > 1800) {
        d -= (y-1701)/100;
        d += (y-1601)/400;
    }

/*
 *  great calendar changeover instant
 */

    if(y > 1752)
        d += 3;

    return d%7;
}
/*e: function [[jan1]](cal.c) */

/*s: function [[curmo]](cal.c) */
/*
 * system dependent
 * get current month and year
 */
int
curmo(void)
{
    Tm *tm;

    tm = localtime(time(0));
    return tm->mon+1;
}
/*e: function [[curmo]](cal.c) */
/*s: function [[curyr]](cal.c) */
int
curyr(void)
{
    Tm *tm;

    tm = localtime(time(0));
    return tm->year+1900;
}
/*e: function [[curyr]](cal.c) */
/*e: time/cal.c */
