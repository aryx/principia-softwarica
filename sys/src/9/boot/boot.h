/* libc equivalent */
extern void	fatal(char*);
extern void	warning(char*);
extern void	run(char *file, ...);
extern void	runv(char **argv);

/* pad style */
void bind_safe(char* old, char* new, int flag);
int open_safe(char* path, int flag);
void print_safe(int fd, char* str);
void close_safe(int fd);

/* main entry point */
extern void	boot(int, char **);



/* what bootpcf.c expect */
typedef struct Method	Method;
struct Method
{
	char	*name;
	void	(*config)(Method*);
	int	(*connect)(void);
	char	*arg;
};

extern Method	method[];		/* defined in ../$arch/boot$CONF.c */
extern char*	bootdisk;		/* defined in ../$arch/boot$CONF.c */

/* methods */
extern void	configlocal(Method*);
extern int	connectlocal(void);

