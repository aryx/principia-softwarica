/*s: assemblers/8a/lex.c */
#include "a.h"
#include "y.tab.h"

void	cinit(void);
int		assemble(char*);
void	cclean(void);
void	outhist(void);

/*s: function main (assemblers/8a/lex.c) */
void
main(int argc, char *argv[])
{
    /*s: [[main()]] locals */
    char *p;
    /*x: [[main()]] locals */
    int nout, nproc, status;
    int i, c;
    /*e: [[main()]] locals */
    /*s: [[main()]] debug initialization */
    memset(debug, false, sizeof(debug));
    /*e: [[main()]] debug initialization */

    thechar = '8';
    thestring = "386";

    cinit();
    outfile = nil;
    include[ninclude++] = ".";

    ARGBEGIN {
    /*s: [[main()]] command line processing */
    case 'o':
        outfile = ARGF();
        break;
    /*x: [[main()]] command line processing */
    case 'I':
        p = ARGF();
        setinclude(p);
        break;
    /*x: [[main()]] command line processing */
    case 'D':
        p = ARGF();
        if(p)
            Dlist[nDlist++] = p;
        break;
    /*x: [[main()]] command line processing */
    default:
        c = ARGC();
        if(c >= 0 || c < sizeof(debug))
            debug[c] = true;
        break;
    /*e: [[main()]] command line processing */
    } ARGEND

    if(*argv == 0) {
        print("usage: %ca [-options] file.s\n", thechar);
        errorexit();
    }

    /*s: [[main()]] multiple files handling */
    if(argc > 1) {
        nproc = 1;
        if(p = getenv("NPROC"))
            nproc = atol(p);	/* */
        c = 0;
        nout = 0;
        for(;;) {
            while(nout < nproc && argc > 0) {
                i = fork();
                if(i < 0) {
                    i = mywait(&status);
                    if(i < 0)
                        errorexit();
                    if(status)
                        c++;
                    nout--;
                    continue;
                }
                if(i == 0) {
                    print("%s:\n", *argv);
                    if(assemble(*argv))
                        errorexit();
                    exits(nil);
                }
                nout++;
                argc--;
                argv++;
            }
            i = mywait(&status);
            if(i < 0) {
                if(c)
                    errorexit();
                exits(nil);
            }
            if(status)
                c++;
            nout--;
        }
    }
    /*e: [[main()]] multiple files handling */

    if(assemble(argv[0]))
        errorexit();
    exits(0);
}
/*e: function main (assemblers/8a/lex.c) */

/*s: function [[assemble]] */
errorn
assemble(char *infile)
{
    fdt of; // outfile
    /*s: [[assemble()]] locals */
    char ofile[100];
    /*x: [[assemble()]] locals */
    char *p;
    /*x: [[assemble()]] locals */
    char incfile[20];
    /*x: [[assemble()]] locals */
    int i;
    /*e: [[assemble()]] locals */

    /*s: [[assemble()]] set p to basename(infile) and adjust include */
    // p = basename(infile)
    strcpy(ofile, infile);
    p = utfrrune(ofile, '/');
    if(p) {
        *p++ = '\0';
        /*s: [[assemble()]] adjust first entry in include with dirname infile */
        include[0] = ofile;
        /*e: [[assemble()]] adjust first entry in include with dirname infile */
    } else
        p = ofile;
    /*e: [[assemble()]] set p to basename(infile) and adjust include */
    if(outfile == nil) {
        /*s: [[assemble()]] set outfile to {basename(infile)}.{thechar} */
        // outfile =  p =~ s/.s/.5/;
        outfile = p;
        if(outfile){
            p = utfrrune(outfile, '.');
            if(p)
                if(p[1] == 's' && p[2] == '\0')
                    p[0] = '\0';
            p = utfrune(outfile, '\0');
            p[0] = '.';
            p[1] = thechar;
            p[2] = '\0';
        } else
            outfile = "/dev/null";
        /*e: [[assemble()]] set outfile to {basename(infile)}.{thechar} */
    }
    /*s: [[assemble()]] setinclude("/{thestring}/include") or use INCLUDE */
    p = getenv("INCLUDE");
    if(p) {
        setinclude(p);
    } else {
         sprint(incfile,"/%s/include", thestring);
         setinclude(strdup(incfile));
    }
    /*e: [[assemble()]] setinclude("/{thestring}/include") or use INCLUDE */

    of = mycreat(outfile, 0664);
    /*s: [[assemble()]] sanity check [[of]] */
    if(of < 0) {
        yyerror("%ca: cannot create %s", thechar, outfile);
        errorexit();
    }
    /*e: [[assemble()]] sanity check [[of]] */
    Binit(&obuf, of, OWRITE);

    // Pass 1
    pass = 1;
    pinit(infile);
    /*s: [[assemble()]] init Dlist after pinit */
    for(i=0; i<nDlist; i++)
            dodefine(Dlist[i]);
    /*e: [[assemble()]] init Dlist after pinit */
    yyparse(); // calls outcode(), which does almost nothing when pass == 1
    /*s: [[assemble()]] sanity check nerrors */
    if(nerrors) {
        cclean();
        return nerrors;
    }
    /*e: [[assemble()]] sanity check nerrors */

    // Pass 2
    pass = 2;
    outhist(); // output file/line history information in object file
    pinit(infile);
    /*s: [[assemble()]] init Dlist after pinit */
    for(i=0; i<nDlist; i++)
            dodefine(Dlist[i]);
    /*e: [[assemble()]] init Dlist after pinit */
    yyparse(); // calls outcode() which now does things

    cclean();
    return nerrors;
}
/*e: function [[assemble]] */

/*s: struct [[Itab]](x86) */
struct Itab
{
    char	*name;

    //enum<token_kind>
    ushort	type;
    //enum<opcode_kind> | enum<operand_kind>
    ushort	value;
};
/*e: struct [[Itab]](x86) */
/*s: global [[itab]](x86) */
struct Itab itab[] =
{
    "SP",		LSP,	D_AUTO,
    "SB",		LSB,	D_EXTERN,
    "FP",		LFP,	D_PARAM,
    "PC",		LPC,	D_BRANCH,

    "AL",		LBREG,	D_AL,
    "CL",		LBREG,	D_CL,
    "DL",		LBREG,	D_DL,
    "BL",		LBREG,	D_BL,
    "AH",		LBREG,	D_AH,
    "CH",		LBREG,	D_CH,
    "DH",		LBREG,	D_DH,
    "BH",		LBREG,	D_BH,

    "AX",		LLREG,	D_AX,
    "CX",		LLREG,	D_CX,
    "DX",		LLREG,	D_DX,
    "BX",		LLREG,	D_BX,
/*	"SP",		LLREG,	D_SP,	*/
    "BP",		LLREG,	D_BP,
    "SI",		LLREG,	D_SI,
    "DI",		LLREG,	D_DI,

    "F0",		LFREG,	D_F0+0,
    "F1",		LFREG,	D_F0+1,
    "F2",		LFREG,	D_F0+2,
    "F3",		LFREG,	D_F0+3,
    "F4",		LFREG,	D_F0+4,
    "F5",		LFREG,	D_F0+5,
    "F6",		LFREG,	D_F0+6,
    "F7",		LFREG,	D_F0+7,

    "CS",		LSREG,	D_CS,
    "SS",		LSREG,	D_SS,
    "DS",		LSREG,	D_DS,
    "ES",		LSREG,	D_ES,
    "FS",		LSREG,	D_FS,
    "GS",		LSREG,	D_GS,

    "GDTR",		LBREG,	D_GDTR,
    "IDTR",		LBREG,	D_IDTR,
    "LDTR",		LBREG,	D_LDTR,
    "MSW",		LBREG,	D_MSW,
    "TASK",		LBREG,	D_TASK,

    "CR0",		LBREG,	D_CR+0,
    "CR1",		LBREG,	D_CR+1,
    "CR2",		LBREG,	D_CR+2,
    "CR3",		LBREG,	D_CR+3,
    "CR4",		LBREG,	D_CR+4,
    "CR5",		LBREG,	D_CR+5,
    "CR6",		LBREG,	D_CR+6,
    "CR7",		LBREG,	D_CR+7,

    "DR0",		LBREG,	D_DR+0,
    "DR1",		LBREG,	D_DR+1,
    "DR2",		LBREG,	D_DR+2,
    "DR3",		LBREG,	D_DR+3,
    "DR4",		LBREG,	D_DR+4,
    "DR5",		LBREG,	D_DR+5,
    "DR6",		LBREG,	D_DR+6,
    "DR7",		LBREG,	D_DR+7,

    "TR0",		LBREG,	D_TR+0,
    "TR1",		LBREG,	D_TR+1,
    "TR2",		LBREG,	D_TR+2,
    "TR3",		LBREG,	D_TR+3,
    "TR4",		LBREG,	D_TR+4,
    "TR5",		LBREG,	D_TR+5,
    "TR6",		LBREG,	D_TR+6,
    "TR7",		LBREG,	D_TR+7,



    "AAA",		LTYPE0,	AAAA,
    "AAD",		LTYPE0,	AAAD,
    "AAM",		LTYPE0,	AAAM,
    "AAS",		LTYPE0,	AAAS,
    "ADCB",		LTYPE3,	AADCB,
    "ADCL",		LTYPE3,	AADCL,
    "ADCW",		LTYPE3,	AADCW,
    "ADDB",		LTYPE3,	AADDB,
    "ADDL",		LTYPE3,	AADDL,
    "ADDW",		LTYPE3,	AADDW,
    "ADJSP",		LTYPE2,	AADJSP,
    "ANDB",		LTYPE3,	AANDB,
    "ANDL",		LTYPE3,	AANDL,
    "ANDW",		LTYPE3,	AANDW,
    "ARPL",		LTYPE3,	AARPL,
    "BOUNDL",		LTYPE3,	ABOUNDL,
    "BOUNDW",		LTYPE3,	ABOUNDW,
    "BSFL",		LTYPE3,	ABSFL,
    "BSFW",		LTYPE3,	ABSFW,
    "BSRL",		LTYPE3,	ABSRL,
    "BSRW",		LTYPE3,	ABSRW,
    "BTCL",		LTYPE3,	ABTCL,
    "BTCW",		LTYPE3,	ABTCW,
    "BTL",		LTYPE3,	ABTL,
    "BTRL",		LTYPE3,	ABTRL,
    "BTRW",		LTYPE3,	ABTRW,
    "BTSL",		LTYPE3,	ABTSL,
    "BTSW",		LTYPE3,	ABTSW,
    "BTW",		LTYPE3,	ABTW,
    "BYTE",		LTYPE2,	ABYTE,
    "CALL",		LTYPEC,	ACALL,
    "CLC",		LTYPE0,	ACLC,
    "CLD",		LTYPE0,	ACLD,
    "CLI",		LTYPE0,	ACLI,
    "CLTS",		LTYPE0,	ACLTS,
    "CMC",		LTYPE0,	ACMC,
    "CMPB",		LTYPE4,	ACMPB,
    "CMPL",		LTYPE4,	ACMPL,
    "CMPW",		LTYPE4,	ACMPW,
    "CMPSB",		LTYPE0,	ACMPSB,
    "CMPSL",		LTYPE0,	ACMPSL,
    "CMPSW",		LTYPE0,	ACMPSW,
    "CMPXCHGB",		LTYPE3,	ACMPXCHGB,
    "CMPXCHGL",		LTYPE3,	ACMPXCHGL,
    "CMPXCHGW",		LTYPE3,	ACMPXCHGW,
    "DAA",		LTYPE0,	ADAA,
    "DAS",		LTYPE0,	ADAS,
    "DATA",		LTYPED,	ADATA,
    "DECB",		LTYPE1,	ADECB,
    "DECL",		LTYPE1,	ADECL,
    "DECW",		LTYPE1,	ADECW,
    "DIVB",		LTYPE2,	ADIVB,
    "DIVL",		LTYPE2,	ADIVL,
    "DIVW",		LTYPE2,	ADIVW,
    "END",		LTYPE0,	AEND,
    "ENTER",		LTYPE2,	AENTER,
    "GLOBL",		LTYPEG,	AGLOBL,
    "HLT",		LTYPE0,	AHLT,
    "IDIVB",		LTYPE2,	AIDIVB,
    "IDIVL",		LTYPE2,	AIDIVL,
    "IDIVW",		LTYPE2,	AIDIVW,
    "IMULB",		LTYPEI,	AIMULB,
    "IMULL",		LTYPEI,	AIMULL,
    "IMULW",		LTYPEI,	AIMULW,
    "INB",		LTYPE0,	AINB,
    "INL",		LTYPE0,	AINL,
    "INW",		LTYPE0,	AINW,
    "INCB",		LTYPE1,	AINCB,
    "INCL",		LTYPE1,	AINCL,
    "INCW",		LTYPE1,	AINCW,
    "INSB",		LTYPE0,	AINSB,
    "INSL",		LTYPE0,	AINSL,
    "INSW",		LTYPE0,	AINSW,
    "INT",		LTYPE2,	AINT,
    "INTO",		LTYPE0,	AINTO,
    "IRETL",		LTYPE0,	AIRETL,
    "IRETW",		LTYPE0,	AIRETW,

    "JOS",		LTYPER,	AJOS,
    "JO",		LTYPER,	AJOS,	/* alternate */
    "JOC",		LTYPER,	AJOC,
    "JNO",		LTYPER,	AJOC,	/* alternate */
    "JCS",		LTYPER,	AJCS,
    "JB",		LTYPER,	AJCS,	/* alternate */
    "JC",		LTYPER,	AJCS,	/* alternate */
    "JNAE",		LTYPER,	AJCS,	/* alternate */
    "JLO",		LTYPER,	AJCS,	/* alternate */
    "JCC",		LTYPER,	AJCC,
    "JAE",		LTYPER,	AJCC,	/* alternate */
    "JNB",		LTYPER,	AJCC,	/* alternate */
    "JNC",		LTYPER,	AJCC,	/* alternate */
    "JHS",		LTYPER,	AJCC,	/* alternate */
    "JEQ",		LTYPER,	AJEQ,
    "JE",		LTYPER,	AJEQ,	/* alternate */
    "JZ",		LTYPER,	AJEQ,	/* alternate */
    "JNE",		LTYPER,	AJNE,
    "JNZ",		LTYPER,	AJNE,	/* alternate */
    "JLS",		LTYPER,	AJLS,
    "JBE",		LTYPER,	AJLS,	/* alternate */
    "JNA",		LTYPER,	AJLS,	/* alternate */
    "JHI",		LTYPER,	AJHI,
    "JA",		LTYPER,	AJHI,	/* alternate */
    "JNBE",		LTYPER,	AJHI,	/* alternate */
    "JMI",		LTYPER,	AJMI,
    "JS",		LTYPER,	AJMI,	/* alternate */
    "JPL",		LTYPER,	AJPL,
    "JNS",		LTYPER,	AJPL,	/* alternate */
    "JPS",		LTYPER,	AJPS,
    "JP",		LTYPER,	AJPS,	/* alternate */
    "JPE",		LTYPER,	AJPS,	/* alternate */
    "JPC",		LTYPER,	AJPC,
    "JNP",		LTYPER,	AJPC,	/* alternate */
    "JPO",		LTYPER,	AJPC,	/* alternate */
    "JLT",		LTYPER,	AJLT,
    "JL",		LTYPER,	AJLT,	/* alternate */
    "JNGE",		LTYPER,	AJLT,	/* alternate */
    "JGE",		LTYPER,	AJGE,
    "JNL",		LTYPER,	AJGE,	/* alternate */
    "JLE",		LTYPER,	AJLE,
    "JNG",		LTYPER,	AJLE,	/* alternate */
    "JGT",		LTYPER,	AJGT,
    "JG",		LTYPER,	AJGT,	/* alternate */
    "JNLE",		LTYPER,	AJGT,	/* alternate */

    "JCXZ",		LTYPER,	AJCXZ,
    "JMP",		LTYPEC,	AJMP,
    "LAHF",		LTYPE0,	ALAHF,
    "LARL",		LTYPE3,	ALARL,
    "LARW",		LTYPE3,	ALARW,
    "LEAL",		LTYPE3,	ALEAL,
    "LEAW",		LTYPE3,	ALEAW,
    "LEAVEL",		LTYPE0,	ALEAVEL,
    "LEAVEW",		LTYPE0,	ALEAVEW,
    "LOCK",		LTYPE0,	ALOCK,
    "LODSB",		LTYPE0,	ALODSB,
    "LODSL",		LTYPE0,	ALODSL,
    "LODSW",		LTYPE0,	ALODSW,
    "LONG",		LTYPE2,	ALONG,
    "LOOP",		LTYPER,	ALOOP,
    "LOOPEQ",		LTYPER,	ALOOPEQ,
    "LOOPNE",		LTYPER,	ALOOPNE,
    "LSLL",		LTYPE3,	ALSLL,
    "LSLW",		LTYPE3,	ALSLW,
    "MOVB",		LTYPE3,	AMOVB,
    "MOVL",		LTYPEM,	AMOVL,
    "MOVW",		LTYPEM,	AMOVW,
    "MOVBLSX",		LTYPE3, AMOVBLSX,
    "MOVBLZX",		LTYPE3, AMOVBLZX,
    "MOVBWSX",		LTYPE3, AMOVBWSX,
    "MOVBWZX",		LTYPE3, AMOVBWZX,
    "MOVWLSX",		LTYPE3, AMOVWLSX,
    "MOVWLZX",		LTYPE3, AMOVWLZX,
    "MOVSB",		LTYPE0,	AMOVSB,
    "MOVSL",		LTYPE0,	AMOVSL,
    "MOVSW",		LTYPE0,	AMOVSW,
    "MULB",		LTYPE2,	AMULB,
    "MULL",		LTYPE2,	AMULL,
    "MULW",		LTYPE2,	AMULW,
    "NEGB",		LTYPE1,	ANEGB,
    "NEGL",		LTYPE1,	ANEGL,
    "NEGW",		LTYPE1,	ANEGW,
    "NOP",		LTYPEN,	ANOP,
    "NOTB",		LTYPE1,	ANOTB,
    "NOTL",		LTYPE1,	ANOTL,
    "NOTW",		LTYPE1,	ANOTW,
    "ORB",		LTYPE3,	AORB,
    "ORL",		LTYPE3,	AORL,
    "ORW",		LTYPE3,	AORW,
    "OUTB",		LTYPE0,	AOUTB,
    "OUTL",		LTYPE0,	AOUTL,
    "OUTW",		LTYPE0,	AOUTW,
    "OUTSB",		LTYPE0,	AOUTSB,
    "OUTSL",		LTYPE0,	AOUTSL,
    "OUTSW",		LTYPE0,	AOUTSW,
    "POPAL",		LTYPE0,	APOPAL,
    "POPAW",		LTYPE0,	APOPAW,
    "POPFL",		LTYPE0,	APOPFL,
    "POPFW",		LTYPE0,	APOPFW,
    "POPL",		LTYPE1,	APOPL,
    "POPW",		LTYPE1,	APOPW,
    "PUSHAL",		LTYPE0,	APUSHAL,
    "PUSHAW",		LTYPE0,	APUSHAW,
    "PUSHFL",		LTYPE0,	APUSHFL,
    "PUSHFW",		LTYPE0,	APUSHFW,
    "PUSHL",		LTYPE2,	APUSHL,
    "PUSHW",		LTYPE2,	APUSHW,
    "RCLB",		LTYPE3,	ARCLB,
    "RCLL",		LTYPE3,	ARCLL,
    "RCLW",		LTYPE3,	ARCLW,
    "RCRB",		LTYPE3,	ARCRB,
    "RCRL",		LTYPE3,	ARCRL,
    "RCRW",		LTYPE3,	ARCRW,
    "REP",		LTYPE0,	AREP,
    "REPN",		LTYPE0,	AREPN,
    "RET",		LTYPE0,	ARET,
    "ROLB",		LTYPE3,	AROLB,
    "ROLL",		LTYPE3,	AROLL,
    "ROLW",		LTYPE3,	AROLW,
    "RORB",		LTYPE3,	ARORB,
    "RORL",		LTYPE3,	ARORL,
    "RORW",		LTYPE3,	ARORW,
    "SAHF",		LTYPE0,	ASAHF,
    "SALB",		LTYPE3,	ASALB,
    "SALL",		LTYPE3,	ASALL,
    "SALW",		LTYPE3,	ASALW,
    "SARB",		LTYPE3,	ASARB,
    "SARL",		LTYPE3,	ASARL,
    "SARW",		LTYPE3,	ASARW,
    "SBBB",		LTYPE3,	ASBBB,
    "SBBL",		LTYPE3,	ASBBL,
    "SBBW",		LTYPE3,	ASBBW,
    "SCASB",		LTYPE0,	ASCASB,
    "SCASL",		LTYPE0,	ASCASL,
    "SCASW",		LTYPE0,	ASCASW,
    "SETCC",		LTYPE1,	ASETCC,
    "SETCS",		LTYPE1,	ASETCS,
    "SETEQ",		LTYPE1,	ASETEQ,
    "SETGE",		LTYPE1,	ASETGE,
    "SETGT",		LTYPE1,	ASETGT,
    "SETHI",		LTYPE1,	ASETHI,
    "SETLE",		LTYPE1,	ASETLE,
    "SETLS",		LTYPE1,	ASETLS,
    "SETLT",		LTYPE1,	ASETLT,
    "SETMI",		LTYPE1,	ASETMI,
    "SETNE",		LTYPE1,	ASETNE,
    "SETOC",		LTYPE1,	ASETOC,
    "SETOS",		LTYPE1,	ASETOS,
    "SETPC",		LTYPE1,	ASETPC,
    "SETPL",		LTYPE1,	ASETPL,
    "SETPS",		LTYPE1,	ASETPS,
    "CDQ",		LTYPE0,	ACDQ,
    "CWD",		LTYPE0,	ACWD,
    "SHLB",		LTYPE3,	ASHLB,
    "SHLL",		LTYPES,	ASHLL,
    "SHLW",		LTYPES,	ASHLW,
    "SHRB",		LTYPE3,	ASHRB,
    "SHRL",		LTYPES,	ASHRL,
    "SHRW",		LTYPES,	ASHRW,
    "STC",		LTYPE0,	ASTC,
    "STD",		LTYPE0,	ASTD,
    "STI",		LTYPE0,	ASTI,
    "STOSB",		LTYPE0,	ASTOSB,
    "STOSL",		LTYPE0,	ASTOSL,
    "STOSW",		LTYPE0,	ASTOSW,
    "SUBB",		LTYPE3,	ASUBB,
    "SUBL",		LTYPE3,	ASUBL,
    "SUBW",		LTYPE3,	ASUBW,
    "SYSCALL",		LTYPE0,	ASYSCALL,
    "TESTB",		LTYPE3,	ATESTB,
    "TESTL",		LTYPE3,	ATESTL,
    "TESTW",		LTYPE3,	ATESTW,
    "TEXT",		LTYPET,	ATEXT,
    "VERR",		LTYPE2,	AVERR,
    "VERW",		LTYPE2,	AVERW,
    "WAIT",		LTYPE0,	AWAIT,
    "WORD",		LTYPE2,	AWORD,
    "XCHGB",		LTYPE3,	AXCHGB,
    "XCHGL",		LTYPE3,	AXCHGL,
    "XCHGW",		LTYPE3,	AXCHGW,
    "XLAT",		LTYPE2,	AXLAT,
    "XORB",		LTYPE3,	AXORB,
    "XORL",		LTYPE3,	AXORL,
    "XORW",		LTYPE3,	AXORW,

    "CMOVLCC",		LTYPE3,	ACMOVLCC,
    "CMOVLCS",		LTYPE3,	ACMOVLCS,
    "CMOVLEQ",		LTYPE3,	ACMOVLEQ,
    "CMOVLGE",		LTYPE3,	ACMOVLGE,
    "CMOVLGT",		LTYPE3,	ACMOVLGT,
    "CMOVLHI",		LTYPE3,	ACMOVLHI,
    "CMOVLLE",		LTYPE3,	ACMOVLLE,
    "CMOVLLS",		LTYPE3,	ACMOVLLS,
    "CMOVLLT",		LTYPE3,	ACMOVLLT,
    "CMOVLMI",		LTYPE3,	ACMOVLMI,
    "CMOVLNE",		LTYPE3,	ACMOVLNE,
    "CMOVLOC",		LTYPE3,	ACMOVLOC,
    "CMOVLOS",		LTYPE3,	ACMOVLOS,
    "CMOVLPC",		LTYPE3,	ACMOVLPC,
    "CMOVLPL",		LTYPE3,	ACMOVLPL,
    "CMOVLPS",		LTYPE3,	ACMOVLPS,
    "CMOVWCC",		LTYPE3,	ACMOVWCC,
    "CMOVWCS",		LTYPE3,	ACMOVWCS,
    "CMOVWEQ",		LTYPE3,	ACMOVWEQ,
    "CMOVWGE",		LTYPE3,	ACMOVWGE,
    "CMOVWGT",		LTYPE3,	ACMOVWGT,
    "CMOVWHI",		LTYPE3,	ACMOVWHI,
    "CMOVWLE",		LTYPE3,	ACMOVWLE,
    "CMOVWLS",		LTYPE3,	ACMOVWLS,
    "CMOVWLT",		LTYPE3,	ACMOVWLT,
    "CMOVWMI",		LTYPE3,	ACMOVWMI,
    "CMOVWNE",		LTYPE3,	ACMOVWNE,
    "CMOVWOC",		LTYPE3,	ACMOVWOC,
    "CMOVWOS",		LTYPE3,	ACMOVWOS,
    "CMOVWPC",		LTYPE3,	ACMOVWPC,
    "CMOVWPL",		LTYPE3,	ACMOVWPL,
    "CMOVWPS",		LTYPE3,	ACMOVWPS,
                	
    "FMOVB",		LTYPE3, AFMOVB,
    "FMOVBP",		LTYPE3, AFMOVBP,
    "FMOVD",		LTYPE3, AFMOVD,
    "FMOVDP",		LTYPE3, AFMOVDP,
    "FMOVF",		LTYPE3, AFMOVF,
    "FMOVFP",		LTYPE3, AFMOVFP,
    "FMOVL",		LTYPE3, AFMOVL,
    "FMOVLP",		LTYPE3, AFMOVLP,
    "FMOVV",		LTYPE3, AFMOVV,
    "FMOVVP",		LTYPE3, AFMOVVP,
    "FMOVW",		LTYPE3, AFMOVW,
    "FMOVWP",		LTYPE3, AFMOVWP,
    "FMOVX",		LTYPE3, AFMOVX,
    "FMOVXP",		LTYPE3, AFMOVXP,
    "FCMOVCC",		LTYPE3, AFCMOVCC,
    "FCMOVCS",		LTYPE3, AFCMOVCS,
    "FCMOVEQ",		LTYPE3, AFCMOVEQ,
    "FCMOVHI",		LTYPE3, AFCMOVHI,
    "FCMOVLS",		LTYPE3, AFCMOVLS,
    "FCMOVNE",		LTYPE3, AFCMOVNE,
    "FCMOVNU",		LTYPE3, AFCMOVNU,
    "FCMOVUN",		LTYPE3, AFCMOVUN,
    "FCOMB",		LTYPE3, AFCOMB,
    "FCOMBP",		LTYPE3, AFCOMBP,
    "FCOMD",		LTYPE3, AFCOMD,
    "FCOMDP",		LTYPE3, AFCOMDP,
    "FCOMDPP",		LTYPE3, AFCOMDPP,
    "FCOMF",		LTYPE3, AFCOMF,
    "FCOMFP",		LTYPE3, AFCOMFP,
    "FCOMI",		LTYPE3, AFCOMI,
    "FCOMIP",		LTYPE3, AFCOMIP,
    "FCOML",		LTYPE3, AFCOML,
    "FCOMLP",		LTYPE3, AFCOMLP,
    "FCOMW",		LTYPE3, AFCOMW,
    "FCOMWP",		LTYPE3, AFCOMWP,
    "FUCOM",		LTYPE3, AFUCOM,
    "FUCOMI",		LTYPE3, AFUCOMI,
    "FUCOMIP",		LTYPE3, AFUCOMIP,
    "FUCOMP",		LTYPE3, AFUCOMP,
    "FUCOMPP",		LTYPE3, AFUCOMPP,
    "FADDW",		LTYPE3, AFADDW,
    "FADDL",		LTYPE3, AFADDL,
    "FADDF",		LTYPE3, AFADDF,
    "FADDD",		LTYPE3, AFADDD,
    "FADDDP",		LTYPE3, AFADDDP,
    "FSUBDP",		LTYPE3, AFSUBDP,
    "FSUBW",		LTYPE3, AFSUBW,
    "FSUBL",		LTYPE3, AFSUBL,
    "FSUBF",		LTYPE3, AFSUBF,
    "FSUBD",		LTYPE3, AFSUBD,
    "FSUBRDP",		LTYPE3, AFSUBRDP,
    "FSUBRW",		LTYPE3, AFSUBRW,
    "FSUBRL",		LTYPE3, AFSUBRL,
    "FSUBRF",		LTYPE3, AFSUBRF,
    "FSUBRD",		LTYPE3, AFSUBRD,
    "FMULDP",		LTYPE3, AFMULDP,
    "FMULW",		LTYPE3, AFMULW,
    "FMULL",		LTYPE3, AFMULL,
    "FMULF",		LTYPE3, AFMULF,
    "FMULD",		LTYPE3, AFMULD,
    "FDIVDP",		LTYPE3, AFDIVDP,
    "FDIVW",		LTYPE3, AFDIVW,
    "FDIVL",		LTYPE3, AFDIVL,
    "FDIVF",		LTYPE3, AFDIVF,
    "FDIVD",		LTYPE3, AFDIVD,
    "FDIVRDP",		LTYPE3, AFDIVRDP,
    "FDIVRW",		LTYPE3, AFDIVRW,
    "FDIVRL",		LTYPE3, AFDIVRL,
    "FDIVRF",		LTYPE3, AFDIVRF,
    "FDIVRD",		LTYPE3, AFDIVRD,
    "FXCHD",		LTYPE3, AFXCHD,
    "FFREE",		LTYPE1, AFFREE,
    "FLDCW",		LTYPE2, AFLDCW,
    "FLDENV",		LTYPE1, AFLDENV,
    "FRSTOR",		LTYPE2, AFRSTOR,
    "FSAVE",		LTYPE1, AFSAVE,
    "FSTCW",		LTYPE1, AFSTCW,
    "FSTENV",		LTYPE1, AFSTENV,
    "FSTSW",		LTYPE1, AFSTSW,
    "F2XM1",		LTYPE0, AF2XM1,
    "FABS",		LTYPE0, AFABS,
    "FCHS",		LTYPE0, AFCHS,
    "FCLEX",		LTYPE0, AFCLEX,
    "FCOS",		LTYPE0, AFCOS,
    "FDECSTP",		LTYPE0, AFDECSTP,
    "FINCSTP",		LTYPE0, AFINCSTP,
    "FINIT",		LTYPE0, AFINIT,
    "FLD1",		LTYPE0, AFLD1,
    "FLDL2E",		LTYPE0, AFLDL2E,
    "FLDL2T",		LTYPE0, AFLDL2T,
    "FLDLG2",		LTYPE0, AFLDLG2,
    "FLDLN2",		LTYPE0, AFLDLN2,
    "FLDPI",		LTYPE0, AFLDPI,
    "FLDZ",		LTYPE0, AFLDZ,
    "FNOP",		LTYPE0, AFNOP,
    "FPATAN",		LTYPE0, AFPATAN,
    "FPREM",		LTYPE0, AFPREM,
    "FPREM1",		LTYPE0, AFPREM1,
    "FPTAN",		LTYPE0, AFPTAN,
    "FRNDINT",		LTYPE0, AFRNDINT,
    "FSCALE",		LTYPE0, AFSCALE,
    "FSIN",		LTYPE0, AFSIN,
    "FSINCOS",		LTYPE0, AFSINCOS,
    "FSQRT",		LTYPE0, AFSQRT,
    "FTST",		LTYPE0, AFTST,
    "FXAM",		LTYPE0, AFXAM,
    "FXTRACT",		LTYPE0, AFXTRACT,
    "FYL2X",		LTYPE0, AFYL2X,
    "FYL2XP1",		LTYPE0, AFYL2XP1,

    0
};
/*e: global [[itab]](x86) */

/*s: function [[cinit]](x86) */
void
cinit(void)
{
    Sym *s;
    int i;

    nullgen.sym = S;
    nullgen.offset = 0;
    if(FPCHIP)
        nullgen.dval = 0;
    for(i=0; i<sizeof(nullgen.sval); i++)
        nullgen.sval[i] = '\0';
    nullgen.type = D_NONE;
    nullgen.index = D_NONE;
    nullgen.scale = 0;

    for(i=0; i<NHASH; i++)
        hash[i] = S;
    for(i=0; itab[i].name; i++) {
        s = slookup(itab[i].name);
        if(s->type != LNAME)
            yyerror("double initialization %s", itab[i].name);
        s->type = itab[i].type;
        s->value = itab[i].value;
    }

    pathname = allocn(pathname, 0, 100);
    if(getwd(pathname, 99) == nil) {
        pathname = allocn(pathname, 100, 900);
        if(getwd(pathname, 999) == nil)
            strcpy(pathname, "/???");
    }
}
/*e: function [[cinit]](x86) */

/*s: function [[checkscale]](x86) */
void
checkscale(int scale)
{

    switch(scale) {
    case 1:
    case 2:
    case 4:
    case 8:
        return;
    }
    yyerror("scale must be 1248: %d", scale);
}
/*e: function [[checkscale]](x86) */

/*s: function [[syminit]] */
void
syminit(Sym *sym)
{
    sym->type = LNAME;
    sym->value = 0;
}
/*e: function [[syminit]] */

/*s: function [[cclean]](x86) */
void
cclean(void)
{
    Gen2 g2 = (Gen2) { nullgen, nullgen };

    outcode(AEND, &g2);
    Bflush(&obuf);
}
/*e: function [[cclean]](x86) */

/*s: function [[zname]](x86) */
void
zname(char *n, int t, int s)
{

    Bputc(&obuf, ANAME);		/* as(2) */
    Bputc(&obuf, ANAME>>8);
    Bputc(&obuf, t);		/* type */
    Bputc(&obuf, s);		/* sym */
    while(*n) {
        Bputc(&obuf, *n);
        n++;
    }
    Bputc(&obuf, '\0');
}
/*e: function [[zname]](x86) */

/*s: function [[zaddr]](x86) */
void
zaddr(Gen *a, int s)
{
    // bitset<enum<misc2>>
    int t;
    long l;
    int i;
    char *n;
    Ieee e;

    t = 0;
    if(a->index != D_NONE || a->scale != 0)
        t |= T_INDEX;
    if(a->offset != 0)
        t |= T_OFFSET;
    if(s != 0)
        t |= T_SYM;

    switch(a->type) {
    case D_NONE:
        break;
    case D_FCONST:
        t |= T_FCONST;
        break;
    case D_CONST2:
        t |= T_OFFSET|T_OFFSET2;
        break;
    case D_SCONST:
        t |= T_SCONST;
        break;
    default:
        t |= T_TYPE;
        break;
    }

    Bputc(&obuf, t);

    if(t & T_INDEX) {	/* implies index, scale */
        Bputc(&obuf, a->index);
        Bputc(&obuf, a->scale);
    }
    if(t & T_OFFSET) {	/* implies offset */
        l = a->offset;
        Bputc(&obuf, l);
        Bputc(&obuf, l>>8);
        Bputc(&obuf, l>>16);
        Bputc(&obuf, l>>24);
    }
    if(t & T_OFFSET2) {
        l = a->offset2;
        Bputc(&obuf, l);
        Bputc(&obuf, l>>8);
        Bputc(&obuf, l>>16);
        Bputc(&obuf, l>>24);
    }
    if(t & T_SYM)		/* implies sym */
        Bputc(&obuf, s);


    if(t & T_FCONST) {
        ieeedtod(&e, a->dval);
        l = e.l;
        Bputc(&obuf, l);
        Bputc(&obuf, l>>8);
        Bputc(&obuf, l>>16);
        Bputc(&obuf, l>>24);
        l = e.h;
        Bputc(&obuf, l);
        Bputc(&obuf, l>>8);
        Bputc(&obuf, l>>16);
        Bputc(&obuf, l>>24);
        return;
    }
    if(t & T_SCONST) {
        n = a->sval;
        for(i=0; i<sizeof(nullgen.sval); i++) {
            Bputc(&obuf, *n);
            n++;
        }
        return;
    }

    if(t & T_TYPE)
        Bputc(&obuf, a->type);
}
/*e: function [[zaddr]](x86) */

/*s: function [[outcode]](x86) */
void
outcode(int a, Gen2 *g2)
{
    // symbol from, index in h[]
    int sf;
    // symbol to, index in h[]
    int st;
    // enum<operand_kind>
    int t;
    Sym *s;

    if(pass == 1)
        goto out;

jackpot:
    sf = 0;
    s = g2->from.sym;

    while(s != S) {
        sf = s->symidx;

        if(sf < 0 || sf >= NSYM)
            sf = 0;

        t = g2->from.type;
        if(t == D_ADDR)
            t = g2->from.index;

        if(h[sf].symkind == t)
            if(h[sf].sym == s)
                break;

        zname(s->name, t, symcounter);
        s->symidx = symcounter;
        h[symcounter].sym = s;
        h[symcounter].symkind = t;
        sf = symcounter;
        symcounter++;
        if(symcounter >= NSYM)
            symcounter = 1;
        break;
    }

    st = 0;
    s = g2->to.sym;

    while(s != S) {
        st = s->symidx;

        if(st < 0 || st >= NSYM)
            st = 0;

        t = g2->to.type;
        if(t == D_ADDR)
            t = g2->to.index;

        if(h[st].symkind == t)
            if(h[st].sym == s)
                break;

        zname(s->name, t, symcounter);
        s->symidx = symcounter;
        h[symcounter].sym = s;
        h[symcounter].symkind = t;
        st = symcounter;
        symcounter++;
        if(symcounter >= NSYM)
            symcounter = 1;

        if(st == sf)
            goto jackpot;
        break;
    }

    Bputc(&obuf, a);
    Bputc(&obuf, a>>8);
    Bputc(&obuf, lineno);
    Bputc(&obuf, lineno>>8);
    Bputc(&obuf, lineno>>16);
    Bputc(&obuf, lineno>>24);
    zaddr(&g2->from, sf);
    zaddr(&g2->to, st);

out:
    if(a != AGLOBL && a != ADATA)
        pc++;
}
/*e: function [[outcode]](x86) */

/*s: function [[outhist]](x86) */
void
outhist(void)
{
    Gen g;
    Hist *h;
    char *p, *q, *op, c;
    int n;

    g = nullgen;
    c = pathchar();

    for(h = hist; h != H; h = h->link) {
        p = h->filename;
        op = nil;
        // relative file?
        if(p && p[0] != c && h->local_line == 0 && pathname){
            if(pathname[0] == c){
                op = p;
                p = pathname;
            }
        }
        while(p) {
            q = strchr(p, c);
            if(q) {
                n = q-p;
                if(n == 0){
                    n = 1;	/* leading "/" */
                    *p = '/';	/* don't emit "\" on windows */
                }
                q++;
            } else {
                n = strlen(p);
                q = nil;
            }

            if(n) {
                Bputc(&obuf, ANAME);
                Bputc(&obuf, ANAME>>8);
                Bputc(&obuf, D_FILE);	/* type */
                Bputc(&obuf, 1);	/* sym */
                Bputc(&obuf, '<');
                Bwrite(&obuf, p, n);
                Bputc(&obuf, '\0');
            }
            p = q;
            if(p == nil && op) {
                p = op;
                op = nil;
            }
        }

        g.offset = h->local_line;

        Bputc(&obuf, AHISTORY);
        Bputc(&obuf, AHISTORY>>8);

        Bputc(&obuf, h->global_line);
        Bputc(&obuf, h->global_line>>8);
        Bputc(&obuf, h->global_line>>16);
        Bputc(&obuf, h->global_line>>24);

        zaddr(&nullgen, 0);
        zaddr(&g, 0);
    }
}
/*e: function [[outhist]](x86) */

// now use aa.a8
//#include "../cc/lexbody"
//#include "../cc/compat"

// used to be in ../cc/lexbody and factorized between assemblers by
// using #include, but ugly, so I copy pasted the function for now
/*s: function [[yylex]] */
/// main -> assemble -> yyparse -> <>
long
yylex(void)
{
    int c;
    /*s: [[yylex()]] locals */
    int c1;
    /*x: [[yylex()]] locals */
    // ref<char> (target = symb)
    char *cp;
    // ref<Symbol> (owner = hash)
    Sym *s;
    /*x: [[yylex()]] locals */
    int baselog2;
    /*x: [[yylex()]] locals */
    int i;
    /*e: [[yylex()]] locals */

    /*s: [[yylex()]] peekc handling, starting part */
    c = peekc;
    if(c != IGN) {
        peekc = IGN; // consume the extra character saved in peekc
        goto l1; // skip the GETC(), we already have a character in c
    }
    /*e: [[yylex()]] peekc handling, starting part */
l0:
    c = GETC();
l1:
    if(c == EOF) {
        return EOF;
    }

    if(isspace(c)) {
        /*s: [[yylex()]] if c is newline */
        if(c == '\n') {
            lineno++;
            return ';'; // newline transformed in fake ';'
        }
        /*e: [[yylex()]] if c is newline */
        // ignore spaces
        goto l0;
    }
    // else

    /*s: [[yylex()]] before switch, if isxxx */
    if(isalpha(c))
        goto talph;
    /*x: [[yylex()]] before switch, if isxxx */
    if(isdigit(c))
        goto tnum;
    /*e: [[yylex()]] before switch, if isxxx */
    switch(c) {
//XxX: missing?
//    case '\n':
//        lineno++;
//        return ';';
    /*s: [[yylex()]] switch c cases */
    case '/':
        c1 = GETC();
        if(c1 == '/') {
            // '/''/' read; skip everything until next '\n'
            for(;;) {
                c = GETC();
                if(c == '\n')
                    goto l1; // which will convert the \n in c in a ';'
                if(c == EOF) {
                    yyerror("eof in comment");
                    errorexit();
                }
            }
        }
        if(c1 == '*') {
            // '/''*' read; skip everything until next '*''/'
            for(;;) {
                c = GETC();
                while(c == '*') { // not an if! to handle /** not finished */
                    c = GETC();
                    if(c == '/')
                        goto l0;
                }
                if(c == EOF) {
                    yyerror("eof in comment");
                    errorexit();
                }
                if(c == '\n')
                    lineno++;
            }
        }
        break;
    /*x: [[yylex()]] switch c cases */
    case '_':
    case '@':
    // case 'a'..'z' 'A'..'Z': (isalpha())
    // XxX: case '.' too
    talph:
        cp = symb;

    aloop:
        *cp++ = c;
        c = GETC();
        if(isalpha(c) || isdigit(c) || c == '_' || c == '$')
            goto aloop;
        // went too far
        peekc = c;

        *cp = '\0';
        s = lookup(); // uses symb global (referenced by cp)
        /*s: [[yylex()]] if macro symbol */
        if(s->macro) {
            newio();
            cp = ionext->b;
            macexpand(s, cp);
            pushio();

            ionext->link = iostack;
            iostack = ionext;

            fi.p = cp;
            fi.c = strlen(cp);
            if(peekc != IGN) {
                cp[fi.c++] = peekc;
                cp[fi.c] = 0;
                peekc = IGN;
            }
            goto l0;
        }
        /*e: [[yylex()]] if macro symbol */
        //XxX?
        //if(s->type == 0)
        //    s->type = LNAME;

        /*s: [[yylex()]] in identifier case, set yylval */
        if(s->type == LNAME || s->type == LLAB || s->type == LVAR) {
            yylval.sym = s;
        } else {
            yylval.lval = s->value;
        }
        /*e: [[yylex()]] in identifier case, set yylval */
        return s->type;
    /*x: [[yylex()]] switch c cases */
    // case '0'..'9': (isdigit())
    tnum:
        cp = symb;
        if(c != '0')
            goto dc;
        // else, read a '0', maybe the start of an hexadecimal number
        /*s: [[yylex()]] in number case, 0xxx handling */
        *cp++ = c;
        c = GETC();
        baselog2 = 3; // 2^3, for octal
        if(c == 'x' || c == 'X') {
            baselog2 = 4; // 2^4, for hexadecimal
            c = GETC();
        } 
        else if(c < '0' || c > '7')
            goto dc;

        yylval.lval = 0;
        for(;;) {
            if(c >= '0' && c <= '9') {
                if(c > '7' && baselog2 == 3)
                    break;
                yylval.lval <<= baselog2;
                yylval.lval += c - '0';
                c = GETC();
                continue;
            }
            // else
            if(baselog2 == 3)
                break;
            // else
            /*s: [[yylex()]] in number case, 0xxx handling, normalize letters */
            if(c >= 'A' && c <= 'F')
                // c = lowercase(c)
                c += 'a' - 'A';
            /*e: [[yylex()]] in number case, 0xxx handling, normalize letters */
            if(c >= 'a' && c <= 'f') {
                yylval.lval <<= baselog2;
                yylval.lval += c - 'a' + 10;
                c = GETC();
                continue;
            }
            break;
        }
        //XxX: goto ncu;
        peekc = c;
        return LCONST;
        /*e: [[yylex()]] in number case, 0xxx handling */

    /*s: [[yylex()]] in number case, decimal dc label handling */
    dc:
        for(;;) {
            if(!isdigit(c))
                break;
            *cp++ = c;
            c = GETC();
        }
        /*s: [[yylex()]] in number case, in decimal case, float handling */
        if(c == '.')
            goto casedot;
        if(c == 'e' || c == 'E')
            goto casee;
         //XxX:         *cp = 0;
         //XxX:         if(sizeof(yylval.lval) == sizeof(vlong))
         //XxX:             yylval.lval = strtoll(symb, nil, 10);
         //XxX:         else
         //XxX:             yylval.lval = strtol(symb, nil, 10);
         //XxX: 
         //XxX:     ncu:
         //XxX:         while(c == 'U' || c == 'u' || c == 'l' || c == 'L')
         //XxX:             c = GETC();

        /*e: [[yylex()]] in number case, in decimal case, float handling */
        *cp = '\0';
        yylval.lval = strtol(symb, nil, 10);

        peekc = c;
        return LCONST;
    /*e: [[yylex()]] in number case, decimal dc label handling */
    /*s: [[yylex()]] in number case, float labels handling */
    casedot:
        for(;;) {
            *cp++ = c;
            c = GETC();
            if(!isdigit(c))
                break;
        }
        if(c == 'e' || c == 'E')
            goto casee;
        goto caseout;

    casee:
        *cp++ = 'e';
        c = GETC();
        if(c == '+' || c == '-') {
            *cp++ = c;
            c = GETC();
        }
        while(isdigit(c)) {
            *cp++ = c;
            c = GETC();
        }

    caseout:
        *cp = '\0';
        peekc = c;
        if(FPCHIP) {
            yylval.dval = atof(symb);
            return LFCONST;
        } else {
            yyerror("assembler cannot interpret fp constants");
            yylval.lval = 1L;
            return LCONST;
        }
    /*e: [[yylex()]] in number case, float labels handling */
    /*x: [[yylex()]] switch c cases */
    case '.':
        c = GETC();
        if(isalpha(c)) { // an identifier
            cp = symb;
            *cp++ = '.';
            goto aloop;
        }
        if(isdigit(c)) { // a float
            cp = symb;
            *cp++ = '.';
            goto casedot;
        }
        // else
        peekc = c;
        return '.'; // a single '.'
    /*x: [[yylex()]] switch c cases */
    case '\'':
        c = escchar('\'');
        /*s: [[yylex()]] in character case, if c is EOF */
        if(c == EOF)
            c = '\'';
        /*e: [[yylex()]] in character case, if c is EOF */
        if(escchar('\'') != EOF)
            yyerror("missing '");

        yylval.lval = c;
        return LCONST;
    /*x: [[yylex()]] switch c cases */
    case '"':
        memcpy(yylval.sval, nullgen.sval, sizeof(yylval.sval));
        cp = yylval.sval;
        i = 0;
        for(;;) {
            c = escchar('"');
            if(c == EOF)
                break;
            if(i < sizeof(yylval.sval))
                *cp++ = c;
            i++;
        }
        if(i > sizeof(yylval.sval))
            yyerror("string constant too long");
        return LSCONST;
    /*x: [[yylex()]] switch c cases */
    case '#':
        domacro();
        goto l0;
    /*e: [[yylex()]] switch c cases */
    default:
        return c;
    }
    /*s: [[yylex()]] peekc handling, ending part */
    peekc = c1;
    /*e: [[yylex()]] peekc handling, ending part */
    return c;
}
/*e: function [[yylex]] */

// #include "../cc/macbody"
/*e: assemblers/8a/lex.c */
