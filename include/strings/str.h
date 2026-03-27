/*s: strings/str.h */
#pragma	src	"/sys/src/libstring"
#pragma	lib	"libstring.a"

// Extensible Strings.

// This file was originally called string.h but this conflicts with
// the Posix <string.h> so simpler to rename str.h (which anyway fits
// well with the 3-letters utf.h, bio.h, fmt.h, etc.).

/* extensible Strings */
typedef struct String {
    Lock;
    char	*base;	/* base of String */
    char	*end;	/* end of allocated space+1 */
    char	*ptr;	/* ptr into String */
    short	ref;
    uchar	fixed;
} String;

/*s: macro [[s_clone]] */
#define s_clone(s) s_copy((s)->base)
/*e: macro [[s_clone]] */
/*s: macro [[s_to_c]] */
#define s_to_c(s) ((s)->base)
/*e: macro [[s_to_c]] */
/*s: macro [[s_len]] */
#define s_len(s) ((s)->ptr-(s)->base)
/*e: macro [[s_len]] */

extern String*	s_append(String*, char*);
extern String*	s_array(char*, int);
extern String*	s_copy(char*);
extern void	s_free(String*);
extern String*	s_incref(String*);	
extern String*	s_memappend(String*, char*, int);
extern String*	s_nappend(String*, char*, int);
extern String*	s_new(void);
extern String*	s_newalloc(int);
extern String*	s_parse(String*, String*);
extern String*	s_reset(String*);
extern String*	s_restart(String*);
extern void	s_terminate(String*);
extern void	s_tolower(String*);
extern void	s_putc(String*, int);
extern String*	s_unique(String*);
extern String*	s_grow(String*, int);

#ifdef BGETC
extern int	s_read(Biobuf*, String*, int);
extern char	*s_read_line(Biobuf*, String*);
extern char	*s_getline(Biobuf*, String*);
typedef struct Sinstack Sinstack;
#pragma incomplete Sinstack
extern char	*s_rdinstack(Sinstack*, String*);
extern Sinstack	*s_allocinstack(char*);
extern void	s_freeinstack(Sinstack*);
#endif /* BGETC */
/*e: strings/str.h */
