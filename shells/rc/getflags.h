/*s: rc/getflags.h */
/*s: constant [[NFLAG]] */
#define	NFLAG	128
/*e: constant [[NFLAG]] */

extern char **flag[NFLAG];
extern char *flagset[];

int getflags(int, char*[], char*, int);
void	usage(char*);
/*e: rc/getflags.h */
