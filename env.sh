# try to use absolute paths below if you use a wrapper script 'mk'

# for 8c, 8a, 8l, iar, etc
export KENCC=/home/pad/kencc
# for rc, mk, etc
# (remember to modify plan9/src/cmd/mk/shell.c and put rcshell as 
# the default shell (the mkfiles in plan9 assumes rc))
export PLAN9=/home/pad/Downloads/plan9port
# for 'ar' that will call kencc 'iar'
export CROSS=/home/pad/plan9/CROSS

export PATH=$CROSS/bin:$PLAN9/bin:$KENCC/bin:$PATH

#for 8._cp to be found and called
PATH=$PATH:.

export objtype=386
export cputype=386
#alt: export objtype=arm
