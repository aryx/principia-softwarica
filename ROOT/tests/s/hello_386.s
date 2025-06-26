// WEIRD: 8c -S hello.c generates assembly code to start at 0(SP) instead of 4(SP)
// (which is what I do for arm and mips) but does not seem to work so
// I switched to 4(SP) (similar to arm and mips)
TEXT	_main(SB), $20
	// pwrite(1, "hello world\n", 12, 00);
	MOVL	$1,AX
	MOVL	AX,4(SP)
	MOVL	$msg(SB),AX
	MOVL	AX,8(SP)
	MOVL	$12,AX
	MOVL	AX,12(SP)
	MOVL	$0,16(SP)
	MOVL	$0,20(SP)

	MOVL $9, AX
	INT $64	

	MOVL	$msg(SB),AX
	MOVL	AX,4(SP)
	MOVL $3, AX
	INT $64	
	RET	

// -------------------------------------------
// msg: must split into 8-byte chunks
// -------------------------------------------
DATA	msg+0(SB)/8,$"hello wo"
DATA	msg+8(SB)/8,$"rld\n\z\z\z\z"
GLOBL	msg(SB),$16
