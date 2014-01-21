export KENCC=/home/pad/kencc
export PLAN9=/usr/local/plan9

export PATH=$PLAN9/bin:$KENCC/bin
#todo: does not work :( have to put this line in all mkfile
export MKSHELL=$PLAN9/bin/rc

#for 8._cp to be found and called
PATH=$PATH:.

export objtype=386
export cputype=386
