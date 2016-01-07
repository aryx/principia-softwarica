/*s: linkers/5l/optab.c */
#include	"l.h"
#include	"m.h"

/*s: global optab (linkers/5l/optab.c)(arm) */
Optab	optab[] =
{
    /*s: [[optab]] entries */
    { ATEXT,	C_LEXT,	C_NONE,	C_LCON, 	 0, 0 },
    { ATEXT,	C_LEXT,	C_REG,	C_LCON, 	 0, 0 },
    /*x: [[optab]] entries */
    { AWORD,	C_NONE,	C_NONE,	C_LCON,		11, 4 },
    /*x: [[optab]] entries */
    //          From    Middle  To       
    { AADD,     C_REG,  C_REG,  C_REG,       1, 4 },
    { AADD,     C_REG,  C_NONE, C_REG,       1, 4 },

    { AMOVW,    C_REG,  C_NONE, C_REG,       1, 4 },
    { AMVN,     C_REG,  C_NONE, C_REG,       1, 4 },
    { ACMP,     C_REG,  C_REG,  C_NONE,      1, 4 },
    /*x: [[optab]] entries */
    { AADD,		C_RCON,	C_REG,	C_REG,		 2, 4 },
    { AADD,		C_RCON,	C_NONE,	C_REG,		 2, 4 },

    { AMOVW,	C_RCON,	C_NONE,	C_REG,		 2, 4 },
    { AMVN,		C_RCON,	C_NONE,	C_REG,		 2, 4 },
    { ACMP,		C_RCON,	C_REG,	C_NONE,		 2, 4 },
    /*x: [[optab]] entries */
    { AADD,		C_SHIFT,C_REG,	C_REG,		 3, 4 },
    { AADD,		C_SHIFT,C_NONE,	C_REG,		 3, 4 },

    { AMVN,		C_SHIFT,C_NONE,	C_REG,		 3, 4 },
    { ACMP,		C_SHIFT,C_REG,	C_NONE,		 3, 4 },
    /*x: [[optab]] entries */
    { ASLL,		C_RCON,	C_REG,	C_REG,		 8, 4 },
    { ASLL,		C_RCON,	C_NONE,	C_REG,		 8, 4 },
    /*x: [[optab]] entries */
    { ASLL,		C_REG,	C_NONE,	C_REG,		 9, 4 },
    { ASLL,		C_REG,	C_REG,	C_REG,		 9, 4 },
    /*x: [[optab]] entries */
    { AMOVB,	C_REG,	C_NONE,	C_REG,		14, 8 },
    { AMOVBU,	C_REG,	C_NONE,	C_REG,		58, 4 },
    { AMOVH,	C_REG,	C_NONE,	C_REG,		14, 8 },
    { AMOVHU,	C_REG,	C_NONE,	C_REG,		14, 8 },
    /*x: [[optab]] entries */
    { AMUL,		C_REG,	C_REG,	C_REG,		15, 4 },
    { AMUL,		C_REG,	C_NONE,	C_REG,		15, 4 },
    /*x: [[optab]] entries */
    { AADD,		C_NCON,	C_REG,	C_REG,		13, 8 },
    { AADD,		C_NCON,	C_NONE,	C_REG,		13, 8 },
    { AMVN,		C_NCON,	C_NONE,	C_REG,		13, 8 },
    { ACMP,		C_NCON,	C_REG,	C_NONE,		13, 8 },
    /*x: [[optab]] entries */
    { AADD,		C_LCON,	C_REG,	C_REG,		13, 8, 0,	LFROM },
    { AADD,		C_LCON,	C_NONE,	C_REG,		13, 8, 0,	LFROM },
    { AMVN,		C_LCON,	C_NONE,	C_REG,		13, 8, 0,	LFROM },
    { ACMP,		C_LCON,	C_REG,	C_NONE,		13, 8, 0,	LFROM },
    /*x: [[optab]] entries */
    { AMOVW,	C_NCON,	C_NONE,	C_REG,		12, 4 },
    { AMOVW,	C_LCON,	C_NONE,	C_REG,		12, 4, 0,	LFROM },
    /*x: [[optab]] entries */
    { AB,		C_NONE,	C_NONE,	C_BRANCH,		 5, 4, 0,	LPOOL },
    { ABL,		C_NONE,	C_NONE,	C_BRANCH,		 5, 4 },
    { ABEQ,		C_NONE,	C_NONE,	C_BRANCH,		 5, 4 },
    /*x: [[optab]] entries */
    { AB,		C_NONE,	C_NONE,	C_ROREG,	 6, 4, 0,	LPOOL },
    { ABL,		C_NONE,	C_NONE,	C_ROREG,	 7, 8 },
    /*x: [[optab]] entries */
    { AMOVW,	C_SEXT,	C_NONE,	C_REG,		21, 4, REGSB },
    { AMOVW,	C_SAUTO,C_NONE,	C_REG,		21, 4, REGSP },
    { AMOVW,	C_SOREG,C_NONE,	C_REG,		21, 4 },

    { AMOVBU,	C_SEXT,	C_NONE,	C_REG,		21, 4, REGSB },
    { AMOVBU,	C_SAUTO,C_NONE,	C_REG,		21, 4, REGSP },
    { AMOVBU,	C_SOREG,C_NONE,	C_REG,		21, 4 },
    /*x: [[optab]] entries */
    { AMOVW,	C_LEXT,	C_NONE,	C_REG,		31, 8, REGSB,	LFROM },
    { AMOVW,	C_LAUTO,C_NONE,	C_REG,		31, 8, REGSP,	LFROM },
    { AMOVW,	C_LOREG,C_NONE,	C_REG,		31, 8, 0,	LFROM },

    { AMOVBU,	C_LEXT,	C_NONE,	C_REG,		31, 8, REGSB,	LFROM },
    { AMOVBU,	C_LAUTO,C_NONE,	C_REG,		31, 8, REGSP,	LFROM },
    { AMOVBU,	C_LOREG,C_NONE,	C_REG,		31, 8, 0,	LFROM },
    /*x: [[optab]] entries */
    { AMOVW,	C_REG,	C_NONE,	C_SEXT,		20, 4, REGSB },
    { AMOVW,	C_REG,	C_NONE,	C_SAUTO,	20, 4, REGSP },
    { AMOVW,	C_REG,	C_NONE,	C_SOREG,	20, 4, 0 },

    { AMOVBU,	C_REG,	C_NONE,	C_SEXT,		20, 4, REGSB },
    { AMOVBU,	C_REG,	C_NONE,	C_SAUTO,	20, 4, REGSP },
    { AMOVBU,	C_REG,	C_NONE,	C_SOREG,	20, 4, 0 },
    /*x: [[optab]] entries */
    { AMOVW,	C_REG,	C_NONE,	C_LEXT,		30, 8, REGSB,	LTO },
    { AMOVW,	C_REG,	C_NONE,	C_LAUTO,	30, 8, REGSP,	LTO },
    { AMOVW,	C_REG,	C_NONE,	C_LOREG,	30, 8, 0,	LTO },

    { AMOVBU,	C_REG,	C_NONE,	C_LEXT,		30, 8, REGSB,	LTO },
    { AMOVBU,	C_REG,	C_NONE,	C_LAUTO,	30, 8, REGSP,	LTO },
    { AMOVBU,	C_REG,	C_NONE,	C_LOREG,	30, 8, 0,	LTO },
    /*x: [[optab]] entries */
    { AMOVB,	C_REG,	C_NONE,	C_SEXT,		20, 4, REGSB },
    { AMOVB,	C_REG,	C_NONE,	C_SAUTO,	20, 4, REGSP },
    { AMOVB,	C_REG,	C_NONE,	C_SOREG,	20, 4, 0 },
    /*x: [[optab]] entries */
    { AMOVB,	C_REG,	C_NONE,	C_LEXT,		30, 8, REGSB,	LTO },
    { AMOVB,	C_REG,	C_NONE,	C_LAUTO,	30, 8, REGSP,	LTO },
    { AMOVB,	C_REG,	C_NONE,	C_LOREG,	30, 8, 0,	LTO },
    /*x: [[optab]] entries */
    { ASWPW,	C_SOREG,C_REG,	C_REG,		40, 4 },
    /*x: [[optab]] entries */
    { AMOVW,	C_RECON,C_NONE,	C_REG,		 4, 4, REGSB },
    { AMOVW,	C_RACON,C_NONE,	C_REG,		 4, 4, REGSP },
    /*x: [[optab]] entries */
    { AMOVW,	C_LACON,C_NONE,	C_REG,		34, 8, REGSP,	LFROM },
    /*x: [[optab]] entries */
    { AMOVH,	C_SEXT,	C_NONE,	C_REG,		22, 12, REGSB },
    { AMOVH,	C_SAUTO,C_NONE,	C_REG,		22, 12, REGSP },
    { AMOVH,	C_SOREG,C_NONE,	C_REG,		22, 12, 0 },

    { AMOVHU,	C_SEXT,	C_NONE,	C_REG,		22, 12, REGSB },
    { AMOVHU,	C_SAUTO,C_NONE,	C_REG,		22, 12, REGSP },
    { AMOVHU,	C_SOREG,C_NONE,	C_REG,		22, 12, 0 },

    { AMOVB,	C_SEXT,	C_NONE,	C_REG,		22, 12, REGSB },
    { AMOVB,	C_SAUTO,C_NONE,	C_REG,		22, 12, REGSP },
    { AMOVB,	C_SOREG,C_NONE,	C_REG,		22, 12, 0 },
    /*x: [[optab]] entries */
    { AMOVH,	C_LEXT,	C_NONE,	C_REG,		32, 16, REGSB,	LFROM },
    { AMOVH,	C_LAUTO,C_NONE,	C_REG,		32, 16, REGSP,	LFROM },
    { AMOVH,	C_LOREG,C_NONE,	C_REG,		32, 16, 0,	LFROM },

    { AMOVHU,	C_LEXT,	C_NONE,	C_REG,		32, 16, REGSB,	LFROM },
    { AMOVHU,	C_LAUTO,C_NONE,	C_REG,		32, 16, REGSP,	LFROM },
    { AMOVHU,	C_LOREG,C_NONE,	C_REG,		32, 16, 0,	LFROM },

    { AMOVB,	C_LEXT,	C_NONE,	C_REG,		32, 16, REGSB,	LFROM },
    { AMOVB,	C_LAUTO,C_NONE,	C_REG,		32, 16, REGSP,	LFROM },
    { AMOVB,	C_LOREG,C_NONE,	C_REG,		32, 16, 0,	LFROM },
    /*x: [[optab]] entries */
    { AMOVH,	C_REG,	C_NONE,	C_SEXT,		23, 12, REGSB },
    { AMOVH,	C_REG,	C_NONE,	C_SAUTO,	23, 12, REGSP },
    { AMOVH,	C_REG,	C_NONE,	C_SOREG,	23, 12, 0 },

    { AMOVHU,	C_REG,	C_NONE,	C_SEXT,		23, 12, REGSB },
    { AMOVHU,	C_REG,	C_NONE,	C_SAUTO,	23, 12, REGSP },
    { AMOVHU,	C_REG,	C_NONE,	C_SOREG,	23, 12, 0 },
    /*x: [[optab]] entries */
    { AMOVH,	C_REG,	C_NONE,	C_LEXT,		33, 24, REGSB,	LTO },
    { AMOVH,	C_REG,	C_NONE,	C_LAUTO,	33, 24, REGSP,	LTO },
    { AMOVH,	C_REG,	C_NONE,	C_LOREG,	33, 24, 0,	LTO },

    { AMOVHU,	C_REG,	C_NONE,	C_LEXT,		33, 24, REGSB,	LTO },
    { AMOVHU,	C_REG,	C_NONE,	C_LAUTO,	33, 24, REGSP,	LTO },
    { AMOVHU,	C_REG,	C_NONE,	C_LOREG,	33, 24, 0,	LTO },
    /*x: [[optab]] entries */
    { ASWI,		C_NONE,	C_NONE,	C_NONE,		10, 4 },
    { ASWI,		C_NONE,	C_NONE,	C_LCON,		10, 4 },
    { ASWI,		C_NONE,	C_NONE,	C_LOREG,	10, 4 },
    /*x: [[optab]] entries */
    { ARFE,		C_NONE,	C_NONE,	C_NONE,		41, 4 },
    /*x: [[optab]] entries */
    { ATEXT,	C_ADDR,	C_NONE,	C_LCON, 	 0, 0 },
    { ATEXT,	C_ADDR,	C_REG,	C_LCON, 	 0, 0 },
    /*x: [[optab]] entries */
    { AWORD,	C_NONE,	C_NONE,	C_ADDR,		11, 4 },
    /*x: [[optab]] entries */
    { AMOVW,	C_REG,	C_NONE,	C_ADDR,		64, 8, 0,	LTO },
    { AMOVB,	C_REG,	C_NONE,	C_ADDR,		64, 8, 0,	LTO },
    { AMOVBU,	C_REG,	C_NONE,	C_ADDR,		64, 8, 0,	LTO },
    /*x: [[optab]] entries */
    { AMOVW,	C_ADDR,	C_NONE,	C_REG,		65, 8, 0,	LFROM },
    { AMOVBU,	C_ADDR,	C_NONE,	C_REG,		65, 8, 0,	LFROM },
    { AMOVB,	C_ADDR,	C_NONE,	C_REG,		66, 16, 0,	LFROM },
    { AMOVH,	C_ADDR,	C_NONE,	C_REG,		66, 16, 0,	LFROM },
    { AMOVHU,	C_ADDR,	C_NONE,	C_REG,		66, 16, 0,	LFROM },
    /*x: [[optab]] entries */
    { AMOVH,	C_REG,	C_NONE,	C_ADDR,		67, 24, 0,	LTO },
    { AMOVHU,	C_REG,	C_NONE,	C_ADDR,		67, 24, 0,	LTO },
    /*x: [[optab]] entries */
    { AMOVF,	C_FREG,	C_NONE,	C_FEXT,		50, 4, REGSB },
    { AMOVF,	C_FREG,	C_NONE,	C_FAUTO,	50, 4, REGSP },
    { AMOVF,	C_FREG,	C_NONE,	C_FOREG,	50, 4, 0 },

    { AMOVF,	C_FEXT,	C_NONE,	C_FREG,		51, 4, REGSB },
    { AMOVF,	C_FAUTO,C_NONE,	C_FREG,		51, 4, REGSP },
    { AMOVF,	C_FOREG,C_NONE,	C_FREG,		51, 4, 0 },

    { AMOVF,	C_FREG,	C_NONE,	C_LEXT,		52, 12, REGSB,	LTO },
    { AMOVF,	C_FREG,	C_NONE,	C_LAUTO,	52, 12, REGSP,	LTO },
    { AMOVF,	C_FREG,	C_NONE,	C_LOREG,	52, 12, 0,	LTO },

    { AMOVF,	C_LEXT,	C_NONE,	C_FREG,		53, 12, REGSB,	LFROM },
    { AMOVF,	C_LAUTO,C_NONE,	C_FREG,		53, 12, REGSP,	LFROM },
    { AMOVF,	C_LOREG,C_NONE,	C_FREG,		53, 12, 0,	LFROM },

    { AMOVF,	C_FREG,	C_NONE,	C_ADDR,		68, 8, 0,	LTO },
    { AMOVF,	C_ADDR,	C_NONE,	C_FREG,		69, 8, 0,	LFROM },

    { AADDF,	C_FREG,	C_NONE,	C_FREG,		54, 4, 0 },
    { AADDF,	C_FREG,	C_REG,	C_FREG,		54, 4, 0 },
    { AADDF,	C_FCON,	C_NONE,	C_FREG,		54, 4, 0 },
    { AADDF,	C_FCON,	C_REG,	C_FREG,		54, 4, 0 },
    { AMOVF,	C_FCON,	C_NONE,	C_FREG,		54, 4, 0 },
    { AMOVF,	C_FREG, C_NONE, C_FREG,		54, 4, 0 },

    { ACMPF,	C_FREG,	C_REG,	C_NONE,		54, 4, 0 },
    { ACMPF,	C_FCON,	C_REG,	C_NONE,		54, 4, 0 },

    { AMOVFW,	C_FREG,	C_NONE,	C_REG,		55, 4, 0 },
    { AMOVFW,	C_REG,	C_NONE,	C_FREG,		55, 4, 0 },
    /*x: [[optab]] entries */
    { AADDF,	C_FREG,	C_NONE,	C_FREG,		74, 4, 0, VFP },
    { AADDF,	C_FREG,	C_REG,	C_FREG,		74, 4, 0, VFP },
    { AMOVF,	C_FREG, C_NONE, C_FREG,		74, 4, 0, VFP },
    { ACMPF,	C_FREG,	C_REG,	C_NONE,		75, 8, 0, VFP },
    { ACMPF,	C_FCON,	C_REG,	C_NONE,		75, 8, 0, VFP },
    { AMOVFW,	C_FREG,	C_NONE,	C_REG,		76, 8, 0, VFP },
    { AMOVFW,	C_REG,	C_NONE,	C_FREG,		76, 8, 0, VFP },
    /*x: [[optab]] entries */
    { AMOVW,	C_REG,	C_NONE,	C_FCR,		56, 4 },
    { AMOVW,	C_FCR,	C_NONE,	C_REG,		57, 4 },
    /*x: [[optab]] entries */
    { ADIV,		C_REG,	C_REG,	C_REG,		16, 4 },
    { ADIV,		C_REG,	C_NONE,	C_REG,		16, 4 },
    /*x: [[optab]] entries */
    { AMULL,	C_REG,	C_REG,	C_REGREG,	17, 4 },
    /*x: [[optab]] entries */
    { AMOVM,	C_LCON,	C_NONE,	C_SOREG,	38, 4 },
    { AMOVM,	C_SOREG,C_NONE,	C_LCON,		39, 4 },
    /*x: [[optab]] entries */
    { AMOVW,	C_PSR,	C_NONE,	C_REG,		35, 4 },
    { AMOVW,	C_REG,	C_NONE,	C_PSR,		36, 4 },
    { AMOVW,	C_RCON,	C_NONE,	C_PSR,		37, 4 },
    /*x: [[optab]] entries */
    { AMOVH,	C_REG,	C_NONE,	C_HEXT,		70, 4, REGSB,	V4 },
    { AMOVH,	C_REG,	C_NONE, C_HAUTO,	70, 4, REGSP,	V4 },
    { AMOVH,	C_REG,	C_NONE,	C_HOREG,	70, 4, 0,		V4 },

    { AMOVHU,	C_REG,	C_NONE,	C_HEXT,		70, 4, REGSB,	V4 },
    { AMOVHU,	C_REG,	C_NONE, C_HAUTO,	70, 4, REGSP,	V4 },
    { AMOVHU,	C_REG,	C_NONE,	C_HOREG,	70, 4, 0,		V4 },


    { AMOVB,	C_HEXT,	C_NONE, C_REG,		71, 4, REGSB,	V4 },
    { AMOVB,	C_HAUTO,C_NONE,	C_REG,		71, 4, REGSP,	V4 },
    { AMOVB,	C_HOREG,C_NONE,	C_REG,		71, 4, 0,		V4 },

    { AMOVH,	C_HEXT,	C_NONE,	C_REG,		71, 4, REGSB,	V4 },
    { AMOVH,	C_HAUTO,C_NONE, C_REG,		71, 4, REGSP,	V4 },
    { AMOVH,	C_HOREG,C_NONE,	C_REG,		71, 4, 0,		V4 },

    { AMOVHU,	C_HEXT,	C_NONE,	C_REG,		71, 4, REGSB,	V4 },
    { AMOVHU,	C_HAUTO,C_NONE, C_REG,		71, 4, REGSP,	V4 },
    { AMOVHU,	C_HOREG,C_NONE,	C_REG,		71, 4, 0,		V4 },


    { AMOVH,	C_REG,	C_NONE,	C_LEXT,		72, 8, REGSB,	LTO|V4 },
    { AMOVH,	C_REG,	C_NONE, C_LAUTO,	72, 8, REGSP,	LTO|V4 },
    { AMOVH,	C_REG,	C_NONE,	C_LOREG,	72, 8, 0,		LTO|V4 },

    { AMOVHU,	C_REG,	C_NONE,	C_LEXT,		72, 8, REGSB,	LTO|V4 },
    { AMOVHU,	C_REG,	C_NONE, C_LAUTO,	72, 8, REGSP,	LTO|V4 },
    { AMOVHU,	C_REG,	C_NONE,	C_LOREG,	72, 8, 0,		LTO|V4 },


    { AMOVB,	C_LEXT,	C_NONE, C_REG,		73, 8, REGSB,	LFROM|V4 },
    { AMOVB,	C_LAUTO,C_NONE,	C_REG,		73, 8, REGSP,	LFROM|V4 },
    { AMOVB,	C_LOREG,C_NONE,	C_REG,		73, 8, 0,		LFROM|V4 },

    { AMOVH,	C_LEXT,	C_NONE,	C_REG,		73, 8, REGSB,	LFROM|V4 },
    { AMOVH,	C_LAUTO,C_NONE, C_REG,		73, 8, REGSP,	LFROM|V4 },
    { AMOVH,	C_LOREG,C_NONE,	C_REG,		73, 8, 0,		LFROM|V4 },

    { AMOVHU,	C_LEXT,	C_NONE,	C_REG,		73, 8, REGSB,	LFROM|V4 },
    { AMOVHU,	C_LAUTO,C_NONE, C_REG,		73, 8, REGSP,	LFROM|V4 },
    { AMOVHU,	C_LOREG,C_NONE,	C_REG,		73, 8, 0,		LFROM|V4 },
    /*x: [[optab]] entries */
    { AMOVW,	C_SHIFT,C_NONE,	C_REG,		59, 4 },
    { AMOVBU,	C_SHIFT,C_NONE,	C_REG,		59, 4 },

    { AMOVB,	C_SHIFT,C_NONE,	C_REG,		60, 4 },

    { AMOVW,	C_REG,	C_NONE,	C_SHIFT,	61, 4 },
    { AMOVB,	C_REG,	C_NONE,	C_SHIFT,	61, 4 },
    { AMOVBU,	C_REG,	C_NONE,	C_SHIFT,	61, 4 },
    /*x: [[optab]] entries */
    { ACASE,	C_REG,	C_NONE,	C_NONE,		62, 4 },
    { ABCASE,	C_NONE, C_NONE, C_BRANCH,   63, 4 },
    /*e: [[optab]] entries */
    { AXXX,		C_NONE,	C_NONE,	C_NONE,		 0, 4 },
};
/*e: global optab (linkers/5l/optab.c)(arm) */
/*e: linkers/5l/optab.c */
