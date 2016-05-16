/*s: include/plumb.h */
#pragma	lib	"libplumb.a"
#pragma	src	"/sys/src/libplumb"

/*
 * Message format:
 *	source application\n
 *	destination port\n
 *	working directory\n
 *	type\n
 *	attributes\n
 *	nbytes\n
 *	n bytes of data
 */

typedef struct Plumbattr Plumbattr;
typedef struct Plumbmsg Plumbmsg;

/*s: struct Plumbmsg */
struct Plumbmsg
{
    char		*src;
    char		*dst;
    char		*wdir;
    char		*type;
    Plumbattr		*attr;
    int			ndata;
    char		*data;
};
/*e: struct Plumbmsg */

/*s: struct Plumbattr */
struct Plumbattr
{
    char		*name;
    char		*value;
    Plumbattr		*next;
};
/*e: struct Plumbattr */

int		    plumbsend(int, Plumbmsg*);
int		    plumbsendtext(int, char*, char*, char*, char*);
Plumbmsg*	plumbrecv(int);
char*		plumbpack(Plumbmsg*, int*);
Plumbmsg*	plumbunpack(char*, int);
Plumbmsg*	plumbunpackpartial(char*, int, int*);
char*		plumbpackattr(Plumbattr*);
Plumbattr*	plumbunpackattr(char*);
Plumbattr*	plumbaddattr(Plumbattr*, Plumbattr*);
Plumbattr*	plumbdelattr(Plumbattr*, char*);
void		plumbfree(Plumbmsg*);
char*		plumblookup(Plumbattr*, char*);
int		    plumbopen(char*, int);

int		eplumb(int, char*);
/*e: include/plumb.h */
