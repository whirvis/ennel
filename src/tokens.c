/* Ennel project */
#include "tokens.h"

/* STD-C libraries */
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_LEN 256
#define STR_LEN  256
#define NUM_LEN   64

#define ename(enum) enum, #enum

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
	/* maths */
	{ ename(SYMB_ADD),     "+", 1 },
	{ ename(SYMB_SUB),     "-", 1 },
	{ ename(SYMB_MUL),     "*", 1 },
	{ ename(SYMB_DIV),     "/", 1 },
	{ ename(SYMB_MOD),     "%", 1 },
	{ ename(SYMB_POW),     "@", 1 },
	{ ename(SYMB_NRT),     "#", 1 },
	{ ename(SYMB_ADD_EQ), "+=", 2 },
	{ ename(SYMB_SUB_EQ), "-=", 2 },
	{ ename(SYMB_MUL_EQ), "*=", 2 },
	{ ename(SYMB_DIV_EQ), "/=", 2 },
	{ ename(SYMB_MOD_EQ), "%=", 2 },
	{ ename(SYMB_POW_EQ), "@=", 2 },
	{ ename(SYMB_NRT_EQ), "#=", 2 },
	
	/* bitwise */
	{ ename(SYMB_BIT_NOT), "~", 1 },
	{ ename(SYMB_BIT_AND), "&", 1 },
	{ ename(SYMB_BIT_OR),  "|", 1 },
	{ ename(SYMB_BIT_XOR), "^", 1 },
	
	/* logical */
	{ ename(SYMB_LGC_NOT),  "!", 1 },
	{ ename(SYMB_LGC_LT),   "<", 1 },
	{ ename(SYMB_LGC_GT),   ">", 1 },
	{ ename(SYMB_LGC_AND), "&&", 2 },
	{ ename(SYMB_LGC_OR),  "||", 2 },
	{ ename(SYMB_LGC_XOR), "^^", 2 },
	{ ename(SYMB_LGC_LTE), "<=", 2 },
	{ ename(SYMB_LGC_GTE), ">=", 2 },
	
	/* grouping */
	{ ename(SYMB_PRNS_OPEN),  "(", 1 },
	{ ename(SYMB_PRNS_CLOSE), ")", 1 },
	{ ename(SYMB_CRLY_OPEN),  "{", 1 },
	{ ename(SYMB_CRLY_CLOSE), "}", 1 },
	{ ename(SYMB_BRKT_OPEN),  "[", 1 },
	{ ename(SYMB_BRKT_CLOSE), "]", 1 },
	
	/* other */
	{ ename(SYMB_EQ),        "=", 1 },
	{ ename(SYMB_DOT),       ".", 1 },
	{ ename(SYMB_COMMA),     ",", 1 },
	{ ename(SYMB_COLON),     ":", 1 },
	{ ename(SYMB_SEMICOLON), ";", 1 },
};

const int SYMB_DESC_AMT = sizeof(symb_descs) / sizeof(struct symb_desc);

struct
rsvd_desc {
	enum ennel_rsvd type;
	char *name;
	char *iden;
	int len;
};

static struct rsvd_desc
rsvd_descs[] = {
	/* declarations */
	{ ename(RSVD_VAR),   "var", 3 },
	{ ename(RSVD_FUNC), "func", 4 },
	
	/* control statements */
	{ ename(RSVD_RETURN), "return", 6 },
};
	
const int RSVD_DESC_AMT = sizeof(rsvd_descs) / sizeof(struct rsvd_desc);

static void
read_symb(FILE *f, enum ennel_symb *symb)
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
		*symb = SYMB_NONE;
		return;
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
	*symb = winner.symb;
}

static void
read_word(FILE *f, enum ennel_rsvd *rsvd, struct ennel_iden *iden, bool *is_word)
{
	char word[WORD_LEN + 1];
	int len = 0;
	while (len < WORD_LEN) {
		char c = fgetc(f);
		if (len == 0 && !isalpha(c) && c != '_') {
			fseek(f, -1, SEEK_CUR);
			break;
		} else if (!isalnum(c) && c != '_') {
			fseek(f, -1, SEEK_CUR);
			break;
		}
		word[len] = c;
		len++;
	}
	word[len] = '\0';
	
	if (len <= 0) {
		*is_word = false;
		return;
	} else {
		*is_word = true;
	}
	
	*rsvd = RSVD_NONE;
	for (int i = 0; i < RSVD_DESC_AMT; i++) {
		struct rsvd_desc cand = rsvd_descs[i];
		if(!strcmp(word, cand.iden)) {
			*rsvd = cand.type;
			break;
		}
	}
	
	if(*rsvd == RSVD_NONE) {
		iden->len = len;
		iden->str = calloc(sizeof(char), len + 1);
		memcpy(iden->str, word, len + 1);
	}
}

static void
read_str(FILE *f, struct ennel_str *str)
{	
	char q = fgetc(f);
	if (q != '\"') {
		fseek(f, -1, SEEK_CUR);
		str->len = -1;
		return;
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
	
	str->len = len;
	str->str = calloc(sizeof(char), len + 1);
	memcpy(str->str, cs, len);
}

static void
read_num(FILE *f, struct ennel_num *num, bool *is_num) {
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
		return;
	}
	
	*is_num = true;
	num->val = atol(ns);
}

void
read_token(FILE *f, struct ennel_token *token)
{
	memset(token, 0x00, sizeof(*token));
	token->type = TOKEN_UNKNOWN;
	
	int c = fgetc(f);
	while (isspace(c)) {
		c = fgetc(f);
	}
	fseek(f, -1, SEEK_CUR);
	
	if (c == EOF) {
		token->type = TOKEN_END;
		return;
	}
	
	enum ennel_symb symb = SYMB_NONE;
	read_symb(f, &symb);
	if (symb != SYMB_NONE) {
		token->type = TOKEN_SYMB;
		token->symb = symb;
		return;
	}
	
	bool is_word;
	enum ennel_rsvd rsvd;
	struct ennel_iden iden;
	read_word(f, &rsvd, &iden, &is_word);
	if (is_word && rsvd != RSVD_NONE) {
		token->type = TOKEN_RSVD;
		token->rsvd = rsvd;
		return;
	} else if (is_word) {
		token->type = TOKEN_IDEN;
		token->iden = iden;
		return;
	}
	
	struct ennel_str str;
	read_str(f, &str);
	if (str.len >= 0) {
		token->type = TOKEN_STR;
		token->str  = str;
		return;
	}
	
	bool is_num = false;
	struct ennel_num num;
	read_num(f, &num, &is_num);
	if (is_num) {
		token->type = TOKEN_NUM;
		token->num = num;
		return;
	}
}
