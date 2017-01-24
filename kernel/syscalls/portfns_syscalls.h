// syscallfmt.c (but called only from arch-specific)
void syscallfmt(int syscallno, ulong pc, va_list list);
void sysretfmt(int syscallno, va_list list, long ret, uvlong start, uvlong stop);
