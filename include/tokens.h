#ifndef ENNEL_TOKENS_H_
#define ENNEL_TOKENS_H_

/* C-std library */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

enum token_type
{
	TOKEN_UNKNOWN,
	TOKEN_END,
	
	TOKEN_SYMB, /* ennel_symb */
	TOKEN_RSVD, /* ennel_rsvd */
	TOKEN_IDEN, /* ennel_iden */
	TOKEN_STR,  /* ennel_str  */
	TOKEN_NUM,  /* ennel_num  */
};

/**
 * A symbol token in the Ennel programming language.
 *
 * Symbol tokens are special tokens, which usually take the form of characters
 * that are viewed as symbols in English (among other spoken languages). These
 * symbols are special in that they can "touch" other tokens (that being, not
 * having any whitespace that separates them) and still be parsed. For this
 * reason, reserved words are not considered symbols.
 */
enum ennel_symb
{
	SYMB_NONE,

	/* maths */
	SYMB_ADD, SYMB_ADD_EQ, /* +, += */
	SYMB_SUB, SYMB_SUB_EQ, /* -, -= */
	SYMB_MUL, SYMB_MUL_EQ, /* *, *= */
	SYMB_DIV, SYMB_DIV_EQ, /* /, /= */
	SYMB_MOD, SYMB_MOD_EQ, /* %, %= */
	SYMB_POW, SYMB_POW_EQ, /* @, @= */
	SYMB_NRT, SYMB_NRT_EQ, /* #, #= */
	
	/* bitwise */
	SYMB_BIT_NOT, SYMB_BIT_AND, /*  ~, &  */
	SYMB_BIT_OR,  SYMB_BIT_XOR, /*  |, ^  */
	
	/* logical */
	SYMB_LGC_NOT, SYMB_LGC_AND, /*  !, && */
	SYMB_LGC_OR,  SYMB_LGC_XOR, /* ||, ^^ */
	SYMB_LGC_LT,  SYMB_LGC_LTE, /*  <, <= */
	SYMB_LGC_GT,  SYMB_LGC_GTE, /*  >, >= */
	
	/* grouping */
	SYMB_PRNS_OPEN, SYMB_PRNS_CLOSE, /* (, ) */
	SYMB_CRLY_OPEN, SYMB_CRLY_CLOSE, /* {, } */
	SYMB_BRKT_OPEN, SYMB_BRKT_CLOSE, /* [, ] */
	
	/* other */
	SYMB_EQ, SYMB_DOT, SYMB_COMMA,
	SYMB_COLON, SYMB_SEMICOLON,
};

/**
 * An identifier reserved by the Ennel programming language.
 *
 * Some identifiers for word tokens are reserved by the Ennel, as they serve
 * a special purpouse. As such, they cannot be used for identifiers in other
 * parts of the language. For example, "var" cannot be the name of a variable,
 * as it is used to indicate a new variable in the program.
 */
enum ennel_rsvd
{
	RSVD_NONE,
	
	/* declarations */
	RSVD_VAR, RSVD_FUNC,
	
	/* control statements */
	RSVD_RETURN,
};

struct ennel_iden
{
	int len;
	char *str;
};

struct ennel_str
{
	int len;
	char *str;
};

struct ennel_num
{
	int64_t val;
};

struct ennel_token
{
	enum token_type type;
	union {
		enum ennel_symb symb;
		enum ennel_rsvd rsvd;
		struct ennel_iden iden;
		struct ennel_str str;
		struct ennel_num num;
	};
};

/**
 * Reads an Ennel token from the file at its current position. If no token can
 * be read, the file will be seeked to its location before this function call.
 *
 * @param f the file to read from.
 */
void
read_token(FILE *f, struct ennel_token *token);

#endif /* ENNEL_TOKENS_H_ */
