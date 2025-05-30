# VARIABLE TO ADJUST
# path to kencc toplevel directory
KENCC=$HOME/kencc

# DO NOT MODIFY THOSE VARIABLES
# to find mk, 8c/5c, 8a/5a, 8l/5l, rc, sed, ...
export PATH=$KENCC/bin:$PATH
# for mk to find and use rc
#export MKSHELL=$KENCC/bin/rc
export MKSHELL=`which rc`
# for rc to find its rcmain initialization file
#export RCMAIN=$KENCC/lib/rcmain.unix
