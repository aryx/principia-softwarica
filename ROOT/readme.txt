We need this directory and some of its files and subdirs for the build because:
 - the mkfiles are using $ROOT/$objtype/lib/ to store the libs and
   $ROOT/$objtype/bin for the binaries.
 - iyacc (from kencc) is using the ROOT environment variable to find
   $ROOT/lib/yaccpar
 - some mkfiles for the kernel look for $ROOT/lib/scsicodes
