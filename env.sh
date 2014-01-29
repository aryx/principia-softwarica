export KENCC=/home/pad/kencc
# need to modify plan9/src/cmd/mk/shell.c and put rcshell as default shell
export PLAN9=/usr/local/plan9

export PATH=$PLAN9/bin:$KENCC/bin:$PATH

#for 8._cp to be found and called
PATH=$PATH:.

export objtype=386
#export objtype=arm
export cputype=386
