/*s: assemblers/aa/lexbody.c */
#include "aa.h"

/*
 * common code for all the assemblers
 */

/*s: function setinclude */
void
setinclude(char *p)
{
    int i;

    if(p == nil)
        return;
    for(i=1; i < ninclude; i++)
        if(strcmp(p, include[i]) == 0)
            return;

    if(ninclude >= nelem(include)) {
        yyerror("ninclude too small %d", nelem(include));
        exits("ninclude");
    }
    include[ninclude++] = p;
}
/*e: function setinclude */

/*s: function pushio */
void
pushio(void)
{
    Io *i;

    i = iostack;
    if(i == I) {
        yyerror("botch in pushio");
        errorexit();
    }
    // save current position in includer
    i->p = fi.p;
    i->c = fi.c;
}
/*e: function pushio */

/*s: function newio */
/// main -> assemble -> pinit -> <>; newfile
void
newio(void)
{
    Io *i;

    /*s: [[newio()]] allocate a new Io in [[i]] or find a free one */
    static int pushdepth = 0;

    i = iofree;
    if(i == I) {
        pushdepth++;
        if(pushdepth > 1000) {
            yyerror("macro/io expansion too deep");
            errorexit();
        }
        //todo: check error code?
        i = alloc(sizeof(Io));
    } else
        iofree = i->link;
    /*e: [[newio()]] allocate a new Io in [[i]] or find a free one */
    ionext = i;
    i->f = FD_NONE;
    i->c = 0;
}
/*e: function newio */

/*s: function newfile */
/// main -> assemble -> pinit -> { newio; <> }
/// yylex -> macinc -> <>
void
newfile(char *s, fdt f)
{
    Io *i;

    // add_list(ionext, iostack)
    i = ionext;
    i->link = iostack;
    iostack = i;

    i->f = f;
    if(i->f == FD_NONE)
        i->f = open(s, 0);
    if(i->f < 0) {
        yyerror("%ca: %r: %s", thechar, s);
        errorexit();
    }
    fi.c = 0;
    /*s: [[newfile()]] call linehist */
    linehist(s, 0);
    /*e: [[newfile()]] call linehist */
}
/*e: function newfile */

// this was hard to factorize in aa/, so this is copy pasted
// in each assembler (8a/, va/, etc)
//long
//yylex(void)
//{
//	int c, c1;
//	char *cp;
//	Sym *s;
//
//	c = peekc;
//	if(c != IGN) {
//		peekc = IGN;
//		goto l1;
//	}
//l0:
//	c = GETC();
//
//l1:
//	if(c == EOF) {
//		peekc = EOF;
//		return -1;
//	}
//	if(isspace(c)) {
//		if(c == '\n') {
//			lineno++;
//			return ';';
//		}
//		goto l0;
//	}
//	if(isalpha(c))
//		goto talph;
//	if(isdigit(c))
//		goto tnum;
//	switch(c)
//	{
//	case '\n':
//		lineno++;
//		return ';';
//
//	case '#':
//		domacro();
//		goto l0;
//
//	case '.':
//		c = GETC();
//		if(isalpha(c)) {
//			cp = symb;
//			*cp++ = '.';
//			goto aloop;
//		}
//		if(isdigit(c)) {
//			cp = symb;
//			*cp++ = '.';
//			goto casedot;
//		}
//		peekc = c;
//		return '.';
//
//	talph:
//	case '_':
//	case '@':
//		cp = symb;
//
//	aloop:
//		*cp++ = c;
//		c = GETC();
//		if(isalpha(c) || isdigit(c) || c == '_' || c == '$')
//			goto aloop;
//		*cp = 0;
//		peekc = c;
//		s = lookup();
//		if(s->macro) {
//			newio();
//			cp = ionext->b;
//			macexpand(s, cp);
//			pushio();
//			ionext->link = iostack;
//			iostack = ionext;
//			fi.p = cp;
//			fi.c = strlen(cp);
//			if(peekc != IGN) {
//				cp[fi.c++] = peekc;
//				cp[fi.c] = 0;
//				peekc = IGN;
//			}
//			goto l0;
//		}
//		if(s->type == 0)
//			s->type = LNAME;
//		if(s->type == LNAME ||
//		   s->type == LVAR ||
//		   s->type == LLAB) {
//			yylval.sym = s;
//			return s->type;
//		}
//		yylval.lval = s->value;
//		return s->type;
//
//	tnum:
//		cp = symb;
//		if(c != '0')
//			goto dc;
//		*cp++ = c;
//		c = GETC();
//		c1 = 3;
//		if(c == 'x' || c == 'X') {
//			c1 = 4;
//			c = GETC();
//		} else
//		if(c < '0' || c > '7')
//			goto dc;
//		yylval.lval = 0;
//		for(;;) {
//			if(c >= '0' && c <= '9') {
//				if(c > '7' && c1 == 3)
//					break;
//				yylval.lval <<= c1;
//				yylval.lval += c - '0';
//				c = GETC();
//				continue;
//			}
//			if(c1 == 3)
//				break;
//			if(c >= 'A' && c <= 'F')
//				c += 'a' - 'A';
//			if(c >= 'a' && c <= 'f') {
//				yylval.lval <<= c1;
//				yylval.lval += c - 'a' + 10;
//				c = GETC();
//				continue;
//			}
//			break;
//		}
//		goto ncu;
//
//	dc:
//		for(;;) {
//			if(!isdigit(c))
//				break;
//			*cp++ = c;
//			c = GETC();
//		}
//		if(c == '.')
//			goto casedot;
//		if(c == 'e' || c == 'E')
//			goto casee;
//		*cp = 0;
//		if(sizeof(yylval.lval) == sizeof(vlong))
//			yylval.lval = strtoll(symb, nil, 10);
//		else
//			yylval.lval = strtol(symb, nil, 10);
//
//	ncu:
//		while(c == 'U' || c == 'u' || c == 'l' || c == 'L')
//			c = GETC();
//		peekc = c;
//		return LCONST;
//
//	casedot:
//		for(;;) {
//			*cp++ = c;
//			c = GETC();
//			if(!isdigit(c))
//				break;
//		}
//		if(c == 'e' || c == 'E')
//			goto casee;
//		goto caseout;
//
//	casee:
//		*cp++ = 'e';
//		c = GETC();
//		if(c == '+' || c == '-') {
//			*cp++ = c;
//			c = GETC();
//		}
//		while(isdigit(c)) {
//			*cp++ = c;
//			c = GETC();
//		}
//
//	caseout:
//		*cp = 0;
//		peekc = c;
//		if(FPCHIP) {
//			yylval.dval = atof(symb);
//			return LFCONST;
//		}
//		yyerror("assembler cannot interpret fp constants");
//		yylval.lval = 1L;
//		return LCONST;
//
//	case '"':
//		memcpy(yylval.sval, nullgen.sval, sizeof(yylval.sval));
//		cp = yylval.sval;
//		c1 = 0;
//		for(;;) {
//			c = escchar('"');
//			if(c == EOF)
//				break;
//			if(c1 < sizeof(yylval.sval))
//				*cp++ = c;
//			c1++;
//		}
//		if(c1 > sizeof(yylval.sval))
//			yyerror("string constant too long");
//		return LSCONST;
//
//	case '\'':
//		c = escchar('\'');
//		if(c == EOF)
//			c = '\'';
//		if(escchar('\'') != EOF)
//			yyerror("missing '");
//		yylval.lval = c;
//		return LCONST;
//
//	case '/':
//		c1 = GETC();
//		if(c1 == '/') {
//			for(;;) {
//				c = GETC();
//				if(c == '\n')
//					goto l1;
//				if(c == EOF) {
//					yyerror("eof in comment");
//					errorexit();
//				}
//			}
//		}
//		if(c1 == '*') {
//			for(;;) {
//				c = GETC();
//				while(c == '*') {
//					c = GETC();
//					if(c == '/')
//						goto l0;
//				}
//				if(c == EOF) {
//					yyerror("eof in comment");
//					errorexit();
//				}
//				if(c == '\n')
//					lineno++;
//			}
//		}
//		break;
//
//	default:
//		return c;
//	}
//	peekc = c1;
//	return c;
//}



/*s: function getc */
int
getc(void)
{
    int c;

    c = peekc;
    if(c != IGN) {
        peekc = IGN;
        return c;
    }

    c = GETC();

    if(c == '\n')
        lineno++;
    if(c == EOF) {
        yyerror("End of file");
        errorexit();
    }
    return c;
}
/*e: function getc */

/*s: function getnsc */
int
getnsc(void)
{
    int c;

    for(;;) {
        c = getc();
        if(!isspace(c) || c == '\n')
            return c;
    }
}
/*e: function getnsc */

/*s: function unget */
void
unget(int c)
{
    peekc = c;
    if(c == '\n')
        lineno--;
}
/*e: function unget */

/*s: function escchar */
int
escchar(int e)
{
    int c, l;

loop:
    c = getc(); // not GETC
    if(c == '\n') {
        yyerror("newline in string");
        return EOF;
    }
    if(c != '\\') {
        if(c == e)
            return EOF;
        return c;
    }
    // else c is '\\'
    c = getc();
    /*s: [[escchar()]] if octal character */
    if(c >= '0' && c <= '7') {
        l = c - '0';
        c = getc();
        if(c >= '0' && c <= '7') {
            l = l*8 + c-'0';
            c = getc();
            if(c >= '0' && c <= '7') {
                l = l*8 + c-'0';
                return l;
            }
        }
        peekc = c;
        return l;
    }
    /*e: [[escchar()]] if octal character */
    switch(c)
    {
    case '\n':	goto loop; // multi line strings

    case 'n':	return '\n';
    case 't':	return '\t';
    case 'b':	return '\b';
    case 'r':	return '\r';
    case 'f':	return '\f';

    /*s: [[escchar()]] switch cases */
    case 'a':	return 0x07;
    case 'v':	return 0x0b;
    case 'z':	return 0x00;
    /*e: [[escchar()]] switch cases */
    }
    return c;
}
/*e: function escchar */

/*s: function pinit */
/// main -> assemble -> { <> ; yyparse } x 2
void
pinit(char *f)
{
    /*s: [[pinit()]] locals */
    int i;
    Sym *s;
    /*e: [[pinit()]] locals */

    lineno = 1;

    newio(); // set ionext
    newfile(f, FD_NONE); // use ionext, set iostack, set fi

    /*s: [[pinit()]] initializations */
    peekc = IGN;
    /*x: [[pinit()]] initializations */
    for(i=0; i<NHASH; i++)
        for(s = hash[i]; s != S; s = s->link)
            s->macro = nil;
    /*x: [[pinit()]] initializations */
    pc = 0;
    /*x: [[pinit()]] initializations */
    symcounter = 1;
    for(i=0; i<NSYM; i++) {
        h[i].symkind = 0; // N_NONE
        h[i].sym = S;
    }
    /*e: [[pinit()]] initializations */
}
/*e: function pinit */

/*s: function filbuf */
int
filbuf(void)
{
    Io *i;

loop:
    i = iostack;
    if(i == I)
        return EOF;
    if(i->f < 0) // When this happens?
        goto pop; 

    // system call! fill really the buffer
    fi.c = read(i->f, i->b, BUFSIZ) - 1;
    if(fi.c < 0) {
        close(i->f);
        /*s: [[filbuf()]] when close file, call linehist */
        linehist(nil, 0);
        /*e: [[filbuf()]] when close file, call linehist */
        goto pop;
    }
    fi.p = i->b + 1;
    return i->b[0];

/*s: [[filbuf()]] pop */
pop:
    // pop(iostack)
    iostack = i->link;
    // push(i, iofree)
    i->link = iofree;
    iofree = i;

    // i = top(iostack), the fresh top of the stack input file
    i = iostack;
    if(i == I)
        return EOF;
    // restore file pointers
    fi.p = i->p;
    fi.c = i->c;
    if(--fi.c < 0)
        goto loop;
    return *fi.p++;
/*e: [[filbuf()]] pop */
}
/*e: function filbuf */

/*e: assemblers/aa/lexbody.c */
