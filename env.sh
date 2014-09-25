export KENCC=/home/pad/kencc
# remember to modify plan9/src/cmd/mk/shell.c and put rcshell as 
# the default shell (the mkfiles in plan9 assumes rc)
export PLAN9=/home/pad/Downloads/plan9port

export PATH=$PLAN9/bin:$KENCC/bin:$PATH

#for 8._cp to be found and called
PATH=$PATH:.

export objtype=386
export cputype=386
#alt: export objtype=arm
