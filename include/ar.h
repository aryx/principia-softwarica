/*s: include/ar.h */
/*s: constant ARMAG */
#define	ARMAG	"!<arch>\n"
/*e: constant ARMAG */
/*s: constant SARMAG */
#define	SARMAG	8
/*e: constant SARMAG */

/*s: constant ARFMAG */
#define	ARFMAG	"`\n"
/*e: constant ARFMAG */
/*s: constant SARNAME */
#define SARNAME	16
/*e: constant SARNAME */

/*s: struct ar_hdr */
struct	ar_hdr
{
    char	name[SARNAME];
    char	date[12];
    char	uid[6];
    char	gid[6];
    char	mode[8];
    char	size[10];
    char	fmag[2];
};
/*e: struct ar_hdr */
/*s: constant SAR_HDR */
#define	SAR_HDR	(SARNAME+44)
/*e: constant SAR_HDR */
/*e: include/ar.h */
