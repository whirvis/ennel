#ifndef ENNEL_TOKENS_H_
#define ENNEL_TOKENS_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum
token_type
{
	UNKNOWN, END,
	
	SYMB, /* ennel_symb */
	WORD, /* ennel_word */
	STR,  /* ennel_str  */
	NUM,  /* ennel_num  */
} token_type;

/**
 * A symbol token in the Ennel programming language.
 *
 * Symbol tokens are special tokens, which usually take the form of characters
 * that are viewed as symbols in English (among other spoken languages). These
 * symbols are special in that they can "touch" other tokens (that being, not
 * having any whitespace that separates them) and still be parsed. For this
 * reason, reserved words are not considered symbols.
 */
typedef enum
ennel_symb
{
	NO_SYMB,

	/* maths */
	ADD, ADD_EQ, /* +, += */
	SUB, SUB_EQ, /* -, -= */
	MUL, MUL_EQ, /* *, *= */
	DIV, DIV_EQ, /* /, /= */
	MOD, MOD_EQ, /* %, %= */
	POW, POW_EQ, /* @, @= */
	NRT, NRT_EQ, /* #, #= */
	
	/* bitwise */
	BIT_NOT, BIT_AND, /*  ~, &  */
	BIT_OR,  BIT_XOR, /*  |, ^  */
	
	/* logical */
	LGC_NOT, LGC_AND, /*  !, && */
	LGC_OR,  LGC_XOR, /* ||, ^^ */
	LGC_LT,  LGC_LTE, /*  <, <= */
	LGC_GT,  LGC_GTE, /*  >, >= */
	
	/* grouping */
	PRNS_OPEN, PRNS_CLOSE, /* (, ) */
	CRLY_OPEN, CRLY_CLOSE, /* {, } */
	BRKT_OPEN, BRKT_CLOSE, /* [, ] */
	
	/* comments */
	CMNT_LINE,
	CMNT_MULTI_OPEN,
	CMNT_MULTI_CLOSE,
	
	/* other */
	DOT, COMMA, COLON, SEMICOLON,
} ennel_symb;

/**
 * An identifier reserved by the Ennel programming language.
 *
 * Some identifiers for word tokens are reserved by the Ennel, as they serve
 * a special purpouse. As such, they cannot be used for identifiers in other
 * parts of the language. For example, "var" cannot be the name of a variable,
 * as it is used to indicate a new variable in the program.
 */
typedef enum
ennel_rsvd
{
	UNRESERVED,
	
	/* declarations */
	VAR, FUNC,
	
	/* control statements */
	RETURN,
} ennel_rsvd;

/**
 * A parsed word token. This may either be a generic identifier, or a keyword
 * reserved by the Ennel programming language.
 */
typedef struct
ennel_word
{
	ennel_rsvd type;
	char *iden;
	int len;
} ennel_word;

typedef struct
ennel_str
{
	char *str;
	int len;
} ennel_str;

typedef struct
ennel_num
{
	int64_t val;
} ennel_num;

typedef struct
ennel_token {
	token_type type;
	union {
		ennel_word word;
		ennel_str str;
		ennel_symb symb;
		ennel_num num;
	};
} ennel_token;

/**
 * Reads an Ennel token from the file at its current position. If no token can
 * be read, the file will be seeked to its location before this function call.
 *
 * @param f the file to read from.
 */
ennel_token
read_token(FILE *f);

#endif /* ENNEL_TOKENS_H_ */
