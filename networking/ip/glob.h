/*s: networking/ip/glob.h */
typedef struct Glob Glob;
typedef struct Globlist Globlist;

/*s: struct Glob */
struct Glob{
    String	*glob;
    Glob	*next;
};
/*e: struct Glob */

/*s: struct Globlist */
struct Globlist{
    Glob	*first;
    Glob	**l;
};
/*e: struct Globlist */

extern	Globlist*	glob(char*);
extern	void		globadd(Globlist*, char*, char*);
extern	void		globlistfree(Globlist *gl);
extern	char*		globiter(Globlist *gl);
/*e: networking/ip/glob.h */
