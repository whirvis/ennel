#include "tokens.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_LEN 256
#define STR_LEN  256
#define NUM_LEN   64

#define ename(enum) enum, #enum

enum token_type
{
	UNKNOWN, END,
	
	SYMB, /* ennel_symb */
	WORD, /* ennel_word */
	STR,  /* ennel_str  */
	NUM,  /* ennel_num  */
} token_type;

enum ennel_symb
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
};

enum ennel_rsvd
{
	UNRESERVED,
	
	/* declarations */
	VAR, FUNC,
	
	/* control statements */
	RETURN,
};

struct ennel_word
{
	enum ennel_rsvd type;
	char *iden;
	int len;
};

struct ennel_str
{
	char *str;
	int len;
};

struct ennel_num
{
	int64_t val;
};

struct ennel_token
{
	enum token_type type;
	union {
		struct ennel_word word;
		struct ennel_str str;
		enum ennel_symb symb;
		struct ennel_num num;
	};
};

struct
symb_desc
{
	enum ennel_symb symb;
	char *name;
	char *symbol;
	int len;
};

static struct symb_desc
symb_descs[] = {
	{ ename(NO_SYMB), NULL, 0 },
	
	/* maths */
	{ ename(ADD),     "+", 1 },
	{ ename(SUB),     "-", 1 },
	{ ename(MUL),     "*", 1 },
	{ ename(DIV),     "/", 1 },
	{ ename(MOD),     "%", 1 },
	{ ename(POW),     "@", 1 },
	{ ename(NRT),     "#", 1 },
	{ ename(ADD_EQ), "+=", 2 },
	{ ename(SUB_EQ), "-=", 2 },
	{ ename(MUL_EQ), "*=", 2 },
	{ ename(DIV_EQ), "/=", 2 },
	{ ename(MOD_EQ), "%=", 2 },
	{ ename(POW_EQ), "@=", 2 },
	{ ename(NRT_EQ), "#=", 2 },
	
	/* bitwise */
	{ ename(BIT_NOT), "~", 1 },
	{ ename(BIT_AND), "&", 1 },
	{ ename(BIT_OR),  "|", 1 },
	{ ename(BIT_XOR), "^", 1 },
	
	/* logical */
	{ ename(LGC_NOT),  "!", 1 },
	{ ename(LGC_LT),   "<", 1 },
	{ ename(LGC_GT),   ">", 1 },
	{ ename(LGC_AND), "&&", 2 },
	{ ename(LGC_OR),  "||", 2 },
	{ ename(LGC_XOR), "^^", 2 },
	{ ename(LGC_LTE), "<=", 2 },
	{ ename(LGC_GTE), ">=", 2 },
	
	/* grouping */
	{ ename(PRNS_OPEN),  "(", 1 },
	{ ename(PRNS_CLOSE), ")", 1 },
	{ ename(CRLY_OPEN),  "{", 1 },
	{ ename(CRLY_CLOSE), "}", 1 },
	{ ename(BRKT_OPEN),  "[", 1 },
	{ ename(BRKT_CLOSE), "]", 1 },
	
	/* comments */
	{ ename(CMNT_LINE),        "//", 1 },
	{ ename(CMNT_MULTI_OPEN),  "/*", 2 },
	{ ename(CMNT_MULTI_CLOSE), "*/", 2 },
	
	/* other */
	{ ename(DOT),       ".", 1 },
	{ ename(COMMA),     ",", 1 },
	{ ename(COLON),     ":", 1 },
	{ ename(SEMICOLON), ";", 1 },
};

const int SYMB_DESC_AMT = sizeof(symb_descs) / sizeof(struct symb_desc);

struct
rsvd_word {
	enum ennel_rsvd type;
	char *name;
	char *iden;
	int len;
};

static struct rsvd_word
rsvd_words[] = {
	{ ename(UNRESERVED), NULL, 0 },
	
	/* declarations */
	{ ename(VAR),   "var", 3 },
	{ ename(FUNC), "func", 4 },
	
	/* control statements */
	{ ename(RETURN), "return", 6 },
};
	
const int WORD_TOKEN_AMT = sizeof(rsvd_words) / sizeof(struct rsvd_word);

static enum ennel_symb
read_symb(FILE *f)
{
	int loc = ftell(f), cand_amt = 0;
	struct symb_desc cands[SYMB_DESC_AMT];

	for (int ci = 0; ci < SYMB_DESC_AMT; ci++) {	
		struct symb_desc cand = symb_descs[ci];
		
		/*
		 * Read the symbol text, with the length being no longer than that of
		 * the current candidate symbol text. It may seem redundant, but this
		 * allows us to easily find all symbol tokens that could match this
		 * text. The longest token will be selected as the winner.
		 *
		 * This system allows us to have tokens that are similar, or even near
		 * identical, but completely different in nature. For example, the "&"
		 * token for bitwise, and the "&&" token for logical.
		 */
		char symbol[cand.len + 1];
		int si = 0;
		while (si < cand.len) {
			char c = fgetc(f);
			if (c == EOF) {
				break;
			}
			symbol[si] = c;
			si++;
		}
		symbol[si] = '\0';
		
		/*
		 * Seek back to the original location in the file. This is required for
		 * the other candidate symbols read the  symbol text and compare it to
		 * their own. This also  leaves the cursor in the correct position if
		 * none of the symbols are candidates for the text just read.
		 */
		fseek(f, loc, SEEK_SET);
		
		/*
		 * If the symbol text just read matches the symbol of our candidate
		 * symbol token, then add it as a possible match. Later, the match
		 * with the longest symbol text will be selected as the winner.
		 */
		if (!strcmp(symbol, cand.symbol)) {
			cands[cand_amt] = cand;
			cand_amt++;
		}
	}
	
	if (cand_amt <= 0) {
		return NO_SYMB;
	}
	
	struct symb_desc winner = cands[0];
	for (int i = 1; i < cand_amt; i++) {
		struct symb_desc cand = cands[i];
		if(cand.len > winner.len) {
			winner = cand;
		}
	}
	
	/*
	 * Seek over the length of the matched token. This is requied to prevent it
	 * from being read a second time.
	 */
	fseek(f, winner.len, SEEK_CUR);
	return winner.symb;
}

static struct ennel_word
read_word(FILE *f, bool *is_word)
{
	struct ennel_word word;
	word.type = UNRESERVED;
	
	char iden[WORD_LEN + 1];
	int len = 0;
	while (len < WORD_LEN) {
		char c = fgetc(f);
		if(!isalnum(c)) {
			fseek(f, -1, SEEK_CUR);
			break;
		}
		iden[len] = c;
		len++;
	}
	iden[len] = '\0';
	
	if (len <= 0) {
		*is_word = false;
		return word;
	} else {
		*is_word = true;
	}
	
	for (int i = 0; i < WORD_TOKEN_AMT; i++) {
		struct rsvd_word cand = rsvd_words[i];
		if(!strcmp(iden, cand.iden)) {
			word.type = cand.type;
			break;
		}
	}
	
	if (word.type != UNRESERVED) {
		word.iden = calloc(sizeof(char), len + 1);
		memcpy(word.iden, iden, len);
		word.len = len;
	}
	
	return word;
}

static struct ennel_str
read_str(FILE *f)
{
	struct ennel_str str;
	
	char q = fgetc(f);
	if (q != '\"') {
		fseek(f, -1, SEEK_CUR);
		str.len = -1;
		return str;
	}
	
	char cs[STR_LEN + 1];
	int len = 0;
	while (len < STR_LEN) {
		char c = fgetc(f);
		if(c == '\"') {
			break;
		}
		cs[len] = c;
		len++;
	}
	cs[len] = '\0';
	
	str.str = calloc(sizeof(char), len + 1);
	memcpy(str.str, cs, len);
	str.len = len;
	
	return str;
}

static struct ennel_num
read_num(FILE *f, bool *is_num) {
	
	struct ennel_num num;
	num.val = 0;
	
	char ns[NUM_LEN + 1];
	int len = 0;
	while (len < NUM_LEN) {
		char c = fgetc(f);
		if(!isdigit(c)) {
			fseek(f, -1, SEEK_CUR);
			break;
		}
		ns[len] = c;
		len++;
	}
	ns[len] = '\0';
	
	if(len <= 0) {
		*is_num = false;
		return num;
	} else {
		*is_num = true;
	}
	
	num.val = atol(ns);
	return num;
}

struct ennel_token
read_token(FILE *f)
{
	struct ennel_token token;
	token.type = UNKNOWN;
	
	int c = fgetc(f);
	while(isspace(c)) {
		c = fgetc(f);
	}
	fseek(f, -1, SEEK_CUR);
	
	if(c == EOF) {
		token.type = END;
		return token;
	}
	
	enum ennel_symb symb = read_symb(f);
	if (symb != NO_SYMB) {
		token.type = SYMB;
		token.symb = symb;
		return token;
	}
	
	struct ennel_str str = read_str(f);
	if (str.len >= 0) {
		token.type = STR;
		token.str  = str;
		return token;
	}
		
	bool is_num = false;
	struct ennel_num num = read_num(f, &is_num);
	if (is_num) {
		token.type = NUM;
		token.num = num;
		return token;
	}
	
	bool is_word = false;
	struct ennel_word word = read_word(f, &is_word);
	if (is_word) {
		token.type = WORD;
		token.word = word;
		return token;
	}
	
	return token;
}
