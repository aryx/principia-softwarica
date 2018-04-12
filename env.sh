# try to use absolute paths below if you use a wrapper script like my 'mk'

export objtype=386
#alt: arm 386
#export cputype=386

# for mk (configured with rc)
export MKPATH=/home/pad
# for 8c, 8a, 8l, ar, yacc, data2s, etc 
export KENCC=/home/pad/kencc
# for rc, sed, xd, ...
export PLAN9=/home/pad/packages/stow/plan9port

export PATH=$MKPATH/bin:$PLAN9/bin:$KENCC/bin:$PATH
