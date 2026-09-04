arg=0
link=14
sp=13

/* claude: split out of setjmp.s -- when this file combined setjmp,
 * sigsetjmp and longjmp as three TEXT symbols together, a program
 * that only pulls this archive member in indirectly (e.g. pic calling
 * ANSI signal(), which needs notetramp.c's _notetramp, whose own
 * siglongjmp() calls longjmp()) failed 5l with "longjmp: undefined",
 * even though the symbol is right there in the archive, and linking
 * the .5 in directly (bypassing the archive) works fine. 386's own
 * setjmp.s has the same three-symbols-in-one-file shape and doesn't
 * hit this, so it's arm-specific -- likely somewhere in libmach's
 * 5obj.c or 5l's archive-member symbol resolution, not confirmed at
 * the byte level. One symbol per file, like every other file in this
 * directory, sidesteps it either way. Minimal repro, investigation
 * notes, and what was ruled out: goken9cc's tests/ar/.
 */
TEXT	longjmp(SB), 1, $-4
	MOVW	r+4(FP), R(arg+2)
	CMP	$0, R(arg+2)
	BNE	ok			/* ansi: "longjmp(0) => longjmp(1)" */
	MOVW	$1, R(arg+2)		/* bless their pointed heads */
ok:	MOVW	(R(arg+0)), R(sp)
	MOVW	4(R(arg+0)), R(link)
	MOVW	R(arg+2), R(arg+0)
	RET
