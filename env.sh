GOKEN=$HOME/goken
# to find mk, rc, 8c/5c, 8a/5a, 8l/5l, sed, ...
export PATH=$GOKEN/bin:$PATH
#coupling: goken/env.sh
export MKSHELL=$GOKEN/bin/rc
export RCMAIN=$GOKEN/etc/rcmain.unix
export YACCPAR=$GOKEN/etc/yaccpar
#export NPROC=`nproc`


#old: alt: when using kencc
## can use mk from xix or kencc or goken but should not use
## rc from xix nor kencc!!!
## VARIABLE TO ADJUST
## path to kencc toplevel directory
#KENCC=$HOME/kencc
##KENCC=/home/pad/work/KENCC/9-cc-david-colombier/Linux/386
#
## DO NOT MODIFY THOSE VARIABLES
## to find mk, 8c/5c, 8a/5a, 8l/5l, sed, ... (but NOT rc right now!)
#export PATH=$KENCC/bin:$PATH
## for mk to find and use rc
##TODO: buggy! export MKSHELL=$KENCC/bin/rc
#export MKSHELL=`which rc`
## for rc to find its rcmain initialization file
## or hack 
##export RCMAIN=$KENCC/lib/rcmain.unix
