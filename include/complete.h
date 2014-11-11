/*s: include/complete.h */
#pragma	lib	"libcomplete.a"
#pragma src "/sys/src/libcomplete"

typedef struct Completion Completion;

/*s: struct Completion */
struct Completion{
    uchar advance;	/* whether forward progress has been made */
    uchar complete;	/* whether the completion now represents a file or directory */
    char *string; /* the string to advance, suffixed " " or "/" for file or directory */
    int nmatch;		/* number of files that matched */
    int nfile;		/* number of files returned */
    char **filename;	/* their names */
};
/*e: struct Completion */

Completion* complete(char *dir, char *s);
void freecompletion(Completion*);
/*e: include/complete.h */
