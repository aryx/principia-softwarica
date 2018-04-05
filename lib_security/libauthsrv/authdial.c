#include <u.h>
#include <libc.h>
#include <authsrv.h>
#include <bio.h>
//PAD #include <ndb.h>

int
authdial(char *netroot, char *dom)
{
//PAD: 	char *p;
//PAD: 	int rv;
//PAD: 
//PAD: 	if(dom == nil)
//PAD: 		/* look for one relative to my machine */
//PAD: 		return dial(netmkaddr("$auth", netroot, "ticket"), 0, 0, 0);
//PAD: 
//PAD: 	/* look up an auth server in an authentication domain */
//PAD: 	p = csgetvalue(netroot, "authdom", dom, "auth", nil);
//PAD: 
//PAD: 	/* if that didn't work, just try the IP domain */
//PAD: 	if(p == nil)
//PAD: 		p = csgetvalue(netroot, "dom", dom, "auth", nil);
//PAD: 	/*
//PAD: 	 * if that didn't work, try p9auth.$dom.  this is very helpful if
//PAD: 	 * you can't edit /lib/ndb.
//PAD: 	 */
//PAD: 	if(p == nil)
//PAD: 		p = smprint("p9auth.%s", dom);
//PAD: 	if(p == nil){			/* should no longer ever happen */
//PAD: 		werrstr("no auth server found for %s", dom);
//PAD: 		return -1;
//PAD: 	}
//PAD: 	rv = dial(netmkaddr(p, netroot, "ticket"), 0, 0, 0);
//PAD: 	free(p);
//PAD: 	return rv;
  return -1;
}
