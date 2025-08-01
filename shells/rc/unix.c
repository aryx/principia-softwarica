/*s: rc/unix.c */
/*
 * Unix versions of system-specific functions
 * By convention, exported routines herein have names beginning with an
 * upper case letter.
 */
// to avoid conflict for wait(), waitpid() signatures
#define NOPLAN9DEFINES
#include "rc.h"
#include "io.h"
#include "exec.h"
#include "getflags.h"

//TODO? use plan9port errstr instead?
#include <errno.h>

// system-specific globals defined here but used in other files
char *Rcmain = "/usr/lib/rcmain";
char *Fdprefix = "/dev/fd/";

//******************************************************************************
// Environment
//******************************************************************************

#define	SEP	'\1'

word*
enval(char *s)
{
  char *t, c;
  word *v;
  for(t = s; *t && *t!=SEP; t++);
  c=*t;
  *t='\0';
  v = newword(s, c=='\0'?(word *)0:enval(t+1));
  *t = c;
  return v;
}

// set in Vinit()
char **environp;

void
Vinit(void)
{
  // see Unix environ(7) man page
  extern char **environ;
  char *s;
  char **env = environ;
  word* wd;
  environp = env;
  for(; *env; env++){
    for(s=*env; *s && *s!='(' && *s!='='; s++);
    switch(*s){
    case '\0':
      pfmt(err, "environment %q?\n", *env);
      break;
    case '=':
      *s='\0';
      wd = enval(s+1);
      setvar(*env, wd);
      //TODO: should generalize this in main.c so it also applies for Plan9
      if(strcmp(*env, "RCMAIN") == 0) {
 Rcmain = strdup(wd->word);
      }
      *s='=';
      break;
    case '(':	/* ignore functions for now */
      break;
    }
  }
}

//TODO: should be set in execfinit
char **envp = nil;

void
Xrdfn(void)
{
  char *s;
  int len;
  for(;*envp;envp++){
    for(s=*envp;*s && *s!='(' && *s!='=';s++);
    switch(*s){
    case '\0':
      pfmt(err, "environment %q?\n", *envp);
      break;
    case '=':	/* ignore variables */
      break;
    case '(':		/* Bourne again */
      s=*envp+3;
      envp++;
      len = strlen(s);
      s[len]='\n';
      execcmds(opencore(s, len+1));
      s[len]='\0';
      return;
    }
  }
  Xreturn();
}
//
//union code rdfns[4];
//
//void
//execfinit(void)
//{
//	static int first = 1;
//	if(first){
//		rdfns[0].i = 1;
//		rdfns[1].f = Xrdfn;
//		rdfns[2].f = Xjump;
//		rdfns[3].i = 1;
//		first = 0;
//	}
//	Xpopm();
//	envp = environp;
//	start(rdfns, 1, runq->local);
//}
//
//int
//cmpenv(const void *aa, const void *ab)
//{
//	char **a = aa, **b = ab;
//
//	return strcmp(*a, *b);
//}
//
//char **
//mkenv(void)
//{
//	char **env, **ep, *p, *q;
//	struct var **h, *v;
//	struct word *a;
//	int nvar = 0, nchr = 0, sep;
//
//	/*
//	 * Slightly kludgy loops look at locals then globals.
//	 * locals no longer exist - geoff
//	 */
//	for(h = gvar-1; h != &gvar[NVAR]; h++)
//	for(v = h >= gvar? *h: runq->local; v ;v = v->next){
//		if((v==vlook(v->name)) && v->val){
//			nvar++;
//			nchr+=strlen(v->name)+1;
//			for(a = v->val;a;a = a->next)
//				nchr+=strlen(a->word)+1;
//		}
//		if(v->fn){
//			nvar++;
//			nchr+=strlen(v->name)+strlen(v->fn[v->pc-1].s)+8;
//		}
//	}
//	env = (char **)emalloc((nvar+1)*sizeof(char *)+nchr);
//	ep = env;
//	p = (char *)&env[nvar+1];
//	for(h = gvar-1; h != &gvar[NVAR]; h++)
//	for(v = h >= gvar? *h: runq->local;v;v = v->next){
//		if((v==vlook(v->name)) && v->val){
//			*ep++=p;
//			q = v->name;
//			while(*q) *p++=*q++;
//			sep='=';
//			for(a = v->val;a;a = a->next){
//				*p++=sep;
//				sep = SEP;
//				q = a->word;
//				while(*q) *p++=*q++;
//			}
//			*p++='\0';
//		}
//		if(v->fn){
//			*ep++=p;
//			*p++='#'; *p++='('; *p++=')';	/* to fool Bourne */
//			*p++='f'; *p++='n'; *p++=' ';
//			q = v->name;
//			while(*q) *p++=*q++;
//			*p++=' ';
//			q = v->fn[v->pc-1].s;
//			while(*q) *p++=*q++;
//			*p++='\0';
//		}
//	}
//	*ep = 0;
//	qsort((void *)env, nvar, sizeof ep[0], cmpenv);
//	return env;	
//}

void
Updenv(void)
{
}

//******************************************************************************
// Signals/notes and Waitfor()
//******************************************************************************

char *sigmsg[] = {
/*  0 normal  */ 0,
/*  1 SIGHUP  */ "Hangup",
/*  2 SIGINT  */ 0,
/*  3 SIGQUIT */ "Quit",
/*  4 SIGILL  */ "Illegal instruction",
/*  5 SIGTRAP */ "Trace/BPT trap",
/*  6 SIGIOT  */ "abort",
/*  7 SIGEMT  */ "EMT trap",
/*  8 SIGFPE  */ "Floating exception",
/*  9 SIGKILL */ "Killed",
/* 10 SIGBUS  */ "Bus error",
/* 11 SIGSEGV */ "Memory fault",
/* 12 SIGSYS  */ "Bad system call",
/* 13 SIGPIPE */ 0,
/* 14 SIGALRM */ "Alarm call",
/* 15 SIGTERM */ "Terminated",
/* 16 unused  */ "signal 16",
/* 17 SIGSTOP */ "Process stopped",
/* 18 unused  */ "signal 18",
/* 19 SIGCONT */ "Process continued",
/* 20 SIGCHLD */ "Child death",
};

//pad: Note that plan9 Waitfor(), that was in processes.c (now in plan9.c), was
// compiling correctly also under Unix and I originally used it and rc
// was partially working. However, when called from mk (via MKSHELL), I had
// weird bugs like the $status was containing weird strings and so
// failing mk even if the command was run correctly.
// Anyway, simpler to uncomment and use the Waitfor() below.
void
Waitfor(int pid, bool persist)
{
 int wpid, sig;
 thread *p;
 int wstat;
 char wstatstr[12];

 for(;;){
  errno = 0;
  wpid = wait(&wstat);
  if(errno==EINTR && persist)
   continue;
  if(wpid==-1)
   break;
  sig = wstat&0177;
  if(sig==0177){
   pfmt(err, "trace: ");
   sig = (wstat>>8)&0177;
  }
  if(sig>(sizeof sigmsg/sizeof sigmsg[0]) || sigmsg[sig]){
   if(pid!=wpid)
    pfmt(err, "%d: ", wpid);
   if(sig<=(sizeof sigmsg/sizeof sigmsg[0]))
    pfmt(err, "%s", sigmsg[sig]);
   else if(sig==0177) pfmt(err, "stopped by ptrace");
   else pfmt(err, "signal %d", sig);
   if(wstat&0200)pfmt(err, " -- core dumped");
   pfmt(err, "\n");
  }
  wstat = sig?sig+1000:(wstat>>8)&0xFF;
  if(wpid==pid){
   inttoascii(wstatstr, wstat);
   setstatus(wstatstr);
   break;
  }
  else{
   for(p = runq->ret;p;p = p->ret)
    if(p->pid==wpid){
     p->pid=-1;
     inttoascii(p->status, wstat);
     break;
    }
  }
 }
}

char *signame[] = {
  "sigexit",	"sighup",	"sigint",	"sigquit",
  "sigill",	"sigtrap",	"sigiot",	"sigemt",
  "sigfpe",	"sigkill",	"sigbus",	"sigsegv",
  "sigsys",	"sigpipe",	"sigalrm",	"sigterm",
  "sig16",	"sigstop",	"sigtstp",	"sigcont",
  "sigchld",	"sigttin",	"sigttou",	"sigtint",
  "sigxcpu",	"sigxfsz",	"sig26",	"sig27",
  "sig28",	"sig29",	"sig30",	"sig31",
  0,
};

// defined in trap.c
extern int trap[NSIG];

void
gettrap(int sig)
{
  signal(sig, gettrap);
  trap[sig]++;
  ntrap++;
  if(ntrap>=NSIG){
    pfmt(err, "rc: Too many traps (trap %d), dumping core\n", sig);
    signal(SIGABRT, (void (*)())0);
    kill(getpid(), SIGABRT);
  }
}

void
Trapinit(void)
{
  int i;
  void (*sig)();

  if(1 || flag['d']){	/* wrong!!! */
    sig = signal(SIGINT, gettrap);
    if(sig==SIG_IGN)
      signal(SIGINT, SIG_IGN);
  }
  else{
    for(i = 1;i<=NSIG;i++) if(i!=SIGCHLD){
 sig = signal(i, gettrap);
 if(sig==SIG_IGN)
   signal(i, SIG_IGN);
      }
  }
}

//******************************************************************************
// Errno
//******************************************************************************

bool
Eintr(void){
  return errno==EINTR;
}

void
Noerror(void)
{
  errno = 0;
}

//******************************************************************************
// Directories
//******************************************************************************

#define	NDIR	14		/* should get this from param.h */
int
Globsize(char *p)
{
  int isglob = 0, globlen = NDIR+1;
  for(;*p;p++){
    if(*p==GLOB){
      p++;
      if(*p!=GLOB)
 isglob++;
      globlen+=*p=='*'?NDIR:1;
    }
    else
      globlen++;
  }
  return isglob?globlen:0;
}

#include <sys/types.h>
#include <dirent.h>

#define	NDIRLIST	50

DIR *dirlist[NDIRLIST];

Opendir(name)
     char *name;
{
  DIR **dp;
  for(dp = dirlist;dp!=&dirlist[NDIRLIST];dp++)
    if(*dp==0){
      *dp = opendir(name);
      return *dp?dp-dirlist:-1;
    }
  return -1;
}

int
Readdir(int f, char *p, int onlydirs)
{
  struct dirent *dp = readdir(dirlist[f]);

  if(dp==0)
    return 0;
  strcpy(p, dp->d_name);
  return 1;
}

void
Closedir(int f)
{
  closedir(dirlist[f]);
  dirlist[f] = 0;
}

//******************************************************************************
// Misc
//******************************************************************************

//char **
//mkargv(a)
//register struct word *a;
//{
//	char **argv = (char **)emalloc((count(a)+2)*sizeof(char *));
//	char **argp = argv+1;	/* leave one at front for runcoms */
//
//	for(;a;a = a->next)
//		*argp++=a->word;
//	*argp = 0;
//	return argv;
//}
//

//void
//Execute(struct word *args, struct word *path)
//{
//	char *msg="not found";
//	int txtbusy = 0;
//	char **env = mkenv();
//	char **argv = mkargv(args);
//	char file[512];
//
//	for(;path;path = path->next){
//		strcpy(file, path->word);
//		if(file[0])
//			strcat(file, "/");
//		strcat(file, argv[1]);
//ReExec:
//		execve(file, argv+1, env);
//		switch(errno){
//		case ENOEXEC:
//			pfmt(err, "%s: Bourne again\n", argv[1]);
//			argv[0]="sh";
//			argv[1] = strdup(file);
//			execve("/bin/sh", argv, env);
//			goto Bad;
//		case ETXTBSY:
//			if(++txtbusy!=5){
//				sleep(txtbusy);
//				goto ReExec;
//			}
//			msg="text busy"; goto Bad;
//		case EACCES:
//			msg="no access";
//			break;
//		case ENOMEM:
//			msg="not enough memory"; goto Bad;
//		case E2BIG:
//			msg="too big"; goto Bad;
//		}
//	}
//Bad:
//	pfmt(err, "%s: %s\n", argv[1], msg);
//	efree((char *)env);
//	efree((char *)argv);
//}


/*
 * Wrong:  should go through components of a|b|c and return the maximum.
 */
void
Exit(char *stat, char* loc)
{
  int n = 0;
  USED(loc);
  //if(flag['s'])
  //  fprint(STDERR, "Exit from %s: %s\n", loc, stat);

  while(*stat){
    if(*stat!='|'){
      if(*stat<'0' || '9'<*stat)
 exit(1);
      else n = n*10+*stat-'0';
    }
    stat++;
  }
  exit(n);
}

bool Isatty(fdt fd){
  return isatty(fd);
}

//void
//execumask(void)		/* wrong -- should fork before writing */
//{
//	int m;
//	struct io out[1];
//	switch(count(runq->argv->words)){
//	default:
//		pfmt(err, "Usage: umask [umask]\n");
//		setstatus("umask usage");
//		poplist();
//		return;
//	case 2:
//		umask(octal(runq->argv->words->next->word));
//		break;
//	case 1: 
//		umask(m = umask(0));
//		out->fd = mapfd(1);
//		out->bufp = out->buf;
//		out->ebuf=&out->buf[NBUF];
//		out->strp = 0;
//		pfmt(out, "%o\n", m);
//		break;
//	}
//	setstatus("");
//	poplist();
//}


void*
Malloc(unsigned long n)
{
  return (void *)malloc(n);
}

int
needsrcquote(int c)
{
  if(c <= ' ')
    return 1;
  if(strchr("`^#*[]=|\\?${}()'<>&;", c))
    return 1;
  return 0;
}

//void
//delwaitpid(int pid)
//{
//	int r, w;
//	
//	for(r=w=0; r<nwaitpids; r++)
//		if(waitpids[r] != pid)
//			waitpids[w++] = waitpids[r];
//	nwaitpids = w;
//}

//int
//havewaitpid(int pid)
//{
//	int i;
//
//	for(i=0; i<nwaitpids; i++)
//		if(waitpids[i] == pid)
//			return 1;
//	return 0;
//
//}

/*e: rc/unix.c */
