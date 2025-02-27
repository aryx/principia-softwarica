/*s: boot.h */
/* libc equivalent */
// aux.c: 
extern void fatal(char*);
extern void warning(char*);
extern void run(char *file, ...);
extern void runv(char **argv);
/* pad style */
void bind_safe(char* old, char* new, int flag);
int open_safe(char* path, int flag);
void print_safe(int fd, char* str);
void close_safe(int fd);

/* methods */
// local.c:
extern void  connectlocal(void);
//extern void configlocal(Method*);
// other possible method connections: network.c, ...

// <arch>/boot$CONF.c
//extern Method method[];   /* defined in ../$arch/boot$CONF.c */
//extern char*  bootdisk;   /* defined in ../$arch/boot$CONF.c */
//void main(int argc, char **argv)

/* main entry point */
// boot.c:
extern void boot(int, char **);

/*e: boot.h */
