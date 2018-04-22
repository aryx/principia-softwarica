OBJS=hello.5 world.5

hello: $OBJS
 5l -o hello $OBJS

%.5: %.c
 5c -c $stem.c

clean:V:
	rm -f *.5 hello

