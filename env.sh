# try to use absolute paths below if you use a wrapper script like my 'mk'

# for 8c, 8a, 8l, iar, etc 
export KENCC=/home/pad/kencc
# for rc, mk (configured with rc), etc
export PLAN9=/home/pad/Downloads/plan9port
# for 'ar' that will call kencc 'iar'
export CROSS=/home/pad/plan9/CROSS

# Note that kencc has an 'mk' but it's configured with 'sh', 
# so it's overriden by the 'mk' in plan9port via the PATH
# ordering below. The reason to configure 'mk' with rc is
# because the mkfiles in plan9 assumes rc in too many places.
# So after downloading plan9port don't forget to modify
# plan9/src/cmd/mk/shell.c and put rcshell as the default shell.

export PATH=$CROSS/bin:$PLAN9/bin:$KENCC/bin:$PATH

#for 8._cp to be found and called
PATH=$PATH:.

export objtype=386
export cputype=386
#alt: export objtype=arm
