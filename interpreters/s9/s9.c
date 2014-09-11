/*
 * Scheme 9 from Empty Space
 * Copyright (C) 2007 Nils M Holm <nmh@t3x.org>
 */

/*
 * Use -DNO_SIGNALS to disable POSIX signal handlers.
 */

/*----- Miscellanea -----*/

int	Debug_GC = 0;

#define VERSION	"2007-07-06 (with errata of 2008)"

/*
 * Tell later MSC compilers to let us use the standard CLIB API.
 * Blake McBride <blake@mcbride.name>
 */
#ifdef _MSC_VER
 #if _MSC_VER > 1200
  #ifndef _CRT_SECURE_NO_DEPRECATE
   #define _CRT_SECURE_NO_DEPRECATE
  #endif
 #endif
 #ifndef _POSIX_
  #define _POSIX_
 #endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#ifdef NO_SIGNALS
 #define signal(sig, fn)
#else
 #include <signal.h>
 #ifdef _MSC_VER
  #ifndef SIGQUIT
   #define SIGQUIT SIGINT
  #endif
 #endif
#endif

#define TEXT_LEN	1024
#define MAX_PORTS	32
#define SEGMENT_LEN	32768
#define HASH_THRESHOLD	64

/* Hard memory limit in K-Nodes, 0 = none */
#define MEMORY_LIMIT_KN	1024

#if INT_MAX >= 1000000000000000000		/* 64-bit */
 #define DIGITS_PER_WORD	18
 #define INT_SEG_LIMIT		1000000000000000000
#elif INT_MAX >= 1000000000 			/* 32-bit */
 #define DIGITS_PER_WORD	9
 #define INT_SEG_LIMIT		1000000000
#elif INT_MAX >= 10000				/* 16-bit */
 #define DIGITS_PER_WORD	4
 #define INT_SEG_LIMIT		10000
#endif

/* GC flags */
#define	AFLAG	0x01	/* Atom, Car = type, CDR = next */
#define	MFLAG	0x02	/* Mark */
#define SFLAG	0x04	/* State */
#define VFLAG	0x08	/* Vector, Car = type, CDR = content */
#define UFLAG	0x10	/* Port: used flag */
#define LFLAG	0x20	/* Port: locked (do not close) */

enum EVAL_STATES {
	MATOM,	/* Processing atom */
	MARGS,	/* Processing argument list */
	MBETA,	/* Beta-reducing */
	MIFPR,	/* Processing predicate of IF */
	MSETV,	/* Processing value of SET! */
	MDEFN,	/* Processing value of DEFINE */
	MDSYN,	/* Processing value of DEFINE-SYNTAX */
	MBEGN,	/* Processing BEGIN */
	MCONJ,	/* Processing arguments of AND */
	MDISJ, 	/* Processing arguments of OR */
	MCOND	/* Processing clauses of COND */
};

#define special_value_p(x)	((x) < 0)
#define NIL		(-1)
#define TRUE		(-2)
#define FALSE		(-3)
#define ENDOFFILE	(-4)
#define UNDEFINED	(-5)
#define UNSPECIFIC	(-6)
#define	DOT		(-7)
#define	RPAREN		(-8)
#define NOEXPR		(-9)

int	Pool_size = 0,
	Vpool_size = 0;
int	*Car = NULL,
	*Cdr = NULL;
char	*Tag = NULL;
int	*Vectors = NULL;
int	Free_vecs = 0;
int	Stack = NIL,
	Stack_bottom = NIL;
int	State_stack = NIL;
int	Tmp_car = NIL,
	Tmp_cdr = NIL,
	Tmp = NIL;
int	Free_list = NIL;
int	Symbols = NIL;
int	Program = NIL;
int	Environment = NIL;
int	Acc = NIL;
FILE	*Ports[MAX_PORTS];
char	Port_flags[MAX_PORTS];
int	Input_port = 0,
	Output_port = 1;
int	Level = 0;
int	Error_flag = 0;
int	Load_level = 0;
int	Displaying = 0;
int	Quiet_mode = 0;

int	S_char, S_else, S_input_port, S_integer, S_latest,
	S_output_port, S_primitive, S_procedure, S_quasiquote,
	S_quote, S_string, S_symbol, S_syntax, S_unquote,
	S_unquote_splicing, S_vector;
int	S_and, S_begin, S_cond, S_define, S_define_syntax, S_if,
	S_lambda, S_let, S_letrec, S_or, S_quote, S_set_b,
	S_syntax_rules;

int	*GC_root[] = { &Program, &Symbols, &Environment, &Tmp, &Tmp_car,
			&Tmp_cdr, &Stack, &Stack_bottom, &State_stack,
			&Acc, NULL };

#define nl()		pr("\n")
#define reject(c)	ungetc(c, Ports[Input_port])
#define string(n)	((char *) &Vectors[Cdr[n]])
#define string_len(n)	(Vectors[Cdr[n] - 1])
#define vector_size(k)	(((k) + sizeof(int)-1) / sizeof(int) + 2)
#define vector(n)	(&Vectors[Cdr[n]])
#define vector_len(n)	(vector_size(string_len(n)) - 2)
#define port_no(n)	(cadr(n))
#define char_value(n)	(cadr(n))

#define caar(x)		(Car[Car[x]])
#define cadr(x)		(Car[Cdr[x]])
#define cdar(x)		(Cdr[Car[x]])
#define cddr(x)		(Cdr[Cdr[x]])
#define caaar(x)	(Car[Car[Car[x]]])
#define caadr(x)	(Car[Car[Cdr[x]]])
#define cadar(x)	(Car[Cdr[Car[x]]])
#define caddr(x)	(Car[Cdr[Cdr[x]]])
#define cdadr(x)	(Cdr[Car[Cdr[x]]])
#define cddar(x)	(Cdr[Cdr[Car[x]]])
#define cdddr(x)	(Cdr[Cdr[Cdr[x]]])
#define caaddr(x)	(Car[Car[Cdr[Cdr[x]]]])
#define caddar(x)	(Car[Cdr[Cdr[Car[x]]]])
#define cadadr(x)	(Car[Cdr[Car[Cdr[x]]]])
#define cadddr(x)	(Car[Cdr[Cdr[Cdr[x]]]])
#define cdddar(x)	(Cdr[Cdr[Cdr[Car[x]]]])
#define cddddr(x)	(Cdr[Cdr[Cdr[Cdr[x]]]])

#define null_p(n)	((n) == NIL)
#define eof_p(n)	((n) == ENDOFFILE)
#define undefined_p(n)	((n) == UNDEFINED)
#define unspecific_p(n)	((n) == UNSPECIFIC)

#define boolean_p(n)	((n) == TRUE || (n) == FALSE)

#define integer_p(n) \
	(!special_value_p(n) && (Tag[n] & AFLAG) && Car[n] == S_integer)
#define primitive_p(n) \
	(!special_value_p(n) && (Tag[n] & AFLAG) && Car[n] == S_primitive)
#define procedure_p(n) \
	(!special_value_p(n) && (Tag[n] & AFLAG) && Car[n] == S_procedure)
#define special_p(n)	((n) == S_and    || \
			 (n) == S_begin  || \
			 (n) == S_cond   || \
			 (n) == S_define || \
			 (n) == S_define_syntax || \
			 (n) == S_if     || \
			 (n) == S_lambda || \
			 (n) == S_let    || \
			 (n) == S_letrec || \
			 (n) == S_or     || \
			 (n) == S_quote  || \
			 (n) == S_syntax_rules  || \
			 (n) == S_set_b)
#define char_p(n) \
	(!special_value_p(n) && (Tag[n] & AFLAG) && Car[n] == S_char)
#define syntax_p(n) \
	(!special_value_p(n) && (Tag[n] & AFLAG) && Car[n] == S_syntax)
#define input_port_p(n)	\
	(!special_value_p(n) && (Tag[n] & AFLAG) && Car[n] == S_input_port)
#define output_port_p(n) \
	(!special_value_p(n) && (Tag[n] & AFLAG) && Car[n] == S_output_port)

#define symbol_p(n) \
	(!special_value_p(n) && (Tag[n] & VFLAG) && Car[n] == S_symbol)
#define vector_p(n) \
	(!special_value_p(n) && (Tag[n] & VFLAG) && Car[n] == S_vector)
#define string_p(n) \
	(!special_value_p(n) && (Tag[n] & VFLAG) && Car[n] == S_string)

#define auto_quoting_p(n) \
	(boolean_p(n)    || \
	 char_p(n)       || \
	 eof_p(n)        || \
	 integer_p(n)    || \
	 string_p(n)     || \
	 undefined_p(n)  || \
	 unspecific_p(n) || \
	 vector_p(n))

#define rib_args(x)	(Car[x])
#define rib_append(x)	(cadr(x))
#define rib_result(x)	(caddr(x))
#define rib_source(x)	(cadddr(x))

#define atom_p(n) \
	(special_value_p(n) || (Tag[n] & AFLAG) || (Tag[n] & VFLAG))

#define pair_p(x) (!atom_p(x))

int error(char *msg, int expr);

void pr(char *s) {
	if (Ports[Output_port] == NULL)
		error("output port is not open", NOEXPR);
	else
		fwrite(s, 1, strlen(s), Ports[Output_port]);
}

void print(int n);

int error(char *msg, int expr) {
	int	oport;

	if (Error_flag) return UNSPECIFIC;
	oport = Output_port;
	Output_port = 1;
	Error_flag = 1;
	printf("error: %s", msg);
	if (expr != NOEXPR) {
		pr(": ");
		print(expr);
	}
	nl();
	Output_port = oport;
	if (Quiet_mode) exit(1);
	return UNSPECIFIC;
}

int fatal(char *msg) {
	printf("fatal ");
	error(msg, NOEXPR);
	exit(2);
}

/*----- GC -----*/

void new_segment(void) {
	Car = realloc(Car, sizeof(int) * (Pool_size + SEGMENT_LEN));
	Cdr = realloc(Cdr, sizeof(int) * (Pool_size + SEGMENT_LEN));
	Tag = realloc(Tag, Pool_size + SEGMENT_LEN);
	Vectors = realloc(Vectors, sizeof(int) * (Vpool_size + SEGMENT_LEN));
	if (	Car == NULL || Cdr == NULL || Tag == NULL ||
		Vectors == NULL
	) {
		fatal("out of memory");
	}
	memset(&Car[Pool_size], 0, SEGMENT_LEN * sizeof(int));
	memset(&Cdr[Pool_size], 0, SEGMENT_LEN * sizeof(int));
	memset(&Tag[Pool_size], 0, SEGMENT_LEN);
	memset(&Vectors[Pool_size], 0, SEGMENT_LEN * sizeof(int));
	Pool_size += SEGMENT_LEN;
	Vpool_size += SEGMENT_LEN;
}

void mark(int n);

/* Mark object at offset N in the Vectors */
void mark_vector(int n, int type) {
	int	*p, k;

	p = &Vectors[Cdr[n] - 2];
	*p = n;
	if (type == S_vector) {
		k = vector_len(n);
		p = vector(n);
		while (k) {
			mark(*p);
			p++;
			k--;
		}
	}
}

/*
 * Mark nodes which can be accessed through N.
 * Using the Deutsch/Schorr/Waite (aka pointer reversal) algorithm.
 * S0: M==0 S==0 unvisited, process CAR
 * S1: M==1 S==1 CAR visited, process CDR
 * S2: M==1 S==0 completely visited, return to parent
 */
void mark(int n) {
	int	p, parent;

	parent = NIL;	/* Initially, there is no parent node */
	while (1) {
		if (special_value_p(n) || Tag[n] & MFLAG) {
			if (parent == NIL) break;
			if (Tag[parent] & SFLAG) {	/* S1 --> S2 */
				p = Cdr[parent];
				Cdr[parent] = Car[parent];
				Car[parent] = n;
				Tag[parent] &= ~SFLAG;
				Tag[parent] |=  MFLAG;
				n = p;
			}
			else {				/* S2 --> done */
				p = parent;
				parent = Cdr[p];
				Cdr[p] = n;
				n = p;
			}
		}
		else {	
			if (Tag[n] & VFLAG) {		/* S0 --> done */
				Tag[n] |= MFLAG;
				/* Tag[n] &= ~SFLAG; */
				mark_vector(n, Car[n]);
			}
			else if (Tag[n] & AFLAG) {	/* S0 --> S2 */
				if (input_port_p(n) || output_port_p(n))
					Port_flags[port_no(n)] |= UFLAG;
				p = Cdr[n];
				Cdr[n] = parent;
				/*Tag[n] &= ~SFLAG;*/
				parent = n;
				n = p;
				Tag[parent] |= MFLAG;
			}
			else {				/* S0 --> S1 */
				p = Car[n];
				Car[n] = parent;
				Tag[n] |= MFLAG;
				parent = n;
				n = p;
				Tag[parent] |= SFLAG;
			}
		}
	}
}

/* Mark all vectors unused */
void unmark_vectors(void) {
	int	p, k, link;

	p = 0;
	while (p < Free_vecs) {
		link = p;
		k = Vectors[p+1];
		p += vector_size(k);
		Vectors[link] = NIL;
	}
}

/* Mark and Sweep GC. */
int gc(void) {
	int	i, k;

	if (Debug_GC) pr("GC called: ");
	for (i=0; i<MAX_PORTS; i++)
		if (Port_flags[i] & LFLAG)
			Port_flags[i] |= UFLAG;
		else
			Port_flags[i] &= ~UFLAG;
	for (i=0; GC_root[i] != NULL; i++) mark(GC_root[i][0]);
	k = 0;
	Free_list = NIL;
	for (i=0; i<Pool_size; i++) {
		if (!(Tag[i] & MFLAG)) {
			Cdr[i] = Free_list;
			Free_list = i;
			k = k+1;
		}
		else {
			Tag[i] &= ~MFLAG;
		}
	}
	for (i=0; i<MAX_PORTS; i++) {
		if (!(Port_flags[i] & UFLAG) && Ports[i] != NULL) {
			fclose(Ports[i]);
			Ports[i] = NULL;
		}
	}
	if (Debug_GC) printf("%d nodes reclaimed\n", k);
	return k;
}

/* Allocate a fresh node and initialize with PCAR,PCDR,PTAG. */
int alloc3(int pcar, int pcdr, int ptag) {
	int	n, k;

	if (Free_list == NIL) {
		if (ptag == 0) Tmp_car = pcar;
		Tmp_cdr = pcdr;
		k = gc();
		/*
		 * Performance increases dramatically if we
		 * do not wait for the pool to run dry.
		 */
		if (k < Pool_size / 10) {
			if (	MEMORY_LIMIT_KN &&
				Pool_size + SEGMENT_LEN > MEMORY_LIMIT_KN*1024
			) {
				if (Free_list == NIL)
					fatal("alloc3(): hit memory limit");
			}
			else {
				new_segment();
				gc();
				if (Debug_GC)
					printf("Alloc3: new segment,"
					       " nodes = %d\n",
							Pool_size);
			}
		}
		Tmp_car = Tmp_cdr = NIL;
	}
	n = Free_list;
	Free_list = Cdr[Free_list];
	Car[n] = pcar;
	Cdr[n] = pcdr;
	Tag[n] = ptag;
	return n;
}

#define alloc(pcar, pcdr) alloc3((pcar), (pcdr), 0)

/* In situ vector pool garbage collection and compaction */
int gcv(void) {
	int	v, k, to, from;

	unmark_vectors();
	gc();		/* re-mark live vectors */
	if (Debug_GC) printf("GCV called: ");
	to = from = 0;
	while (from < Free_vecs) {
		v = Vectors[from+1];
		k = vector_size(v);
		if (Vectors[from] != NIL) {
			if (to != from) {
				memmove(&Vectors[to], &Vectors[from],
					k * sizeof(int));
				Cdr[Vectors[to]] = to + 2;
			}
			to += k;
		}
		from += k;
	}
	k = Free_vecs - to;
	if (Debug_GC) printf("%d cells reclaimed\n", k);
	Free_vecs = to;
	return k;
}

/* Allocate vector from pool */
int allocv(int type, int size) {
	int	v, n, wsize, k;

	wsize = vector_size(size);
	if (Free_vecs + wsize >= Vpool_size) {
		k = gcv();
		if (	Free_vecs + wsize >=
			Vpool_size - Vpool_size / 2
		) {
			if (	MEMORY_LIMIT_KN &&
				Pool_size + SEGMENT_LEN > MEMORY_LIMIT_KN*1024
			) {
				if (Free_list == NIL)
					fatal("allocv(): hit memory limit");
			}
			else {
				new_segment();
				gcv();
				if (Debug_GC)
					printf("Allocv: new segment,"
					    	" nodes = %d\n",
							Pool_size);
			}
		}
	}
	v = Free_vecs;
	Free_vecs += wsize;
	n = alloc3(type, v+2, VFLAG);
	Vectors[v] = n;
	Vectors[v+1] = size;
	return n;
}

#define save(n) (Stack = alloc((n), Stack))

/* Pop K nodes off the Stack, return last one. */
int unsave(int k) {
	int	n = NIL; /*LINT*/

	while (k) {
		if (Stack == NIL) fatal("unsave(): stack underflow");
		n = Car[Stack];
		Stack = Cdr[Stack];
		k = k-1;
	}
	return n;
}

/*----- Reader -----*/

int find_symbol(char *s) {
	int	y;

	y = Symbols;
	while (y != NIL) {
		if (!strcmp(string(Car[y]), s))
			return Car[y];
		y = Cdr[y];
	}
	return NIL;
}

int make_symbol(char *s, int k) {
	int	n;

	n = allocv(S_symbol, k+1);
	strcpy(string(n), s);
	return n;
}

int add_symbol(char *s) {
	int	y, new;

	y = find_symbol(s);
	if (y != NIL) return y;
	Symbols = alloc(NIL, Symbols);
	new = make_symbol(s, strlen(s));
	Car[Symbols] = new;
	return Car[Symbols];
}

#define read_c() getc(Ports[Input_port])

#define read_c_ci() tolower(read_c())

int read_form(void);

int read_list(void) {
	int	n,	/* Node read */
		m,	/* List */
		a,	/* Used to append nodes to m */
		c;	/* Member counter */
	int	new;
	char	*badpair;

	badpair = "bad pair";
	Level = Level+1;
	m = alloc(NIL, NIL);	/* root */
	save(m);
	a = NIL;
	c = 0;
	while (1) {
		if (Error_flag) {
			unsave(1);
			return NIL;
		}
		n = read_form();
		if (n == ENDOFFILE)  {
			if (Load_level) {
				unsave(1);
				return ENDOFFILE;
			}
			error("missing ')'", NOEXPR);
		}
		if (n == DOT) {
			if (c < 1) {
				error(badpair, NOEXPR);
				continue;
			}
			n = read_form();
			Cdr[a] = n;
			if (n == RPAREN || read_form() != RPAREN) {
				error(badpair, NOEXPR);
				continue;
			}
			unsave(1);
			Level = Level-1;
			return m;
		}
		if (n == RPAREN) break;
		if (a == NIL) 
			a = m;		/* First member: insert at root */
		else
			a = Cdr[a];	/* Following members: append */
		Car[a] = n;
		new = alloc(NIL, NIL); /* Alloc space for next member */
		Cdr[a] = new;
		c = c+1;
	}
	Level = Level-1;
	if (a != NIL) Cdr[a] = NIL;	/* Remove trailing empty node */
	unsave(1);
	return c? m: NIL;
}

int quote(int n, int quotation) {
	int	q;

	q = alloc(n, NIL);
	return alloc(quotation, q);
}

int str_numeric_p(char *s) {
	int	i;

	i = 0;
	if (s[0] == '+' || s[0] == '-') i = 1;
	if (!s[i]) return 0;
	while (s[i]) {
		if (!isdigit(s[i])) return 0;
		i = i+1;
	}
	return 1;
}

int string_to_bignum(char *s) {
	int	k, j, n, v, sign;

	sign = 1;
	if (s[0] == '-') {
		s++;
		sign = -1;
	}
	else if (s[0] == '+') {
		s++;
	}
	k = strlen(s);
	n = NIL;
	while (k) {
		j = k <= DIGITS_PER_WORD? k: DIGITS_PER_WORD;
		v = atol(&s[k-j]);
		s[k-j] = 0;
		k -= j;
		if (k == 0) v *= sign;
		n = alloc3(v, n, AFLAG);
	}
	return alloc3(S_integer, n, AFLAG);
}

/* Create a character literal. */
int make_char(int x) {
	int	n;

	n = alloc3(x, NIL, AFLAG);
	return alloc3(S_char, n, AFLAG);
}

/* Read a character literal. */
int character(void) {
	char	buf[10];
	int	i, c;

	for (i=0; i<9; i++) {
		c = read_c();
		if (i > 0 && !isalpha(c)) break;
		buf[i] = c;
	}
	reject(c);
	buf[i] = 0;
	if (i == 0) c = ' ';
	else if (i == 1) c = buf[0];
	else if (!strcmp(buf, "space")) c = ' ';
	else if (!strcmp(buf, "newline")) c = '\n';
	else if (!strcmp(buf, "linefeed")) c = '\n';
	else {
		error("bad # syntax", NOEXPR);
		c = 0;
	}
	return make_char(c);
}

/* Create a string; K = length */
int make_string(char *s, int k) {
	int	n;

	n = allocv(S_string, k+1);
	strcpy(string(n), s);
	return n;
}

/* Clone an existing string.
 * When copying a string to a string, the source string
 * may get relocated during vector pool compaction.
 * Hence this routine must be used instead of make_string().
 */
int clone_string(int s, int k) {
	int	n;

	save(s);
	n = allocv(S_string, k+1);
	strcpy(string(n), string(s));
	unsave(1);
	return n;
}

/* Read a string literal. */
int string_literal(void) {
	char	s[TEXT_LEN+1];
	int	c, i, n, q;
	int	inv;

	i = 0;
	q = 0;
	c = read_c();
	inv = 0;
	while (q || c != '"') {
		if (Error_flag) break;
		if (i >= TEXT_LEN-2) {
			error("string literal too long", NOEXPR);
			i = i-1;
		}
		if (q && c != '"' && c != '\\') {
			s[i++] = '\\';
			inv = 1;
		}
		s[i] = c;
		q = !q && c == '\\';
		if (!q) i = i+1;
		c = read_c();
	}
	s[i] = 0;
	n = make_string(s, i);
	if (inv) error("invalid escape sequence in string", n);
	return n;
}

/* Report unreadable object */
int unreadable(void) {
	int	c, i;
	char	buf[TEXT_LEN];
	int	d;

	strcpy(buf, "#<");
	i = 2;
	while (1) {
		c = read_c_ci();
		if (c == '>' || c == '\n') break;
		if (i < TEXT_LEN-2) buf[i++] = c;
	}
	buf[i++] = '>';
	buf[i] = 0;
	d = Displaying;
	Displaying = 1;
	error("unreadable object", make_string(buf, i));
	Displaying = d;
	return NIL;
}

#define separator(c) \
	((c) == ' '  || (c) == '\t' || (c) == '\n' || \
	 (c) == '\r' || (c) == '('  || (c) == ')'  || \
	 (c) == ';'  || (c) == '#'  || (c) == '\'' || \
	 (c) == '`'  || (c) == ','  || (c) == '"'  || \
	 (c) == EOF)

int symbol_or_number(int c) {
	char	s[TEXT_LEN];
	int	i;

	i = 0;
	while (!separator(c)) {
		if (i >= TEXT_LEN-2) {
			error("symbol too long", NOEXPR);
			i = i-1;
		}
		s[i] = c;
		i = i+1;
		c = read_c_ci();
	}
	s[i] = 0;
	reject(c);
	if (str_numeric_p(s)) return string_to_bignum(s);
	return add_symbol(s);
}

int nested_comment(void) {
	int	p, c, k;

	k = 1;
	p = 0;
	c = read_c();
	while (k) {
		if (c == EOF) fatal("end of input in nested comment");
		if (p == '#' && c == '|') { k++; c = 0; }
		if (p == '|' && c == '#') { k--; c = 0; }
		p = c;
		c = read_c();
	}
	return c;
}

static int list_to_vector(int m, char *msg) {
	int	n;
	int	vec, k;
	int	*p;

	k = 0;
	for (n = m; n != NIL; n = Cdr[n]) {
		if (atom_p(n)) return error(msg, m);
		k++;
	}
	vec = allocv(S_vector, k*sizeof(int));
	p = vector(vec);
	for (n = m; n != NIL; n = Cdr[n]) {
		*p = Car[n];
		p++;
	}
	return vec;
}

static int read_vector(void) {
	int	n;

	n = read_list();
	save(n);
	n = list_to_vector(n, "bad vector syntax");
	unsave(1);
	return n;
}

int read_form(void) {
	int	c, c2;

	c = read_c_ci();
	while (1) {	/* Skip spaces and comments */
		while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
			if (Error_flag) return NIL;
			c = read_c_ci();
		}
		if (c == '#') {
			c = read_c_ci();
			if (c == '|') {
				c = nested_comment();
				continue;
			}
			if (c == ';') {
				read_form();
				c = read_c_ci();
				continue;
			}
			if (c != '!') {
				reject(c);
				c = '#';
				break;
			}
		}
		else if (c != ';')
			break;
		while (c != '\n' && c != EOF) c = read_c_ci();
	}
	if (c == EOF) return ENDOFFILE;
	if (c == '(') {
		return read_list();
	}
	else if (c == '\'') {
		return quote(read_form(), S_quote);
	}
	else if (c == '`') {
		return quote(read_form(), S_quasiquote);
	}
	else if (c == ',') {
		c = read_c_ci();
		if (c == '@') {
			return quote(read_form(), S_unquote_splicing);
		}
		else {
			reject(c);
			return quote(read_form(), S_unquote);
		}
	}
	else if (c == '#') {
		c = read_c_ci();
		if (c == 'f') return FALSE;
		if (c == 't') return TRUE;
		if (c == '\\') return character();
		if (c == '(') return read_vector();
		if (c == '<') return unreadable();
		return error("bad # syntax", NOEXPR);
	}
	else if (c == '"') {
		return string_literal();
	}
	else if (c == ')') {
		if (!Level) return error("unexpected ')'", NOEXPR);
		return RPAREN;
	}
	else if (c == '.') {
		c2 = read_c_ci();
		reject(c2);
		if (separator(c2)) {
			if (!Level) return error("unexpected '.'", NOEXPR);
			return DOT;
		}
		return symbol_or_number(c);
	}
	else {
		return symbol_or_number(c);
	}
}

int xread(void) {
	if (Ports[Input_port] == NULL)
		return error("input port is not open", NOEXPR);
	Level = 0;
	return read_form();
}

/*----- Printer -----*/

/* Print bignum integer. */
int print_integer(int n) {
	int	first;
	char	buf[DIGITS_PER_WORD+2];

	if (Car[n] != S_integer) return 0;
	n = Cdr[n];
	first = 1;
	while (1) {
		if (n == NIL) break;
		if (first)
			sprintf(buf, "%d", Car[n]);
		else
			sprintf(buf, "%0*d", DIGITS_PER_WORD, Car[n]);
		pr(buf);
		n = Cdr[n];
		first = 0;
	}
	return -1;
}

void print(int n);

/* Print expressions of the form (QUOTE X) as 'X. */
int print_quoted(int n) {
	if (	Car[n] == S_quote &&
		Cdr[n] != NIL &&
		cddr(n) == NIL
	) {
		pr("'");
		print(cadr(n));
		return 1;
	}
	return 0;
}

int print_procedure(int n) {
	if (Car[n] == S_procedure) {
		pr("#<procedure ");
		print(cadr(n));
	/*	pr(" ");		*/
	/*	print(caddr(n));	*/
	/*	pr(" ");		*/
	/*	print(cdddr(n));	*/
		pr(">");
		return -1;
	}
	return 0;
}

int print_char(int n) {
	char	b[2];
	int	c;

	if (Car[n] != S_char) return 0;
	if (!Displaying) pr("#\\");
	c = cadr(n);
	if (!Displaying && c == ' ') {
		pr("space");
	}
	else if (!Displaying && c == '\n') {
		pr("newline");
	}
	else {
		b[1] = 0;
		b[0] = c;
		pr(b);
	}
	return -1;
}

int print_string(int n) {
	char	b[2];
	int	k;
	char	*s;

	if (Car[n] != S_string) return 0;
	if (!Displaying) pr("\"");
	s = string(n);
	k = string_len(n);
	b[1] = 0;
	while (k) {
		b[0] = *s++;
		if (!Displaying)
			if (b[0] == '"' || b[0] == '\\')
				pr("\\");
		pr(b);
		k = k-1;
	}
	if (!Displaying) pr("\"");
	return -1;
}

int print_symbol(int n) {
	char	b[2];
	int	k;
	char	*s;

	if (Car[n] != S_symbol) return 0;
	s = string(n);
	k = string_len(n);
	b[1] = 0;
	while (k) {
		b[0] = *s++;
		pr(b);
		k = k-1;
	}
	return -1;
}

int print_primitive(int n) {
	if (Car[n] != S_primitive) return 0;
	pr("#<primitive ");
	print(cddr(n));
	pr(">");
	return -1;
}

int print_syntax(int n) {
	if (Car[n] != S_syntax) return 0;
	pr("#<syntax>");
	return -1;
}

int print_vector(int n) {
	int	*p;
	int	k;

	if (Car[n] != S_vector) return 0;
	pr("#(");
	p = vector(n);
	k = vector_len(n);
	while (k--) {
		print(*p++);
		if (k) pr(" ");
	}
	pr(")");
	return -1;
}

int print_port(int n) {
	char	buf[100];

	if (Car[n] != S_input_port && Car[n] != S_output_port)
		return 0;
	sprintf(buf, "#<%s-port %d>",
		Car[n] == S_input_port? "input": "output",
		cadr(n));
	pr(buf);
	return -1;
}

void print(int n) {
	if (Ports[Output_port] == NULL) {
		error("output port is not open", NOEXPR);
		return;
	}
	else if (n == NIL) {
		pr("()");
	}
	else if (n == ENDOFFILE) {
		pr("#<eof>");
	}
	else if (n == FALSE) {
		pr("#f");
	}
	else if (n == TRUE) {
		pr("#t");
	}
	else if (n == UNDEFINED) {
		pr("#<undefined>");
	}
	else if (n == UNSPECIFIC) {
		pr("#<unspecific>");
	}
	else {
		if (print_char(n)) return;
		if (print_procedure(n)) return;
		if (print_integer(n)) return;
		if (print_primitive(n)) return;
		if (print_quoted(n)) return;
		if (print_string(n)) return;
		if (print_symbol(n)) return;
		if (print_syntax(n)) return;
		if (print_vector(n)) return;
		if (print_port(n)) return;
		pr("(");
		while (n != NIL) {
			print(Car[n]);
			n = Cdr[n];
			if (n != NIL && atom_p(n)) {
				pr(" . ");
				print(n);
				n = NIL;
			}
			if (n != NIL) pr(" ");
		}
		pr(")");
	}
}

/*----- Miscellanea -----*/

int length(int n) {
	int	k = 0;

	while (n != NIL) {
		k++;
		n = Cdr[n];
	}
	return k;
}

int appendb(int a, int b) {
	int	p, last = NIL;

	if (a == NIL) return b;
	p = a;
	while (p != NIL) {
		if (atom_p(p)) fatal("append!: improper list");
		last = p;
		p = Cdr[p];
	}
	Cdr[last] = b;
	return a;
}

int flat_copy(int n, int *lastp) {
	int     a, m, last, new;

	if (n == NIL) {
		lastp[0] = NIL;
		return NIL;
	}
	m = alloc(NIL, NIL);
	save(m);
	a = m;
	last = m;
	while (n != NIL) {
		Car[a] = Car[n];
		last = a;
		n = Cdr[n];
		if (n != NIL) {
			new = alloc(NIL, NIL);
			Cdr[a] = new;
			a = Cdr[a];
		}
	}
	unsave(1);
	lastp[0] = last;
	return m;
}

int argument_list_p(int n) {
	if (n == NIL || symbol_p(n)) return 1;
	if (atom_p(n)) return 0;
	while (!atom_p(n)) {
		if (!symbol_p(Car[n])) return 0;
		n = Cdr[n];
	}
	return n == NIL || symbol_p(n);
}

int list_of_symbols_p(int n) {
	return !symbol_p(n) && argument_list_p(n);
}

void rehash(int e) {
	int		i, p, *v;
	unsigned int	h, k = length(e)-1;
	char		*s;
	int		new;

	if (Program == NIL || k < HASH_THRESHOLD) return;
	new = allocv(S_vector, k * sizeof(int));
	Car[e] = new;
	v = vector(Car[e]);
	for (i=0; i<k; i++) v[i] = NIL;
	p = Cdr[e];
	for (i=1; i<k; i++) {
		s = string(caar(p));
		h = 0;
		while (*s) h = (h<<8) + *s++;
		v[h%k] = Car[p];
		p = Cdr[p];
	}
}

int extend(int v, int a, int e) {
	int	n, new;

	n = alloc(a, NIL);
	n = alloc(v, n);
	new = alloc(n, Cdr[e]);
	Cdr[e] = new;
	rehash(e);
	return e;
}

int make_env(int rib, int env) {
	int	e;

	Tmp = env;
	rib = alloc(NIL, rib);
	e = alloc(rib, env);
	Tmp = NIL;
	if (length(rib) >= HASH_THRESHOLD) {
		save(e);
		rehash(rib);
		unsave(1);
	}
	return e;
}

/* hash stats */
int coll = 0, hits = 0;

int try_hash(int v, int e) {
	int		*hv, p;
	unsigned int	h, k;
	char		*s;

	if (e == NIL || Car[e] == NIL) return NIL;
	hv = vector(Car[e]);
	k = vector_len(Car[e]);
	s = string(v);
	h = 0;
	while (*s) h = (h<<8) + *s++;
	p = hv[h%k];
	if (p != NIL && Car[p] == v) {
		hits++;
		return p;
	}
	coll++;
	return NIL;
}

/*----- Evaluator -----*/

int lookup(int v, int env) {
	int	e, n;

	while (env != NIL) {
		e = Car[env];
		n = try_hash(v, e);
		if (n != NIL) return n;
		while (e != NIL) {
			if (v == caar(e)) return Car[e];
			e = Cdr[e];
		}
		env = Cdr[env];
	}
	return NIL;
}

int location_of(int v, int env) {
	int	n;

	n = lookup(v, env);
	if (n == NIL) {
		if (special_p(v))
			error("bad syntax", v);
		else
			error("symbol not bound", v);
		return NIL;
	}
	return Cdr[n];
}

int value_of(int v, int env) {
	int	n;

	n = location_of(v, env);
	return n == NIL? NIL: Car[n];
}

/*----- Specials -----*/

int too_few_args(int n) {
	return error("too few arguments", n);
}

int too_many_args(int n) {
	return error("too many arguments", n);
}

/* Set up sequence for AND, BEGIN, OR. */
int make_sequence(int state, int neutral, int x, int *pc, int *ps) {
	if (Cdr[x] == NIL) {
		return neutral;
	}
	else if (cddr(x) == NIL) {
		*pc = 1;
		return cadr(x);
	}
	else {
		*pc = 2;
		*ps = state;
		save(Cdr[x]);
		return cadr(x);
	}
}

#define sf_and(x, pc, ps) \
	make_sequence(MCONJ, TRUE, x, pc, ps)

#define sf_begin(x, pc, ps) \
	make_sequence(MBEGN, UNSPECIFIC, x, pc, ps)

int sf_cond(int x, int *pc, int *ps) {
	int	clauses, p;

	clauses = Cdr[x];
	p = clauses;
	while (p != NIL) {
		if (atom_p(p) || atom_p(Car[p]) || atom_p(cdar(p)))
			return error("cond: bad syntax", p);
		p = Cdr[p];
	}
	if (clauses == NIL) return UNSPECIFIC;
	if (caar(clauses) == S_else && Cdr[clauses] == NIL) {
		p = alloc(TRUE, cdar(clauses));
		clauses = alloc(p, Cdr[clauses]);
	}
	save(clauses);
	*pc = 2;
	*ps = MCOND;
	return caar(clauses);
}

int sf_if(int x, int *pc, int *ps) {
	int	m;

	m = Cdr[x];
	if (m == NIL || Cdr[m] == NIL)
		return too_few_args(x);
	if (cddr(m) != NIL && cdddr(m) != NIL)
		return too_many_args(x);
	if (cddr(m) == NIL)
		cddr(m) = alloc(UNSPECIFIC, NIL);
	save(m);
	*pc = 2;
	*ps = MIFPR;
	return Car[m];
}

int make_temporaries(int x) {
	int	n, v, k = 0;
	char	buf[10];

	n = NIL;
	save(n);
	while (x != NIL) {
		sprintf(buf, "##%d", k);
		v = add_symbol(buf);
		n = alloc(v, n);
		Car[Stack] = n;
		x = Cdr[x];
		k++;
	}
	unsave(1);
	return n;
}

int make_assignments(int x, int t) {
	int	n, asg;

	n = NIL;
	save(n);
	while (x != NIL) {
		asg = alloc(Car[t], NIL);
		asg = alloc(Car[x], asg);
		asg = alloc(S_set_b, asg);
		n = alloc(asg, n);
		Car[Stack] = n;
		x = Cdr[x];
		t = Cdr[t];
	}
	unsave(1);
	return alloc(S_begin, n);
}

int make_undefineds(int x) {
	int	n;

	n = NIL;
	while (x != NIL) {
		n = alloc(UNDEFINED, n);
		x = Cdr[x];
	}
	return n;
}

/* Returns ((lambda (v0 ...)
 *            ((lambda (t0 ...)
 *               (begin (set! v0 t0)
 *                      ...
 *                      body))
 *             a0 ...))
 *          #<undefined> ...)
 */
int make_recursive_lambda(int v, int a, int body) {
	int	t, n;

	t = make_temporaries(v);
	save(t);
	body = appendb(make_assignments(v, t), body);
	body = alloc(body, NIL);
	n = alloc(t, body);
	n = alloc(S_lambda, n);
	n = alloc(n, a);
	n = alloc(n, NIL);
	n = alloc(v, n);
	n = alloc(S_lambda, n);
	save(n);
	n = alloc(n, make_undefineds(v));
	unsave(2);
	return n;
}

#define VARS 1
#define ARGS 2

/* Extract variables or arguments from LET/LETREC. */
int extract_from_let(int caller, int x, int part) {
	int	a, n;
	char	*err;

	err = caller == S_let? "let: bad syntax":
				"letrec: bad syntax";
	a = NIL;
	while (x != NIL) {
		if (atom_p(x)) return error(err, x);
		n = Car[x];
		if (atom_p(n) || Cdr[n] == NIL || cddr(n) != NIL)
			return error(err, x);
		a = alloc(part==VARS? caar(x): cadar(x), a);
		x = Cdr[x];
	}
	return a;
}

/* Extract variables or arguments from a set of DEFINEs. */
int extract_from_defines(int x, int part, int *restp) {
	int	a, n, new;

	a = NIL;
	while (x != NIL) {
		if (atom_p(x) || atom_p(Car[x]) || caar(x) != S_define)
			break;
		n = Car[x];
		if (length(n) != 3)
			return error("define: bad syntax", n);
		if (pair_p(cadr(n))) {
			/* (define (proc vars) ...) */
			if (part == VARS) {
				a = alloc(caadr(n), a);
			}
			else {
				a = alloc(NIL, a);
				save(a);
				new = alloc(cdadr(n), cddr(n));
				new = alloc(S_lambda, new);
				Car[a] = new;
				unsave(1);
			}
		}
		else {
			a = alloc(part==VARS? cadr(n): caddr(n), a);
		}
		x = Cdr[x];
	}
	*restp = x;
	return a;
}

/* Rewrite local defines using LAMBDA and SET! */
int resolve_local_defines(int x) {
	int	v, a, n, rest;

	a = extract_from_defines(x, ARGS, &rest);
	if (Error_flag) return NIL;
	save(a);
	v = extract_from_defines(x, VARS, &rest);
	save(v);
	if (rest == NIL) rest = alloc(UNSPECIFIC, NIL);
	save(rest);
	n = make_recursive_lambda(v, a, rest);
	unsave(3);
	return n;
}

int sf_lambda(int x) {
	int	n, k;

	k = length(x);
	if (k < 3) return too_few_args(x);
	if (!argument_list_p(cadr(x)))
		return error("bad argument list", cadr(x));
	if (pair_p(caddr(x)) && caaddr(x) == S_define)
		n = resolve_local_defines(cddr(x));
	else if (k > 3)
		n = alloc(S_begin, cddr(x));
	else
		n = caddr(x);
	n = alloc(n, Environment);
	n = alloc(cadr(x), n);
	return alloc3(S_procedure, n, AFLAG);
}

/* Transform LET to LAMBDA */
int sf_let(int x, int *pc) {
	int	v, a, b;
	int	n, e;

	if (length(x) < 3) too_few_args(x);
	e = cadr(x);
	a = extract_from_let(S_let, e, ARGS);
	if (Error_flag) return NIL;
	save(a);
	v = extract_from_let(S_let, e, VARS);
	b = cddr(x);
	n = alloc(v, b);
	n = alloc(S_lambda, n);
	n = alloc(n, a);
	unsave(1);
	*pc = 1;
	return n;
}

/* Transform LETREC to LAMBDA and SET! */
int sf_letrec(int x, int *pc) {
	int	v, a;
	int	n, e;

	if (length(x) < 3) too_few_args(x);
	e = cadr(x);
	a = extract_from_let(S_letrec, e, ARGS);
	if (Error_flag) return NIL;
	save(a);
	v = extract_from_let(S_letrec, e, VARS);
	save(v);
	n = make_recursive_lambda(v, a, cddr(x));
	unsave(2);
	*pc = 1;
	return n;
}

int sf_quote(int x) {
	int	k = length(x);

	if (k < 2) return too_few_args(x);
	if (k > 2) return too_many_args(x);
	return cadr(x);
}

#define sf_or(x, pc, ps) \
	make_sequence(MDISJ, FALSE, x, pc, ps)

int sf_set_b(int x, int *pc, int *ps) {
	int	n, k;

	k = length(x);
	if (k < 3) return too_few_args(x);
	if (k > 3) return too_many_args(x);
	if (!symbol_p(cadr(x)))
		return error("set!: symbol expected", cadr(x));
	n = location_of(cadr(x), Environment);
	if (Error_flag) return NIL;
	save(n);
	*pc = 2;
	*ps = MSETV;
	return caddr(x);
}

int find_local_variable(int v, int e) {
	while (e != NIL) {
		if (v == caar(e)) return Car[e];
		e = Cdr[e];
	}
	return NIL;
}

int sf_define(int x, int *pc, int *ps) {
	int	v, a, n, k, new;

	if (Car[State_stack] == MARGS)
		return error("define: bad local context", x);
	k = length(x);
	if (k < 3) return too_few_args(x);
	if (symbol_p(cadr(x)) && k > 3) return too_many_args(x);
	if (!argument_list_p(cadr(x)))
		return error("define: expected symbol or list, got", cadr(x));
	if (!symbol_p(cadr(x))) {
		a = cddr(x);
		a = alloc(cdadr(x), a);
		a = alloc(S_lambda, a);
		Tmp = a;
		n = caadr(x);
	}
	else {
		a = caddr(x);
		n = cadr(x);
	}
	v = find_local_variable(n, Car[Environment]);
	if (v == NIL) {
		new = extend(n, UNDEFINED, Car[Environment]);
		Car[Environment] = new;
		v = cadar(Environment);
	}
	save(Cdr[v]);
	*pc = 2;
	if (!atom_p(a) && Car[a] == S_lambda)
		*ps = MDEFN;	/* use dynamic scoping */
	else
		*ps = MSETV;
	Tmp = NIL;
	return a;
}

int sf_define_syntax(int x, int *pc, int *ps) {
	int	a, n, v, new, k = length(x);

	if (k < 3) return too_few_args(x);
	if (k > 3) return too_many_args(x);
	if (!symbol_p(cadr(x)))
		return error("define-syntax: expected symbol, got", cadr(x));
	a = caddr(x);
	n = cadr(x);
	v = lookup(n, Environment);
	if (v == NIL) {
		new = extend(n, UNDEFINED, Car[Environment]);
		Car[Environment] = new;
		v = cadar(Environment);
	}
	save(Cdr[v]);
	*pc = 2;
	*ps = MDSYN;
	return a;
}

int sf_syntax_rules(int x) {
	int	m, cl, k = length(x);

	m = Cdr[x];
	if (k < 3) return too_few_args(x);
	if (!list_of_symbols_p(Car[m]))
		return error("syntax-rules: expected list of symbols, got",
			Car[m]);
	cl = Cdr[m];
	while (cl != NIL) {
		if (atom_p(cl))
			return error("syntax-rules: improper list of rules",
				Cdr[m]);
		if (atom_p(Car[cl]) || atom_p(cdar(cl)))
			return error("syntax-rules: bad clause", Car[cl]);
		cl = Cdr[cl];
	}
	return alloc3(S_syntax, m, AFLAG);
}

/*----- Bignums -----*/

int make_integer(int i) {
	int	n;

	n = alloc3(i, NIL, AFLAG);
	return alloc3(S_integer, n, AFLAG);
}

int integer_value(char *src, int x) {
	char	msg[100];

	if (cddr(x) != NIL) {
		sprintf(msg, "%s: integer argument too big", src);
		error(msg, x);
		return 0;
	}
	return cadr(x);
}

int bignum_abs(int a) {
	int	n;

	n = alloc3(abs(cadr(a)), cddr(a), AFLAG);
	return alloc3(S_integer, n, AFLAG);
}

int bignum_negate(int a) {
	int	n;

	n = alloc3(-cadr(a), cddr(a), AFLAG);
	return alloc3(S_integer, n, AFLAG);
}

#define bignum_negative_p(a) ((cadr(a)) < 0)

#define bignum_zero_p(a) ((cadr(a) == 0) && (cddr(a)) == NIL)

int reverse_segments(int n) {
	int	m;

	m = NIL;
	save(m);
	while (n != NIL) {
		m = alloc3(Car[n], m, AFLAG);
		Car[Stack] = m;
		n = Cdr[n];
	}
	unsave(1);
	return m;
}

int bignum_add(int a, int b);
int bignum_subtract(int a, int b);

int _bignum_add(int a, int b) {
	int	fa, fb, carry, r;
	int	result;

	if (bignum_negative_p(a)) {
		if (bignum_negative_p(b)) {
			/* -A+-B --> -(|A|+|B|) */
			a = bignum_abs(a);
			save(a);
			b = bignum_abs(b);
			save(b);
			a = bignum_add(a, b);
			unsave(2);
			return bignum_negate(a);
		}
		else {
			/* -A+B --> B-|A| */
			a = bignum_abs(a);
			save(a);
			a = bignum_subtract(b, a);
			unsave(1);
			return a;
		}
	}
	else if (bignum_negative_p(b)) {
		/* A+-B --> A-|B| */
		b = bignum_abs(b);
		save(b);
		a = bignum_subtract(a, b);
		unsave(1);
		return a;
	}
	/* A+B */
	a = reverse_segments(Cdr[a]);
	save(a);
	b = reverse_segments(Cdr[b]);
	save(b);
	carry = 0;
	result = NIL;
	save(result);
	while (a != NIL || b != NIL || carry) {
		fa = a==NIL? 0: Car[a];
		fb = b==NIL? 0: Car[b];
		r = fa + fb + carry;
		carry = 0;
		if (r >= INT_SEG_LIMIT) {
			r -= INT_SEG_LIMIT;
			carry = 1;
		}
		result = alloc3(r, result, AFLAG);
		Car[Stack] = result;
		if (a != NIL) a = Cdr[a];
		if (b != NIL) b = Cdr[b];
	}
	unsave(3);
	return alloc3(S_integer, result, AFLAG);
}

int bignum_add(int a, int b) {
	Tmp = b;
	save(a);
	save(b);
	Tmp = NIL;
	a = _bignum_add(a, b);
	unsave(2);
	return a;
}

int bignum_less_p(int a, int b) {
	int	ka, kb, neg_a, neg_b;

	neg_a = bignum_negative_p(a);
	neg_b = bignum_negative_p(b);
	if (neg_a && !neg_b) return 1;
	if (!neg_a && neg_b) return 0;
	ka = length(a);
	kb = length(b);
	if (ka < kb) return neg_a? 0: 1;
	if (ka > kb) return neg_a? 1: 0;
	Tmp = b;
	a = bignum_abs(a);
	save(a);
	b = bignum_abs(b);
	unsave(1);
	Tmp = NIL;
	a = Cdr[a];
	b = Cdr[b];
	while (a != NIL) {
		if (Car[a] < Car[b]) return neg_a? 0: 1;
		if (Car[a] > Car[b]) return neg_a? 1: 0;
		a = Cdr[a];
		b = Cdr[b];
	}
	return 0;
}

int bignum_equal_p(int a, int b) {
	a = Cdr[a];
	b = Cdr[b];
	while (a != NIL && b != NIL) {
		if (Car[a] != Car[b]) return 0;
		a = Cdr[a];
		b = Cdr[b];
	}
	return a == NIL && b == NIL;
}

int _bignum_subtract(int a, int b) {
	int	fa, fb, borrow, r;
	int	result;

	if (bignum_negative_p(a)) {
		if (bignum_negative_p(b)) {
			/* -A--B --> -A+|B| --> |B|-|A| */
			a = bignum_abs(a);
			save(a);
			b = bignum_abs(b);
			save(b);
			a = bignum_subtract(b, a);
			unsave(2);
			return a;
		}
		else {
			/* -A-B --> -(|A|+B) */
			a = bignum_abs(a);
			save(a);
			a = bignum_add(a, b);
			unsave(1);
			return bignum_negate(a);
		}
	}
	else if (bignum_negative_p(b)) {
		/* A--B --> A+|B| */
		b = bignum_abs(b);
		save(b);
		a = bignum_add(a, b);
		unsave(1);
		return a;
	}
	/* A-B, A<B --> -(B-A) */
	if (bignum_less_p(a, b))
		return bignum_negate(bignum_subtract(b, a));
	/* A-B, A>=B */
	a = reverse_segments(Cdr[a]);
	save(a);
	b = reverse_segments(Cdr[b]);
	save(b);
	borrow = 0;
	result = NIL;
	save(result);
	while (a != NIL || b != NIL || borrow) {
		fa = a==NIL? 0: Car[a];
		fb = b==NIL? 0: Car[b];
		r = fa - fb - borrow;
		borrow = 0;
		if (r < 0) {
			r += INT_SEG_LIMIT;
			borrow = 1;
		}
		result = alloc3(r, result, AFLAG);
		Car[Stack] = result;
		if (a != NIL) a = Cdr[a];
		if (b != NIL) b = Cdr[b];
	}
	unsave(3);
	while (Car[result] == 0 && Cdr[result] != NIL)
		result = Cdr[result];
	return alloc3(S_integer, result, AFLAG);
}

int bignum_subtract(int a, int b) {
	Tmp = b;
	save(a);
	save(b);
	Tmp = NIL;
	a = _bignum_subtract(a, b);
	unsave(2);
	return a;
}

int bignum_shift_left(int a, int fill) {
	int	r, carry, c;
	int	result;

	a = reverse_segments(Cdr[a]);
	save(a);
	carry = fill;
	result = NIL;
	save(result);
	while (a != NIL) {
		if (Car[a] >= INT_SEG_LIMIT/10) {
			c = Car[a] / (INT_SEG_LIMIT/10);
			r = Car[a] % (INT_SEG_LIMIT/10) * 10;
			r += carry;
			carry = c;
		}
		else {
			r = Car[a] * 10 + carry;
			carry = 0;
		}
		result = alloc3(r, result, AFLAG);
		Car[Stack] = result;
		a = Cdr[a];
	}
	if (carry) result = alloc3(carry, result, AFLAG);
	unsave(2);
	return alloc3(S_integer, result, AFLAG);
}

/* Result: (a/10 . a%10) */
int bignum_shift_right(int a) {
	int	r, carry, c;
	int	result;

	a = Cdr[a];
	save(a);
	carry = 0;
	result = NIL;
	save(result);
	while (a != NIL) {
		c = Car[a] % 10;
		r = Car[a] / 10;
		r += carry * (INT_SEG_LIMIT/10);
		carry = c;
		result = alloc3(r, result, AFLAG);
		Car[Stack] = result;
		a = Cdr[a];
	}
	result = reverse_segments(result);
	if (Car[result] == 0 && Cdr[result] != NIL) result = Cdr[result];
	result = alloc3(S_integer, result, AFLAG);
	Car[Stack] = result;
	carry = make_integer(carry);
	unsave(2);
	return alloc(result, carry);
}

int bignum_multiply(int a, int b) {
	int	neg, result, r, i;

	neg = bignum_negative_p(a) != bignum_negative_p(b);
	a = bignum_abs(a);
	save(a);
	b = bignum_abs(b);
	save(b);
	result = make_integer(0);
	save(result);
	while (!bignum_zero_p(a)) {
		r = bignum_shift_right(a);
		i = caddr(r);
		a = Car[r];
		caddr(Stack) = a;
		while (i) {
			result = bignum_add(result, b);
			Car[Stack] = result;
			i--;
		}
		b = bignum_shift_left(b, 0);
		cadr(Stack) = b;
	}
	if (neg) result = bignum_negate(result);
	unsave(3);
	return result;
}

int bignum_equalize(int a, int b) {
	int	r, f, r0, f0;

	r0 = a;
	save(r0);
	f0 = make_integer(1);
	save(f0);
	r = r0;
	save(r);
	f = f0;
	save(f);
	while (bignum_less_p(r, b)) {
		cadddr(Stack) = r0 = r;
		caddr(Stack) = f0 = f;
		r = bignum_shift_left(r, 0);
		cadr(Stack) = r;
		f = bignum_shift_left(f, 0);
		Car[Stack] = f;
	}
	unsave(4);
	return alloc(r0, f0);
}

/* Result: (a/b . a%b) */
int _bignum_divide(int a, int b) {
	int	neg, neg_a, result, f;
	int	i, c, c0;

	neg_a = bignum_negative_p(a);
	neg = neg_a != bignum_negative_p(b);
	a = bignum_abs(a);
	save(a);
	b = bignum_abs(b);
	save(b);
	if (bignum_less_p(a, b)) {
		if (neg_a) a = bignum_negate(a);
		unsave(2);
		return alloc(make_integer(0), a);
	}
	b = bignum_equalize(b, a);
	cadr(Stack) = b; /* cadddddr */
	Car[Stack] = a;	/* caddddr */
	c = NIL;
	save(c);	/* cadddr */
	c0 = NIL;
	save(c0);	/* caddr */
	f = Cdr[b];
	b = Car[b];
	cadddr(Stack) = b;
	save(f);	/* cadr */
	result = make_integer(0);
	save(result);	/* Car */
	while (!bignum_zero_p(f)) {
		c = make_integer(0);
		cadddr(Stack) = c;
		caddr(Stack) = c0 = c;
		i = 0;
		while (!bignum_less_p(a, c)) {
			caddr(Stack) = c0 = c;
			c = bignum_add(c, b);
			cadddr(Stack) = c;
			i++;
		}
		result = bignum_shift_left(result, i-1);
		Car[Stack] = result;
		a = bignum_subtract(a, c0);
		Car[cddddr(Stack)] = a;
		f = Car[bignum_shift_right(f)];
		cadr(Stack) = f;
		b = Car[bignum_shift_right(b)];
		cadr(cddddr(Stack)) = b;
	}
	if (neg) result = bignum_negate(result);
	Car[Stack] = result;
	if (neg_a) a = bignum_negate(a);
	unsave(6);
	return alloc(result, a);
}

int bignum_divide(int x, int a, int b) {
	if (bignum_zero_p(b))
		return error("divide by zero", x);
	Tmp = b;
	save(a);
	save(b);
	Tmp = NIL;
	a = _bignum_divide(a, b);
	unsave(2);
	return a;
}

/*----- Primitives -----*/

int pp_apply(int x) {
	int	m, p, q, last;
	char	*err = "apply: improper argument list";

	m = Cdr[x];
	p = Cdr[m];
	last = p;
	while (p != NIL) {
		if (atom_p(p)) return error(err, x);
		last = p;
		p = Cdr[p];
	}
	p = Car[last];
	while (p != NIL) {
		if (atom_p(p)) return error(err, Car[last]);
		p = Cdr[p];
	}
	if (cddr(m) == NIL) {
		p = cadr(m);
	}
	else {
		p = flat_copy(Cdr[m], &q);
		q = p;
		while (cddr(q) != NIL) q = Cdr[q];
		Cdr[q] = Car[last];
	}
	return alloc(Car[m], p);
}

int pp_boolean_p(int x) {
	return boolean_p(cadr(x))? TRUE: FALSE;
}

int pp_car(int x) {
	return caadr(x);
}

int pp_cdr(int x) {
	return cdadr(x);
}

int pp_char_p(int x) {
	return char_p(cadr(x))? TRUE: FALSE;
}

int pp_char_to_integer(int x) {
	return make_integer(cadadr(x));
}

int pp_char_alphabetic_p(int x) {
	return isalpha(char_value(cadr(x)))? TRUE: FALSE;
}

#define L(c) tolower(c)
int char_ci_le(int c1, int c2) { return L(c1) <= L(c2); }
int char_ci_lt(int c1, int c2) { return L(c1) <  L(c2); }
int char_ci_eq(int c1, int c2) { return L(c1) == L(c2); }
int char_ci_ge(int c1, int c2) { return L(c1) >= L(c2); }
int char_ci_gt(int c1, int c2) { return L(c1) >  L(c2); }

int char_le(int c1, int c2) { return c1 <= c2; }
int char_lt(int c1, int c2) { return c1 <  c2; }
int char_eq(int c1, int c2) { return c1 == c2; }
int char_ge(int c1, int c2) { return c1 >= c2; }
int char_gt(int c1, int c2) { return c1 >  c2; }

int char_predicate(char *name, int (*p)(int c1, int c2), int x) {
	char	msg[100];

	x = Cdr[x];
	while (Cdr[x] != NIL) {
		if (!char_p(cadr(x))) {
			sprintf(msg, "%s: expected char, got", name);
			return error(msg, cadr(x));
		}
		if (!p(char_value(Car[x]), char_value(cadr(x))))
			return FALSE;
		x = Cdr[x];
	}
	return TRUE;
}

#define R return
int pp_char_ci_le_p(int x) { R char_predicate("char-ci<=?", char_ci_le, x); }
int pp_char_ci_lt_p(int x) { R char_predicate("char-ci<?",  char_ci_lt, x); }
int pp_char_ci_eq_p(int x) { R char_predicate("char-ci=?",  char_ci_eq, x); }
int pp_char_ci_ge_p(int x) { R char_predicate("char-ci>=?", char_ci_ge, x); }
int pp_char_ci_gt_p(int x) { R char_predicate("char-ci>?",  char_ci_gt, x); }

int pp_char_le_p(int x) { R char_predicate("char<=?", char_le, x); }
int pp_char_lt_p(int x) { R char_predicate("char<?",  char_lt, x); }
int pp_char_eq_p(int x) { R char_predicate("char=?",  char_eq, x); }
int pp_char_ge_p(int x) { R char_predicate("char>=?", char_ge, x); }
int pp_char_gt_p(int x) { R char_predicate("char>?",  char_gt, x); }

int pp_char_downcase(int x) {
	return make_char(tolower(char_value(cadr(x))));
}

int pp_char_lower_case_p(int x) {
	return islower(char_value(cadr(x)))? TRUE: FALSE;
}

int pp_char_numeric_p(int x) {
	return isdigit(char_value(cadr(x)))? TRUE: FALSE;
}

int pp_char_upcase(int x) {
	return make_char(toupper(char_value(cadr(x))));
}

int pp_char_upper_case_p(int x) {
	return isupper(char_value(cadr(x)))? TRUE: FALSE;
}

int pp_char_whitespace_p(int x) {
	int	c = char_value(cadr(x));

	return (c == ' '  || c == '\t' || c == '\n' ||
	        c == '\r' || c == '\f')? TRUE: FALSE;
}

void close_port(int port) {
	if (port < 0 || port >= MAX_PORTS) return;
	if (Ports[port] == NULL) return;
	if (fclose(Ports[port]))
		fatal("close_port(): fclose() failed");
	Ports[port] = NULL;
	Port_flags[port] = 0;
}

int pp_close_input_port(int x) {
	if (port_no(cadr(x)) < 2)
		return error("please do not close the standard input port",
				NOEXPR);
	close_port(port_no(cadr(x)));
	return UNSPECIFIC;
}

int pp_close_output_port(int x) {
	if (port_no(cadr(x)) < 2)
		return error("please do not close the standard output port",
				NOEXPR);
	close_port(port_no(cadr(x)));
	return UNSPECIFIC;
}

int pp_cons(int x) {
	return alloc(cadr(x), caddr(x));
}

int make_port(int port_no, int type) {
	int	n;

	n = alloc3(port_no, NIL, AFLAG);
	return alloc3(type, n, AFLAG);
}

int pp_current_input_port(int x) {
	return make_port(Input_port, S_input_port);
}

int pp_current_output_port(int x) {
	return make_port(Output_port, S_output_port);
}

int pp_write(int x);

int pp_display(int x) {
	Displaying = 1;
	pp_write(x);
	Displaying = 0;
	return UNSPECIFIC;
}

int pp_eof_object_p(int x) {
	return cadr(x) == ENDOFFILE? TRUE: FALSE;
}

int pp_eq_p(int x) {
	return cadr(x) == caddr(x)? TRUE: FALSE;
}

int pp_equal(int x) {
	x = Cdr[x];
	while (Cdr[x] != NIL) {
		if (!integer_p(cadr(x)))
			return error("=: expected integer, got", cadr(x));
		if (!bignum_equal_p(Car[x], cadr(x))) return FALSE;
		x = Cdr[x];
	}
	return TRUE;
}

int pp_greater(int x) {
	x = Cdr[x];
	while (Cdr[x] != NIL) {
		if (!integer_p(cadr(x)))
			return error(">: expected integer, got", cadr(x));
		if (!bignum_less_p(cadr(x), Car[x])) return FALSE;
		x = Cdr[x];
	}
	return TRUE;
}

int pp_greater_equal(int x) {
	x = Cdr[x];
	while (Cdr[x] != NIL) {
		if (!integer_p(cadr(x)))
			return error(">=: expected integer, got", cadr(x));
		if (bignum_less_p(Car[x], cadr(x))) return FALSE;
		x = Cdr[x];
	}
	return TRUE;
}

int pp_input_port_p(int x) {
	return input_port_p(cadr(x))? TRUE: FALSE;
}

int pp_integer_to_char(int x) {
	int	n;

	n = integer_value("integer->char", cadr(x));
	if (n < 0 || n > 127)
		return error("integer->char: argument value out of range",
				cadr(x));
	return make_char(n);
}

int pp_integer_p(int x) {
	return integer_p(cadr(x))? TRUE: FALSE;
}

int pp_less(int x) {
	x = Cdr[x];
	while (Cdr[x] != NIL) {
		if (!integer_p(cadr(x)))
			return error("<: expected integer, got", cadr(x));
		if (!bignum_less_p(Car[x], cadr(x))) return FALSE;
		x = Cdr[x];
	}
	return TRUE;
}

int pp_less_equal(int x) {
	x = Cdr[x];
	while (Cdr[x] != NIL) {
		if (!integer_p(cadr(x)))
			return error("<=: expected integer, got", cadr(x));
		if (bignum_less_p(cadr(x), Car[x])) return FALSE;
		x = Cdr[x];
	}
	return TRUE;
}

int pp_list_to_string(int x) {
	int	n, p, k = length(cadr(x));
	char	*s;

	n = make_string("", k);
	s = string(n);
	p = cadr(x);
	while (p != NIL) {
		if (atom_p(p))
			return error("list->string: improper list", p);
		if (!char_p(Car[p]))
			return error("list->string: expected list of char, "
				"got list containing",
				Car[p]);
		*s++ = cadar(p);
		p = Cdr[p];
	}
	*s = 0;
	return n;
}

int pp_list_to_vector(int x) {
	return list_to_vector(cadr(x), "improper list in list->vector");
}

int open_port(char *path, char *mode) {
	int	i, tries;

	for (tries=0; tries<2; tries++) {
		for (i=0; i<MAX_PORTS; i++) {
			if (Ports[i] == NULL) {
				Ports[i] = fopen(path, mode);
				if (Ports[i] == NULL)
					return -1;
				else
					return i;
			}
		}
		if (tries == 0) gc();
	}
	return -1;
}

int eval(int x);

int load(char *file) {
	int	n;
	int	new_port, old_port;

	new_port = open_port(file, "r");
	if (new_port == -1) return -1;
	Port_flags[new_port] |= LFLAG;
	old_port = Input_port;
	Input_port = new_port;
	while (!Error_flag) {
		n = xread();
		if (n == ENDOFFILE) break;
		if (!Error_flag) n = eval(n);
	}
	close_port(new_port);
	Input_port = old_port;
	return 0;
}

int pp_load(int x) {
	if (load(string(cadr(x))) < 0)
		return error("load: cannot open file", cadr(x));
	return UNSPECIFIC;
}

int pp_make_string(int x) {
	int	n, c, k;
	char	*s;

	k = integer_value("make-string", cadr(x));
	n = make_string("", k);
	s = string(n);
	c = cddr(x) == NIL? ' ': char_value(caddr(x));
	memset(s, c, k);
	s[k] = 0;
	return n;
}

int pp_make_vector(int x) {
	int	n, i, m, k;
	int	*v;

	k = integer_value("make-vector", cadr(x));
	n = allocv(S_vector, k * sizeof(int));
	v = vector(n);
	m = cddr(x) == NIL? FALSE: caddr(x);
	for (i=0; i<k; i++) v[i] = m;
	return n;
}

int pp_minus(int x) {
	int	a;

	x = Cdr[x];
	if (Cdr[x] == NIL) return bignum_negate(Car[x]);
	a = Car[x];
	x = Cdr[x];
	save(a);
	while (x != NIL) {
		if (!integer_p(Car[x]))
			return error("-: expected integer, got", Car[x]);
		a = bignum_subtract(a, Car[x]);
		Car[Stack] = a;
		x = Cdr[x];
	}
	unsave(1);
	return a;
}

int pp_open_input_file(int x) {
	int	n, p;

	p = open_port(string(cadr(x)), "r");
	if (p < 0) return error("could not open input file", cadr(x));
	Port_flags[p] |= LFLAG;
	n = make_port(p, S_input_port);
	Port_flags[p] &= ~LFLAG;
	return n;
}

int pp_open_output_file(int x) {
	int	n, p;

	p = open_port(string(cadr(x)), "w");
	if (p < 0) return error("could not open output file", cadr(x));
	Port_flags[p] |= LFLAG;
	n = make_port(p, S_output_port);
	Port_flags[p] &= ~LFLAG;
	return n;
}

int pp_output_port_p(int x) {
	return output_port_p(cadr(x))? TRUE: FALSE;
}

int pp_pair_p(int x) {
	return atom_p(cadr(x))? FALSE: TRUE;
}

int pp_plus(int x) {
	int	a;

	x = Cdr[x];
	if (Cdr[x] == NIL) return Car[x];
	a = make_integer(0);
	save(a);
	while (x != NIL) {
		if (!integer_p(Car[x]))
			return error("+: expected integer, got", Car[x]);
		a = bignum_add(a, Car[x]);
		Car[Stack] = a;
		x = Cdr[x];
	}
	unsave(1);
	return a;
}

int pp_procedure_p(int x) {
	return (procedure_p(cadr(x)) || primitive_p(cadr(x)))?
		TRUE: FALSE;
}

int pp_quotient(int x) {
	return Car[bignum_divide(x, cadr(x), caddr(x))];
}

int pp_read(int x) {
	int	n, new_port, old_port;

	new_port = Cdr[x] == NIL? Input_port: port_no(cadr(x));
	if (new_port < 0 || new_port >= MAX_PORTS)
		return error("bad input port", cadr(x));
	old_port = Input_port;
	Input_port = new_port;
	n = xread();
	Input_port = old_port;
	return n;
}

int read_char(int x, int unget) {
	int	c, new_port, old_port;

	new_port = Cdr[x] == NIL? Input_port: port_no(cadr(x));
	if (new_port < 0 || new_port >= MAX_PORTS)
		return error("bad input port", cadr(x));
	if (Ports[new_port] == NULL)
		return error("input port is not open", NOEXPR);
	old_port = Input_port;
	Input_port = new_port;
	c = read_c();
	if (unget) reject(c);
	Input_port = old_port;
	return c == EOF? ENDOFFILE: make_char(c);
}

int pp_peek_char(int x) {
	return read_char(x, 1);
}

int pp_read_char(int x) {
	return read_char(x, 0);
}

int pp_remainder(int x) {
	return Cdr[bignum_divide(x, cadr(x), caddr(x))];
}

int pp_set_car_b(int x) {
	caadr(x) = caddr(x);
	return UNSPECIFIC;
}

int pp_set_cdr_b(int x) {
	cdadr(x) = caddr(x);
	return UNSPECIFIC;
}

int pp_set_input_port_b(int x) {
	Input_port = port_no(cadr(x));
	return UNSPECIFIC;
}

int pp_set_output_port_b(int x) {
	Output_port = port_no(cadr(x));
	return UNSPECIFIC;
}

int pp_string_to_list(int x) {
	char	*s;
	int	n, a, k, i, new;

	s = string(cadr(x));
	k = string_len(cadr(x));
	n = NIL;
	a = NIL;
	for (i=0; i<k-1; i++) {
		if (n == NIL) {
			n = a = alloc(make_char(s[i]), NIL);
			save(n);
		}
		else {
			new = alloc(make_char(s[i]), NIL);
			Cdr[a] = new;
			a = Cdr[a];
		}
	}
	if (n != NIL) unsave(1);
	return n;
}

int pp_string_to_symbol(int x) {
	int	y, n;
	char    *s = string(cadr(x));
 
	y = find_symbol(s);
	if (y != NIL) return y;
	/*
	 * Cannot use make_symbol(), because
	 * string(cadr(x)) may move during GC.
	*/
	Symbols = alloc(NIL, Symbols);
	n = allocv(S_symbol, strlen(s)+1);
	Car[Symbols] = n;
	strcpy(string(n), string(cadr(x)));
	return Car[Symbols];
}


int pp_string_append(int x) {
	int	p, k, n;
	char	*s;

	k = 0;
	for (p = Cdr[x]; p != NIL; p = Cdr[p]) {
		if (!string_p(Car[p]))
			return error("string-append: expected string, got",
					Car[p]);
		k += string_len(Car[p])-1;
	}
	n = make_string("", k);
	s = string(n);
	k = 0;
	for (p = Cdr[x]; p != NIL; p = Cdr[p]) {
		strcpy(&s[k], string(Car[p]));
		k += string_len(Car[p])-1;
	}
	return n;
}

int pp_string_copy(int x) {
	return clone_string(cadr(x), string_len(cadr(x))-1);
}

int pp_string_fill_b(int x) {
	int	c = char_value(caddr(x)),
		i, k = string_len(cadr(x))-1;
	char	*s = string(cadr(x));

	for (i=0; i<k; i++) s[i] = c;
	return UNSPECIFIC;
}

int pp_substring(int x) {
	int	k = string_len(cadr(x))-1;
	int	p0 = integer_value("substring", caddr(x));
	int	pn = integer_value("substring", cadddr(x));
	char	*src = string(cadr(x));
	char	*dst;
	int	n;

	if (p0 < 0 || p0 > k || pn < 0 || pn > k || pn < p0) {
		n = alloc(cadddr(x), NIL);
		return error("substring: bad range",
				alloc(caddr(x), n));
	}
	n = make_string("", pn-p0);
	dst = string(n);
	if (pn-p0 != 0) memcpy(dst, &src[p0], pn-p0);
	dst[pn-p0] = 0;
	return n;
}

int pp_string_length(int x) {
	return make_integer(string_len(cadr(x))-1);
}

int pp_string_ref(int x) {
	int	p, k = string_len(cadr(x))-1;

	p = integer_value("string-ref", caddr(x));
	if (p < 0 || p >= k)
		return error("string-ref: index out of range",
				caddr(x));
	return make_char(string(cadr(x))[p]);
}

int pp_string_set_b(int x) {
	int	p, k = string_len(cadr(x))-1;

	p = integer_value("string-set!", caddr(x));
	if (p < 0 || p >= k)
		return error("string-set!: index out of range",
				caddr(x));
	string(cadr(x))[p] = char_value(cadddr(x));
	return UNSPECIFIC;
}

int strcmp_ci(char *s1, char *s2) {
	int	c1, c2;

	for (;;) {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
		if (!c1 || !c2 || c1 != c2) break;
	}
	return c1<c2? -1: c1>c2? 1: 0;
}

int string_ci_le(char *s1, char *s2) { return strcmp_ci(s1, s2) <= 0; }
int string_ci_lt(char *s1, char *s2) { return strcmp_ci(s1, s2) <  0; }
int string_ci_eq(char *s1, char *s2) { return strcmp_ci(s1, s2) == 0; }
int string_ci_ge(char *s1, char *s2) { return strcmp_ci(s1, s2) >= 0; }
int string_ci_gt(char *s1, char *s2) { return strcmp_ci(s1, s2) >  0; }

int string_le(char *s1, char *s2) { return strcmp(s1, s2) <= 0; }
int string_lt(char *s1, char *s2) { return strcmp(s1, s2) <  0; }
int string_eq(char *s1, char *s2) { return strcmp(s1, s2) == 0; }
int string_ge(char *s1, char *s2) { return strcmp(s1, s2) >= 0; }
int string_gt(char *s1, char *s2) { return strcmp(s1, s2) >  0; }

int string_predicate(char *name, int (*p)(char *s1, char *s2), int x) {
	char	msg[100];

	x = Cdr[x];
	while (Cdr[x] != NIL) {
		if (!string_p(cadr(x))) {
			sprintf(msg, "%s: expected string, got", name);
			return error(msg, cadr(x));
		}
		if (!p(string(Car[x]), string(cadr(x))))
			return FALSE;
		x = Cdr[x];
	}
	return TRUE;
}

#define SP return string_predicate
int pp_string_ci_le_p(int x) { SP("string-ci<=?", string_ci_le, x); }
int pp_string_ci_lt_p(int x) { SP("string-ci<?",  string_ci_lt, x); }
int pp_string_ci_eq_p(int x) { SP("string-ci=?",  string_ci_eq, x); }
int pp_string_ci_ge_p(int x) { SP("string-ci>=?", string_ci_ge, x); }
int pp_string_ci_gt_p(int x) { SP("string-ci>?",  string_ci_gt, x); }

int pp_string_le_p(int x) { SP("string<=?", string_le, x); }
int pp_string_lt_p(int x) { SP("string<?",  string_lt, x); }
int pp_string_eq_p(int x) { SP("string=?",  string_eq, x); }
int pp_string_ge_p(int x) { SP("string>=?", string_ge, x); }
int pp_string_gt_p(int x) { SP("string>?",  string_gt, x); }

int pp_string_p(int x) {
	return string_p(cadr(x))? TRUE: FALSE;
}

int pp_symbol_to_string(int x) {
	return clone_string(cadr(x), (int) strlen(string(cadr(x))));
}

int pp_symbol_p(int x) {
	return symbol_p(cadr(x))? TRUE: FALSE;
}

int pp_syntax_to_list(int x) {
	int	n;

	n = cadr(x);
	if (symbol_p(n)) n = lookup(n, Environment);
	if (n == NIL) return FALSE;
	n = cadr(n);
	if (!syntax_p(n)) return FALSE;
	return Cdr[n];
}

int pp_times(int x) {
	int	a;

	x = Cdr[x];
	if (Cdr[x] == NIL) return Car[x];
	a = make_integer(1);
	save(a);
	while (x != NIL) {
		if (!integer_p(Car[x]))
			return error("+: expected integer, got", Car[x]);
		a = bignum_multiply(a, Car[x]);
		Car[Stack] = a;
		x = Cdr[x];
	}
	unsave(1);
	return a;
}

int pp_unquote(int x) {
	return error("unquote: not in quasiquote context", NOEXPR);
}

int pp_unquote_splicing(int x) {
	return error("unquote-splicing: not in quasiquote context", NOEXPR);
}

int pp_vector_to_list(int x) {
	int	*v, n, a, k, i, new;

	v = vector(cadr(x));
	k = vector_len(cadr(x));
	n = NIL;
	a = NIL;
	for (i=0; i<k; i++) {
		if (n == NIL) {
			n = a = alloc(v[i], NIL);
			save(n);
		}
		else {
			new = alloc(v[i], NIL);
			Cdr[a] = new;
			a = Cdr[a];
		}
	}
	if (n != NIL) unsave(1);
	return n;
}

int pp_vector_fill_b(int x) {
	int	fill = caddr(x),
		i, k = vector_len(cadr(x)),
		*v = vector(cadr(x));

	for (i=0; i<k; i++) v[i] = fill;
	return UNSPECIFIC;
}

int pp_vector_length(int x) {
	return make_integer(vector_len(cadr(x)));
}

int pp_vector_ref(int x) {
	int	p, k = vector_len(cadr(x));

	p = integer_value("vector-ref", caddr(x));
	if (p < 0 || p >= k)
		return error("vector-ref: index out of range",
				caddr(x));
	return vector(cadr(x))[p];
}

int pp_vector_set_b(int x) {
	int	p, k = vector_len(cadr(x));

	p = integer_value("vector-set!", caddr(x));
	if (p < 0 || p >= k)
		return error("vector-set!: index out of range",
				caddr(x));
	vector(cadr(x))[p] = cadddr(x);
	return UNSPECIFIC;
}

int pp_vector_p(int x) {
	return vector_p(cadr(x))? TRUE: FALSE;
}

int pp_write(int x) {
	int	new_port, old_port;

	new_port = cddr(x) == NIL? Output_port: port_no(caddr(x));
	if (new_port < 0 || new_port >= MAX_PORTS)
		return error("bad output port", caddr(x));
	old_port = Output_port;
	Output_port = new_port;
	print(cadr(x));
	Output_port = old_port;
	return UNSPECIFIC;
}

int pp_write_char(int x) {
	return pp_display(x);
}

int pp_wrong(int x) {
	return error(string(cadr(x)), length(x) > 2? caddr(x): NOEXPR);
}

/*----- Evaluator -----*/

enum TYPES {
	T_NONE,
	T_BOOLEAN,
	T_CHAR,
	T_INPUT_PORT,
	T_INTEGER,
	T_OUTPUT_PORT,
	T_PAIR,
	T_PAIR_OR_NIL,
	T_PROCEDURE,
	T_STRING,
	T_SYMBOL,
	T_VECTOR
};

struct Primitive_procedure {
	int	(*handler)(int expr);
	int	min_args;
	int	max_args;	/* -1 = variadic */
	int	arg_types[3];
};

enum PRIMITIVES {
	PP_APPLY, PP_BOOLEAN_P, PP_CAR, PP_CDR, PP_CHAR_P,
	PP_CHAR_TO_INTEGER, PP_CHAR_ALPHABETIC_P, PP_CHAR_CI_LE_P,
	PP_CHAR_CI_LT_P, PP_CHAR_CI_EQ_P, PP_CHAR_CI_GE_P,
	PP_CHAR_CI_GT_P, PP_CHAR_DOWNCASE, PP_CHAR_LOWER_CASE_P,
	PP_CHAR_NUMERIC_P, PP_CHAR_UPCASE, PP_CHAR_UPPER_CASE_P,
	PP_CHAR_WHITESPACE_P, PP_CHAR_LE_P, PP_CHAR_LT_P, PP_CHAR_EQ_P,
	PP_CHAR_GE_P, PP_CHAR_GT_P, PP_CLOSE_INPUT_PORT,
	PP_CLOSE_OUTPUT_PORT, PP_CONS, PP_CURRENT_INPUT_PORT,
	PP_CURRENT_OUTPUT_PORT, PP_DISPLAY, PP_EOF_OBJECT_P, PP_EQ_P,
	PP_EQUAL, PP_GREATER, PP_GREATER_EQUAL, PP_INPUT_PORT_P,
	PP_INTEGER_P, PP_INTEGER_TO_CHAR, PP_LESS, PP_LESS_EQUAL,
	PP_LIST_TO_STRING, PP_LIST_TO_VECTOR, PP_LOAD, PP_MAKE_STRING,
	PP_MAKE_VECTOR, PP_MINUS, PP_OPEN_INPUT_FILE, PP_OPEN_OUTPUT_FILE,
	PP_OUTPUT_PORT_P, PP_PAIR_P, PP_PEEK_CHAR, PP_PLUS, PP_PROCEDURE_P,
	PP_QUOTIENT, PP_READ, PP_READ_CHAR, PP_REMAINDER, PP_SET_CAR_B,
	PP_SET_CDR_B, PP_SET_INPUT_PORT_B, PP_SET_OUTPUT_PORT_B,
	PP_STRING_TO_LIST, PP_STRING_TO_SYMBOL, PP_STRING_APPEND,
	PP_STRING_COPY, PP_STRING_FILL_B, PP_STRING_LENGTH, PP_STRING_REF,
	PP_STRING_SET_B, PP_STRING_CI_LE_P, PP_STRING_CI_LT_P,
	PP_STRING_CI_EQ_P, PP_STRING_CI_GE_P, PP_STRING_CI_GT_P,
	PP_STRING_LE_P, PP_STRING_LT_P, PP_STRING_EQ_P, PP_STRING_GE_P,
	PP_STRING_GT_P, PP_STRING_P, PP_SUBSTRING, PP_SYMBOL_P,
	PP_SYMBOL_TO_STRING, PP_SYNTAX_TO_LIST, PP_TIMES, PP_UNQUOTE,
	PP_UNQUOTE_SPLICING, PP_VECTOR_FILL_B, PP_VECTOR_LENGTH,
	PP_VECTOR_SET_B, PP_VECTOR_REF, PP_VECTOR_TO_LIST, PP_VECTOR_P,
	PP_WRITE, PP_WRITE_CHAR, PP_WRONG
};

/*
 * Above enum must reflect the order of the array below!
 */

struct Primitive_procedure Primitives[] = {
 { pp_apply,               2, -1, { T_PROCEDURE,  T_NONE,       T_NONE } },
 { pp_boolean_p,           1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_car,                 1,  1, { T_PAIR,       T_NONE,       T_NONE } },
 { pp_cdr,                 1,  1, { T_PAIR,       T_NONE,       T_NONE } },
 { pp_char_p,              1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_char_to_integer,     1,  1, { T_CHAR,       T_NONE,       T_NONE } },
 { pp_char_alphabetic_p,   1,  1, { T_CHAR,       T_NONE,       T_NONE } },
 { pp_char_ci_le_p,        2, -1, { T_CHAR,       T_CHAR,       T_NONE } },
 { pp_char_ci_lt_p,        2, -1, { T_CHAR,       T_CHAR,       T_NONE } },
 { pp_char_ci_eq_p,        2, -1, { T_CHAR,       T_CHAR,       T_NONE } },
 { pp_char_ci_ge_p,        2, -1, { T_CHAR,       T_CHAR,       T_NONE } },
 { pp_char_ci_gt_p,        2, -1, { T_CHAR,       T_CHAR,       T_NONE } },
 { pp_char_downcase,       1,  1, { T_CHAR,       T_NONE,       T_NONE } },
 { pp_char_lower_case_p,   1,  1, { T_CHAR,       T_NONE,       T_NONE } },
 { pp_char_numeric_p,      1,  1, { T_CHAR,       T_NONE,       T_NONE } },
 { pp_char_upcase,         1,  1, { T_CHAR,       T_NONE,       T_NONE } },
 { pp_char_upper_case_p,   1,  1, { T_CHAR,       T_NONE,       T_NONE } },
 { pp_char_whitespace_p,   1,  1, { T_CHAR,       T_NONE,       T_NONE } },
 { pp_char_le_p,           2, -1, { T_CHAR,       T_CHAR,       T_NONE } },
 { pp_char_lt_p,           2, -1, { T_CHAR,       T_CHAR,       T_NONE } },
 { pp_char_eq_p,           2, -1, { T_CHAR,       T_CHAR,       T_NONE } },
 { pp_char_ge_p,           2, -1, { T_CHAR,       T_CHAR,       T_NONE } },
 { pp_char_gt_p,           2, -1, { T_CHAR,       T_CHAR,       T_NONE } },
 { pp_close_input_port,    1,  1, { T_INPUT_PORT, T_NONE,       T_NONE } },
 { pp_close_output_port,   1,  1, { T_OUTPUT_PORT,T_NONE,       T_NONE } },
 { pp_cons,                2,  2, { T_NONE,       T_NONE,       T_NONE } },
 { pp_current_input_port,  0,  0, { T_NONE,       T_NONE,       T_NONE } },
 { pp_current_output_port, 0,  0, { T_NONE,       T_NONE,       T_NONE } },
 { pp_display,             1,  2, { T_NONE,       T_OUTPUT_PORT,T_NONE } },
 { pp_eof_object_p,        1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_eq_p,                2,  2, { T_NONE,       T_NONE,       T_NONE } },
 { pp_equal,               2, -1, { T_INTEGER,    T_INTEGER,    T_NONE } },
 { pp_greater,             2, -1, { T_INTEGER,    T_INTEGER,    T_NONE } },
 { pp_greater_equal,       2, -1, { T_INTEGER,    T_INTEGER,    T_NONE } },
 { pp_input_port_p,        1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_integer_p,           1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_integer_to_char,     1,  1, { T_INTEGER,    T_NONE,       T_NONE } },
 { pp_less,                2, -1, { T_INTEGER,    T_INTEGER,    T_NONE } },
 { pp_less_equal,          2, -1, { T_INTEGER,    T_INTEGER,    T_NONE } },
 { pp_list_to_string,      1,  1, { T_PAIR_OR_NIL,T_NONE,       T_NONE } },
 { pp_list_to_vector,      1,  1, { T_PAIR_OR_NIL,T_NONE,       T_NONE } },
 { pp_load,                1,  1, { T_STRING,     T_NONE,       T_NONE } },
 { pp_make_string,         1,  2, { T_INTEGER,    T_NONE,       T_NONE } },
 { pp_make_vector,         1,  2, { T_INTEGER,    T_NONE,       T_NONE } },
 { pp_minus,               1, -1, { T_INTEGER,    T_NONE,       T_NONE } },
 { pp_open_input_file,     1,  1, { T_STRING,     T_NONE,       T_NONE } },
 { pp_open_output_file,    1,  1, { T_STRING,     T_NONE,       T_NONE } },
 { pp_output_port_p,       1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_pair_p,              1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_peek_char,           0,  1, { T_INPUT_PORT, T_NONE,       T_NONE } },
 { pp_plus,                0, -1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_procedure_p,         1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_quotient,            2,  2, { T_INTEGER,    T_INTEGER,    T_NONE } },
 { pp_read,                0,  1, { T_INPUT_PORT, T_NONE,       T_NONE } },
 { pp_read_char,           0,  1, { T_INPUT_PORT, T_NONE,       T_NONE } },
 { pp_remainder,           2,  2, { T_INTEGER,    T_INTEGER,    T_NONE } },
 { pp_set_car_b,           2,  2, { T_PAIR,       T_NONE,       T_NONE } },
 { pp_set_cdr_b,           2,  2, { T_PAIR,       T_NONE,       T_NONE } },
 { pp_set_input_port_b,    1,  1, { T_INPUT_PORT, T_NONE,       T_NONE } },
 { pp_set_output_port_b,   1,  1, { T_OUTPUT_PORT,T_NONE,       T_NONE } },
 { pp_string_to_list,      1,  1, { T_STRING,     T_NONE,       T_NONE } },
 { pp_string_to_symbol,    1,  1, { T_STRING,     T_NONE,       T_NONE } },
 { pp_string_append,       0, -1, { T_STRING,     T_NONE,       T_NONE } },
 { pp_string_copy,         1,  1, { T_STRING,     T_NONE,       T_NONE } },
 { pp_string_fill_b,       2,  2, { T_STRING,     T_CHAR,       T_NONE } },
 { pp_string_length,       1,  1, { T_STRING,     T_NONE,       T_NONE } },
 { pp_string_ref,          2,  2, { T_STRING,     T_INTEGER,    T_NONE } },
 { pp_string_set_b,        3,  3, { T_STRING,     T_INTEGER,    T_CHAR } },
 { pp_string_ci_le_p,      2, -1, { T_STRING,     T_STRING,     T_NONE } },
 { pp_string_ci_lt_p,      2, -1, { T_STRING,     T_STRING,     T_NONE } },
 { pp_string_ci_eq_p,      2, -1, { T_STRING,     T_STRING,     T_NONE } },
 { pp_string_ci_ge_p,      2, -1, { T_STRING,     T_STRING,     T_NONE } },
 { pp_string_ci_gt_p,      2, -1, { T_STRING,     T_STRING,     T_NONE } },
 { pp_string_le_p,         2, -1, { T_STRING,     T_STRING,     T_NONE } },
 { pp_string_lt_p,         2, -1, { T_STRING,     T_STRING,     T_NONE } },
 { pp_string_eq_p,         2, -1, { T_STRING,     T_STRING,     T_NONE } },
 { pp_string_ge_p,         2, -1, { T_STRING,     T_STRING,     T_NONE } },
 { pp_string_gt_p,         2, -1, { T_STRING,     T_STRING,     T_NONE } },
 { pp_string_p,            1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_substring,           3,  3, { T_STRING,     T_INTEGER,    T_INTEGER } },
 { pp_symbol_p,            1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_symbol_to_string,    1,  1, { T_SYMBOL,     T_NONE,       T_NONE } },
 { pp_syntax_to_list,      1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_times,               0, -1, { T_INTEGER,    T_NONE,       T_NONE } },
 { pp_unquote,             1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_unquote_splicing,    1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_vector_fill_b,       2,  2, { T_VECTOR,     T_NONE,       T_NONE } },
 { pp_vector_length,       1,  1, { T_VECTOR,     T_NONE,       T_NONE } },
 { pp_vector_set_b,        3,  3, { T_VECTOR,     T_INTEGER,    T_NONE } },
 { pp_vector_ref,          2,  2, { T_VECTOR,     T_INTEGER,    T_NONE } },
 { pp_vector_to_list,      1,  1, { T_VECTOR,     T_NONE,       T_NONE } },
 { pp_vector_p,            1,  1, { T_NONE,       T_NONE,       T_NONE } },
 { pp_write,               1,  2, { T_NONE,       T_OUTPUT_PORT,T_NONE } },
 { pp_write_char,          1,  2, { T_CHAR,       T_OUTPUT_PORT,T_NONE } },
 { pp_wrong,               1,  2, { T_STRING,     T_NONE,       T_NONE } },
};

int expected(int who, char *what, int got) {
	char	msg[100];

	sprintf(msg, "%s: expected %s, got",
		string(cddr(who)), what);
	return error(msg, got);
}

int primitive(int x) {
	int	id, n, k, na, i, a;

	id = cadar(x);
	k = length(x);
	if (k-1 < Primitives[id].min_args)
		return too_few_args(x);
	if (k-1 > Primitives[id].max_args && Primitives[id].max_args >= 0)
		return too_many_args(x);
	a = Cdr[x];
	na = Primitives[id].max_args < 0? Primitives[id].min_args:
					Primitives[id].max_args;
        if (na > k-1) na = k-1;
	for (i=1; i<=na; i++) {
		switch (Primitives[id].arg_types[i-1]) {
		case T_NONE:
			break;
		case T_BOOLEAN:
			if (!boolean_p(Car[a]))
				return expected(Car[x], "boolean", Car[a]);
			break;
		case T_CHAR:
			if (!char_p(Car[a]))
				return expected(Car[x], "char", Car[a]);
			break;
		case T_INPUT_PORT:
			if (!input_port_p(Car[a]))
				return expected(Car[x], "input-port", Car[a]);
			break;
		case T_INTEGER:
			if (!integer_p(Car[a]))
				return expected(Car[x], "integer", Car[a]);
			break;
		case T_OUTPUT_PORT:
			if (!output_port_p(Car[a]))
				return expected(Car[x], "output-port", Car[a]);
			break;
		case T_PAIR:
			if (atom_p(Car[a]))
				return expected(Car[x], "pair", Car[a]);
			break;
		case T_PAIR_OR_NIL:
			if (Car[a] != NIL && atom_p(Car[a]))
				return expected(Car[x], "pair or ()", Car[a]);
			break;
		case T_PROCEDURE:
			if (	!procedure_p(Car[a]) &&
				!primitive_p(Car[a])
			)
				return expected(Car[x], "procedure", Car[a]);
			break;
		case T_STRING:
			if (!string_p(Car[a]))
				return expected(Car[x], "string", Car[a]);
			break;
		case T_SYMBOL:
			if (!symbol_p(Car[a]))
				return expected(Car[x], "symbol", Car[a]);
			break;
		case T_VECTOR:
			if (!vector_p(Car[a]))
				return expected(Car[x], "vector", Car[a]);
			break;
		}
		a = Cdr[a];
	}
	n = (*Primitives[id].handler)(x);
	return n;
}

/* Return (#<procedure> (quote #f)) or () */
int make_application(char *proc_name) {
	int	p_sym, p, app;

	p_sym = find_symbol(proc_name);
	if (p_sym == NIL) return NIL;
	p = lookup(p_sym, Environment);
	if (p == NIL) return NIL;
	p = cadr(p);
	app = alloc(FALSE, NIL);
	app = alloc(S_quote, app);
	app = alloc(app, NIL);
	app = alloc(p, app);
	return app;
}

int has_property_p(int (*p)(int x), int x) {
	if (atom_p(x)) return 0;
	if (Car[x] == S_quote) return 0;
	if (p(x)) return 1;
	while (!atom_p(x)) {
		if (has_property_p(p, Car[x])) return 1;
		x = Cdr[x];
	}
	return 0;
}

int syntactic_symbol_p(int x) {
	int	y;

	if (symbol_p(Car[x])) {
		y = lookup(Car[x], Environment);
		if (y != NIL && syntax_p(cadr(y))) return 1;
	}
	return 0;
}

int quasiquotation_p(int x) {
	return Car[x] == S_quasiquote;
}

int uses_transformer_p(int x) {
	return has_property_p(syntactic_symbol_p, x);
}

int uses_quasiquote_p(int x) {
	return has_property_p(quasiquotation_p, x);
}

int _eval(int x);

int expand_qq(int x, int app) {
	int	n, a, new;

	if (Error_flag) return x;
	if (atom_p(x)) return x;
	if (Car[x] == S_quote) return x;
	if (Car[x] == S_quasiquote) {
		cadadr(app) = x;
		return _eval(app);
	}
	n = a = NIL;
	save(n);
	while (!atom_p(x)) {
		if (n == NIL) {
			n = alloc(expand_qq(Car[x], app), NIL);
			Car[Stack] = n;
			a = n;
		}
		else {
			new = alloc(expand_qq(Car[x], app), NIL);
			Cdr[a] = new;
			a = Cdr[a];
		}
		x = Cdr[x];
	}
	Cdr[a] = x;
	unsave(1);
	return n;
}

int expand_quasiquote(int x) {
	int	app;

	if (Error_flag) return x;
	if (atom_p(x)) return x;
	if (!uses_quasiquote_p(x)) return x;
	app = make_application("expand-quasiquote");
	if (app == NIL) return x;
	save(app);
	x = expand_qq(x, app);
	unsave(1);
	return x;
}

int expand_all_syntax(int x, int app) {
	int	y, n, a, new;

	if (Error_flag) return x;
	if (atom_p(x)) return x;
	if (Car[x] == S_quote || Car[x] == S_define_syntax) return x;
	if (symbol_p(Car[x])) {
		y = lookup(Car[x], Environment);
		if (y != NIL && syntax_p(cadr(y))) {
			cadadr(app) = x;
			return _eval(app);
		}
	}
	n = a = NIL;
	save(n);
	while (!atom_p(x)) {
		if (n == NIL) {
			n = alloc(expand_all_syntax(Car[x], app), NIL);
			Car[Stack] = n;
			a = n;
		}
		else {
			new = alloc(expand_all_syntax(Car[x], app), NIL);
			Cdr[a] = new;
			a = Cdr[a];
		}
		x = Cdr[x];
	}
	Cdr[a] = x;
	unsave(1);
	return n;
}

int expand_syntax(int x) {
	int	app;

	if (Error_flag) return x;
	if (atom_p(x)) return x;
	if (Car[x] == S_quote || Car[x] == S_define_syntax) return x;
	if (!uses_transformer_p(x)) return x;
	app = make_application("expand-syntax");
	if (app == NIL) return x;
	save(app);
	x = expand_all_syntax(x, app);
	unsave(1);
	return x;
}

#define save_state(v) (State_stack = alloc3((v), State_stack, AFLAG))

static int restore_state(void) {
	int	v;

	if (State_stack == NIL) fatal("restore_state(): stack underflow");
	v = Car[State_stack];
	State_stack = Cdr[State_stack];
	return v;
}

int bind_arguments(int n, int name) {
	int	p, v, a, e;
	int	rib;

	save(Environment);
	p = Car[n];
	v = cadr(p);
	e = cdddr(p);
	a = Cdr[n];
	if (e != NIL) Environment = e;
	rib = NIL;
	save(rib);
	while (!atom_p(v)) {
		if (atom_p(a)) return too_few_args(n);
		Tmp = alloc(Car[a], NIL);
		Tmp = alloc(Car[v], Tmp);
		rib = alloc(Tmp, rib);
		Car[Stack] = rib;
		v = Cdr[v];
		a = Cdr[a];
	}
	if (symbol_p(v)) {
		Tmp = alloc(a, NIL);
		Tmp = alloc(v, Tmp);
		rib = alloc(Tmp, rib);
		Car[Stack] = rib;
	}
	else if (a != NIL) {
		return too_many_args(n);
	}
	unsave(1);
	Environment = make_env(rib, Environment);
	return UNSPECIFIC;
}

void tail_call(void) {
	if (State_stack == NIL || Car[State_stack] != MBETA) return;
	Tmp = unsave(1);
	Environment = Car[Stack];
	unsave(2);
	restore_state();
	save(Tmp);
	Tmp = NIL;
}

int apply_special(int x, int *pc, int *ps) {
	int	sf;

	sf = Car[x];
	if (sf == S_and) return sf_and(x, pc, ps);
	else if (sf == S_begin) return sf_begin(x, pc, ps);
	else if (sf == S_cond) return sf_cond(x, pc, ps);
	else if (sf == S_define) return sf_define(x, pc, ps);
	else if (sf == S_define_syntax) return sf_define_syntax(x, pc, ps);
	else if (sf == S_if) return sf_if(x, pc, ps);
	else if (sf == S_lambda) return sf_lambda(x);
	else if (sf == S_let) return sf_let(x, pc);
	else if (sf == S_letrec) return sf_letrec(x, pc);
	else if (sf == S_quote) return sf_quote(x);
	else if (sf == S_or) return sf_or(x, pc, ps);
	else if (sf == S_set_b) return sf_set_b(x, pc, ps);
	else if (sf == S_syntax_rules) return sf_syntax_rules(x);
	else fatal("internal: unknown special form");
	return UNSPECIFIC;
}

void make_dynamic(int x) {
	if (procedure_p(x))
		cdddr(x) = NIL; /* clear lexical env. */
}

int _eval(int x) {
	int	m2,	/* Root of result list */
		a,	/* Used to append to result */
		rib;	/* Temp storage for args */
	int	cbn,	/* Use call-by-name in next iteration */
		s,	/* Current state */
		c;	/* Continuation */
	int	name;	/* Name of procedure to apply */
	int	new;

	save(x);
	save(State_stack);
	save(Stack_bottom);
	Stack_bottom = Stack;
	s = MATOM;
	c = 0;
	cbn = 0;
	while (!Error_flag) {
		if (x == NIL) {			/* () -> () */
			/* should catch unquoted () */
			Acc = x;
			cbn = 0;
		}
		else if (auto_quoting_p(x) ||
			procedure_p(x) ||
			primitive_p(x)
		) {
			Acc = x;
			cbn = 0;
		}
		else if (symbol_p(x)) {		/* Symbol -> Value */
			if (cbn) {
				Acc = x;
				cbn = 0;
			}
			else {
				Acc = value_of(x, Environment);
				if (Acc == UNDEFINED)
					error("symbol not bound", x);
				if (Error_flag) break;
			}
		}
		else {				/* (...) -> Value */
			/*
			 * This block is used to DESCEND into lists.
			 * The following structure is saved on the
			 * Stack: RIB = (args append result source)
			 * The current s is saved on the State_stack.
			 */
			Acc = x;
			x = Car[x];
			save_state(s);
			/* Check call-by-name built-ins and flag */
			if (special_p(x) || cbn) {
				cbn = 0;
				rib = alloc(Acc, NIL);	/* source */
				rib = alloc(Acc, rib);	/* result */
				rib = alloc(NIL, rib);	/* append */
				rib = alloc(NIL, rib);	/* args */
				x = NIL;
			}
			else {
				Tmp = alloc(NIL, NIL);
				rib = alloc(Acc, NIL);	/* source */
				rib = alloc(Tmp, rib);	/* result */
				rib = alloc(Tmp, rib);	/* append */
				rib = alloc(Cdr[Acc], rib); /* args */
				Tmp = NIL;
				x = Car[Acc];
			}
			save(rib);
			s = MARGS;
			continue;
		}
		/*
		 * The following loop is used to ASCEND back to the
		 * root of a list, thereby performing BETA REDUCTION.
		 */
		while (1) if (s == MBETA) {
			/* Finish BETA reduction */
			Environment = unsave(1);
			unsave(1);	/* source expression */
			s = restore_state();
		}
		else if (s == MARGS) {	/* Append to list, reduce */
			rib = Car[Stack];
			x = rib_args(rib);
			a = rib_append(rib);
			m2 = rib_result(rib);
			/* Append new member */
			if (a != NIL) Car[a] = Acc;
			if (x == NIL) {		/* End of list */
				Acc = m2;
 				/* Remember name of caller */
				name = Car[rib_source(Car[Stack])];
				/* Save result (new source expression) */
				Car[Stack] = Acc;
				if (primitive_p(Car[Acc])) {
					if (cadar(Acc) == PP_APPLY)
						c = cbn = 1;
					Acc = x = primitive(Acc);
				}
				else if (special_p(Car[Acc])) {
					Acc = x = apply_special(Acc, &c, &s);
				}
				else if (procedure_p(Car[Acc])) {
					name = symbol_p(name)? name: NIL;
					tail_call();
					bind_arguments(Acc, name);
					x = caddar(Acc);
					c = 2;
					s = MBETA;
				}
				else {
					error("application of non-procedure",
						name);
					x = NIL;
				}
				if (c != 2) {
					unsave(1); /* drop source expr */
					s = restore_state();
				}
				/* Leave the ASCENDING loop and descend */
				/* once more into N. */
				if (c) break;
			}
			else if (atom_p(x)) {
				error("improper list in application", x);
				x = NIL;
				break;
			}
			else {		/* N =/= NIL: Append to list */
				/* Create space for next argument */
				new = alloc(NIL, NIL);
				Cdr[a] = new;
				rib_append(rib) = Cdr[a];
				rib_args(rib) = Cdr[x];
				x = Car[x];	/* Evaluate next member */
				break;
			}
		}
		else if (s == MIFPR) {
			x = unsave(1);
			unsave(1);	/* source expression */
			s = restore_state();
			if (Acc != FALSE)
				x = cadr(x);
			else
				x = caddr(x);
			c = 1;
			break;
		}
		else if (s == MCONJ || s == MDISJ) {
			Car[Stack] = cdar(Stack);
			if (	(Acc == FALSE && s == MCONJ) || 
				(Acc != FALSE && s == MDISJ) ||
				Car[Stack] == NIL
			) {
				unsave(2);	/* state, source expr */
				s = restore_state();
				x = Acc;
				cbn = 1;
			}
			else if (cdar(Stack) == NIL) {
				x = caar(Stack);
				unsave(2);	/* state, source expr */
				s = restore_state();
			}
			else {
				x = caar(Stack);
			}
			c = 1;
			break;
		}
		else if (s == MCOND) {
			if (Acc != FALSE) {
				x = cdar(Car[Stack]);
				if (length(x) > 1)
					Acc = x = alloc(S_begin, x);
				else
					x = Car[x];
				unsave(2);	/* state, source expr */
				s = restore_state();
			}
			else if (cdar(Stack) == NIL) {
				unsave(2);	/* state, source expr */
				s = restore_state();
				x = UNSPECIFIC;
			}
			else {
				Car[Stack] = cdar(Stack);
				x = caaar(Stack);
				if (x == S_else && cdar(Stack) == NIL)
					x = TRUE;
			}
			c = 1;
			break;
		}
		else if (s == MBEGN) {
			Car[Stack] = cdar(Stack);
			if (cdar(Stack) == NIL) {
				x = caar(Stack);
				unsave(2);	/* state, source expr */
				s = restore_state();
			}
			else {
				x = caar(Stack);
			}
			c = 1;
			break;
		}
		else if (s == MSETV || s == MDEFN || s == MDSYN) {
			if (s == MDEFN) make_dynamic(Acc);
			if (s == MDSYN && !syntax_p(Acc)) {
				error("define-syntax: expected syntax, got",
					Acc);
				break;
			}
			x = unsave(1);
			unsave(1);	/* source expression */
			s = restore_state();
			Car[x] = Acc;
			Acc = x = UNSPECIFIC;
			c = 0;
			break;
		}
		else { /* s == MATOM */
			break;
		}
		if (c) {	/* Continue evaluation if requested */
			c = 0;
			continue;
		}
		if (Stack == Stack_bottom) break;
	}
	while (Stack != Stack_bottom) unsave(1);
	Stack_bottom = unsave(1);
	State_stack = unsave(1);
	unsave(1);
	return Acc;		/* Return the evaluated expr */
}

int eval(int x) {
	save(x);
	x = expand_quasiquote(x);
	Car[Stack] = x;
	x = expand_syntax(x);
	x = _eval(x);
	unsave(1);
	return x;
}

/*----- REPL -----*/

void clear_local_envs(void) {
	while (Cdr[Environment] != NIL)
		Environment = Cdr[Environment];
}

#ifndef NO_SIGNALS
void keyboard_interrupt(int sig) {
	error("interrupted", NOEXPR);
	signal(SIGINT, keyboard_interrupt);
}

void keyboard_quit(int sig) {
	fatal("received quit signal, exiting");
}
#endif

void repl(void) {
	int	n, sane_env;

	sane_env = alloc(NIL, NIL);
	save(sane_env);
	if (!Quiet_mode) {
		signal(SIGINT, keyboard_interrupt);
		signal(SIGQUIT, keyboard_quit);
	}
	while (1) {
		Error_flag = 0;
		Input_port = 0;
		Output_port = 1;
		clear_local_envs();
		Car[sane_env] = Environment;
		if (!Quiet_mode) pr("> ");
		Program = xread();
		if (Program == ENDOFFILE) break;
		if (!Error_flag) n = eval(Program);
		if (!Error_flag && n != UNSPECIFIC) {
			print(n);
			pr("\n");
			Car[S_latest] = n;
		}
		if (Error_flag) Environment = Car[sane_env];
	}
	unsave(1);
	pr("\n");
}

/*----- Miscellanea -----*/

int make_primitive(char *s, int id) {
	int	n;

	n = add_symbol(s);
	n = alloc3(id, n, AFLAG);
	return alloc3(S_primitive, n, AFLAG);
}

void add_primitive(char *s, int id) {
	int	v;

	v = add_symbol(s);
	Environment = extend(v, make_primitive(s, id), Environment);
}

int make_initial_env(void) {
	Environment = alloc(NIL, NIL);
	Environment = extend(add_symbol("**"), NIL, Environment);
	S_latest = cdadr(Environment);
	add_primitive("*", PP_TIMES);
	add_primitive("+", PP_PLUS);
	add_primitive("-", PP_MINUS);
	add_primitive("<", PP_LESS);
	add_primitive("<=", PP_LESS_EQUAL);
	add_primitive("=", PP_EQUAL);
	add_primitive(">", PP_GREATER);
	add_primitive(">=", PP_GREATER_EQUAL);
	add_primitive("apply", PP_APPLY);
	add_primitive("boolean?", PP_BOOLEAN_P);
	add_primitive("car", PP_CAR);
	add_primitive("cdr", PP_CDR);
	add_primitive("char->integer", PP_CHAR_TO_INTEGER);
	add_primitive("char-alphabetic?", PP_CHAR_ALPHABETIC_P);
	add_primitive("char-ci<=?", PP_CHAR_CI_LE_P);
	add_primitive("char-ci<?", PP_CHAR_CI_LT_P);
	add_primitive("char-ci=?", PP_CHAR_CI_EQ_P);
	add_primitive("char-ci>=?", PP_CHAR_CI_GE_P);
	add_primitive("char-ci>?", PP_CHAR_CI_GT_P);
	add_primitive("char-downcase", PP_CHAR_DOWNCASE);
	add_primitive("char-lower-case?", PP_CHAR_LOWER_CASE_P);
	add_primitive("char-numeric?", PP_CHAR_NUMERIC_P);
	add_primitive("char-upcase", PP_CHAR_UPCASE);
	add_primitive("char-upper-case?", PP_CHAR_UPPER_CASE_P);
	add_primitive("char-whitespace?", PP_CHAR_WHITESPACE_P);
	add_primitive("char<=?", PP_CHAR_LE_P);
	add_primitive("char<?", PP_CHAR_LT_P);
	add_primitive("char=?", PP_CHAR_EQ_P);
	add_primitive("char>=?", PP_CHAR_GE_P);
	add_primitive("char>?", PP_CHAR_GT_P);
	add_primitive("char?", PP_CHAR_P);
	add_primitive("close-input-port", PP_CLOSE_INPUT_PORT);
	add_primitive("close-output-port", PP_CLOSE_OUTPUT_PORT);
	add_primitive("cons", PP_CONS);
	add_primitive("current-input-port", PP_CURRENT_INPUT_PORT);
	add_primitive("current-output-port", PP_CURRENT_OUTPUT_PORT);
	add_primitive("display", PP_DISPLAY);
	add_primitive("eq?", PP_EQ_P);
	add_primitive("eof-object?", PP_EOF_OBJECT_P);
	add_primitive("input-port?", PP_INPUT_PORT_P);
	add_primitive("integer->char", PP_INTEGER_TO_CHAR);
	add_primitive("integer?", PP_INTEGER_P);
	add_primitive("list->string", PP_LIST_TO_STRING);
	add_primitive("list->vector", PP_LIST_TO_VECTOR);
	add_primitive("load", PP_LOAD);
	add_primitive("make-string", PP_MAKE_STRING);
	add_primitive("make-vector", PP_MAKE_VECTOR);
	add_primitive("open-input-file", PP_OPEN_INPUT_FILE);
	add_primitive("open-output-file", PP_OPEN_OUTPUT_FILE);
	add_primitive("output-port?", PP_OUTPUT_PORT_P);
	add_primitive("pair?", PP_PAIR_P);
	add_primitive("peek-char", PP_PEEK_CHAR);
	add_primitive("procedure?", PP_PROCEDURE_P);
	add_primitive("quotient", PP_QUOTIENT);
	add_primitive("read", PP_READ);
	add_primitive("read-char", PP_READ_CHAR);
	add_primitive("remainder", PP_REMAINDER);
	add_primitive("set-car!", PP_SET_CAR_B);
	add_primitive("set-cdr!", PP_SET_CDR_B);
	add_primitive("set-input-port!", PP_SET_INPUT_PORT_B);
	add_primitive("set-output-port!", PP_SET_OUTPUT_PORT_B);
	add_primitive("string->list", PP_STRING_TO_LIST);
	add_primitive("string->symbol", PP_STRING_TO_SYMBOL);
	add_primitive("string-append", PP_STRING_APPEND);
	add_primitive("string-copy", PP_STRING_COPY);
	add_primitive("string-fill!", PP_STRING_FILL_B);
	add_primitive("string-length", PP_STRING_LENGTH);
	add_primitive("string-ref", PP_STRING_REF);
	add_primitive("string-set!", PP_STRING_SET_B);
	add_primitive("string-ci<=?", PP_STRING_CI_LE_P);
	add_primitive("string-ci<?", PP_STRING_CI_LT_P);
	add_primitive("string-ci=?", PP_STRING_CI_EQ_P);
	add_primitive("string-ci>=?", PP_STRING_CI_GE_P);
	add_primitive("string-ci>?", PP_STRING_CI_GT_P);
	add_primitive("string<=?", PP_STRING_LE_P);
	add_primitive("string<?", PP_STRING_LT_P);
	add_primitive("string=?", PP_STRING_EQ_P);
	add_primitive("string>=?", PP_STRING_GE_P);
	add_primitive("string>?", PP_STRING_GT_P);
	add_primitive("string?", PP_STRING_P);
	add_primitive("substring", PP_SUBSTRING);
	add_primitive("symbol->string", PP_SYMBOL_TO_STRING);
	add_primitive("symbol?", PP_SYMBOL_P);
	add_primitive("syntax->list", PP_SYNTAX_TO_LIST);
	add_primitive("unquote", PP_UNQUOTE);
	add_primitive("unquote-splicing", PP_UNQUOTE_SPLICING);
	add_primitive("vector->list", PP_VECTOR_TO_LIST);
	add_primitive("vector-fill!", PP_VECTOR_FILL_B);
	add_primitive("vector-length", PP_VECTOR_LENGTH);
	add_primitive("vector-ref", PP_VECTOR_REF);
	add_primitive("vector-set!", PP_VECTOR_SET_B);
	add_primitive("vector?", PP_VECTOR_P);
	add_primitive("write", PP_WRITE);
	add_primitive("write-char", PP_WRITE_CHAR);
	add_primitive("wrong", PP_WRONG);
	Environment = alloc(Environment, NIL);
	return Environment;
}

void init(void) {
	int	i;

	for (i=2; i<MAX_PORTS; i++) Ports[i] = NULL;
	Ports[0] = stdin;
	Ports[1] = stdout;
	Port_flags[0] = LFLAG;
	Port_flags[1] = LFLAG;
	Input_port = 0;
	Output_port = 1;
	new_segment();
	gc();
	S_char = add_symbol("#<char>");
	S_input_port = add_symbol("#<input-port>");
	S_integer = add_symbol("#<integer>");
	S_output_port = add_symbol("#<output-port>");
	S_primitive = add_symbol("#<primitive>");
	S_procedure = add_symbol("#<procedure>");
	S_string = add_symbol("#<string>");
	S_symbol = add_symbol("#<symbol>");
	S_syntax = add_symbol("#<syntax>");
	S_vector = add_symbol("#<vector>");
	S_else = add_symbol("else");
	S_and = add_symbol("and");
	S_begin = add_symbol("begin");
	S_cond = add_symbol("cond");
	S_define = add_symbol("define");
	S_define_syntax = add_symbol("define-syntax");
	S_if = add_symbol("if");
	S_lambda = add_symbol("lambda");
	S_let = add_symbol("let");
	S_letrec = add_symbol("letrec");
	S_quote = add_symbol("quote");
	S_quasiquote = add_symbol("quasiquote");
	S_unquote = add_symbol("unquote");
	S_unquote_splicing = add_symbol("unquote-splicing");
	S_or = add_symbol("or");
	S_set_b = add_symbol("set!");
	S_syntax_rules = add_symbol("syntax-rules");
	Environment = make_initial_env();
	Program = TRUE;
	rehash(Car[Environment]);
}

void load_library(void) {
	char	*path, buf[100], *p;
	char	libpath[256];
	char	*home;

	path = getenv("S9FES_LIBRARY_PATH");
	home = getenv("HOME");
	if (path == NULL)
		path = strcpy(buf, ".:~/.s9fes:/usr/local/share/s9fes");
	p = strtok(path, ":");
	while (p != NULL) {
		if (p[0] == '~') {
			if (strlen(p) + strlen(home) > 240)
				fatal("path too long in S9FES_LIBRARY_PATH");
			sprintf(libpath, "%s%s/s9.scm", home, &p[1]);
		}
		else {
			if (strlen(p) > 248)
				fatal("path too long in S9FES_LIBRARY_PATH");
			sprintf(libpath, "%s/s9.scm", p);
		}
		if (load(libpath) == 0) {
			/* printf("Library: %s\n", libpath); */
			return;
		}
		p = strtok(NULL, ":");
	}
	fatal("could not load library: \"s9.scm\"");
}

void load_rc(void) {
	char	rcpath[256];
	char	*home;

	home = getenv("HOME");
	if (home == NULL) return;
	if (strlen(home) + 12 >= 256) fatal("path too long in HOME");
	sprintf(rcpath, "%s/.s9fes/rc", home);
	load(rcpath);
}

void usage(void) {
	printf("Usage: s9 [-q] [-v] [-f program]\n");
	exit(1);
}

int main(int argc, char **argv) {
	init();
	argv++;
	load_library();
	while (*argv != NULL) {
		if (**argv != '-') break;
		(*argv)++;
		while (**argv) {
			switch (**argv)  {
			case 'q':
				Quiet_mode = 1;
				(*argv)++;
				break;
			case 'f':
				if (argv[1] == NULL) usage();
				load_rc();
				if (load(argv[1]))
					error("program file not found",
						NOEXPR);
				exit(Error_flag? 1: 0);
				break;
			case 'v':
				printf("Version: %s\n", VERSION);
				exit(1);
				break;
			default:
				usage();
				break;
			}
		}
		argv++;
	}
	if (!Quiet_mode) printf("Scheme 9 from Empty Space"
				" (C) 2007 Nils M Holm\n");
	load_rc();
	repl();
	return 0;
}
