/* Ennel project */
#include "parser.h"
#include "tokens.h"

/* STD-C libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fatal(...)       \
    printf(__VA_ARGS__); \
	exit(0)

struct out_queue {
	struct ennel_token token;
	struct out_queue *next;
};

static struct out_queue*
queue(struct out_queue *q, struct ennel_token token)
{
	struct out_queue *next = malloc(sizeof(*q));
	memset(next, 0x00, sizeof(*next));
	next->token = token;
	
	if (q != NULL) {
		struct out_queue *cur = q;
		while (cur->next != NULL) {
			cur = cur->next;
		}
		cur->next = next;
		return q;
	} else {
		return next;
	}
}

static struct ennel_token
dequeue(struct out_queue **q)
{
    struct ennel_token token = (*q)->token;
    *q = (*q)->next;
    return token;
}

struct op_stack {
	struct ennel_token token;
	struct op_stack *prev;
};

static struct op_stack*
push(struct op_stack *s, struct ennel_token token)
{
	struct op_stack *top = malloc(sizeof(*top));
	top->token = token;
	top->prev = s;
	return top;
}

static struct ennel_token
pop(struct op_stack **s)
{
	struct ennel_token token = (*s)->token;
    *s = (*s)->prev;
    return token;
}

void
parse_expr(FILE *f, struct syntax_tree *tree)
{
	struct out_queue *oq = NULL;
	struct op_stack *os = NULL;
	
	tree = tree;
	bool is_expr = true;
	do {
		int pos = ftell(f);
		struct ennel_token current;
		read_token(f, &current);
		
		switch (current.type) {
		case TOKEN_NUM:
			oq = queue(oq, current);
			break;
		case TOKEN_STR:
			oq = queue(oq, current);
			break;
		case TOKEN_SYMB:
			switch(current.symb) {
			case SYMB_ADD:
				os = push(os, current);
				break;
			default:
				fseek(f, pos, SEEK_SET);
				is_expr = false;
				break;
			}
			break;
		default:
			fseek(f, pos, SEEK_SET);
			is_expr = false;
		}
	} while(is_expr);
	
	while (os != NULL) {
		struct ennel_token cur = pop(&os);
		if(cur.symb == SYMB_ADD) {
			struct ennel_token le = dequeue(&oq);
			struct ennel_token ri = dequeue(&oq);
			printf("ADDING %lld AND %lld!\n", le.num.val, ri.num.val);
		}
	}
}

void
parse_var(FILE *f, struct syntax_tree *tree)
{
	struct ennel_token current;
	struct syntax_tree *sibling;
		
	read_token(f, &current);
	if (current.type != TOKEN_IDEN) {
		fatal("Expected identifier token!\n");
	}
		
	struct syntax_tree *name = malloc(sizeof(*name));
	memset(name, 0x00, sizeof(*name));
	name->token = current;
	sibling = name;
	tree->child = name;
	
	read_token(f, &current);
	if (current.type != TOKEN_SYMB || current.symb != SYMB_COLON) {
		fatal("Expected \':\' after identifier!\n");
	}
	
	read_token(f, &current);
	if (current.type != TOKEN_IDEN) {
		fatal("Expected identifier after \':\'!\n");
	}
	
	struct syntax_tree *type = malloc(sizeof(*type));
	memset(type, 0x00, sizeof(*type));
	type->token = current;
	sibling->next_sibling = type;
	sibling = type;

	read_token(f, &current);
	if (current.type != TOKEN_SYMB || current.symb != SYMB_EQ) {
		fatal("Expected \'=\' after identifier!\n");
	}
	
	struct syntax_tree *value = malloc(sizeof(*value));
	memset(value, 0x00, sizeof(*value));
	parse_expr(f, value);
	sibling->next_sibling = value;
	sibling = value;

	read_token(f, &current);
	if (current.type != TOKEN_SYMB || current.symb != SYMB_SEMICOLON) {
		fatal("Expected \';\' after value!\n");
	}
}	

void
parse_rsvd(FILE *f, struct syntax_tree *tree, struct ennel_token *token)
{
	tree = tree;
	
	enum ennel_rsvd rsvd = token->rsvd;
	printf("\tGot reserved: %d\n", rsvd);
	
	if (rsvd == RSVD_VAR) {
		parse_var(f, tree);
	} else {
		fatal("Unknown reserved!\n");
	}
}

void
parse(FILE *f, struct ennel_parser *parser)
{
	struct syntax_tree *root = malloc(sizeof(*root));
	memset(root, 0x00, sizeof(*root));
	struct syntax_tree *c = parser->root;
	
	bool reached_end = false;
	struct syntax_tree *current = root;
	do {
		struct ennel_token token;
		read_token(f, &token);
		current->token = token;
		
		printf("Got token: %d\n", token.type);
		switch (token.type) {
		case TOKEN_RSVD:
			parse_rsvd(f, current, &token);
			break;
		case TOKEN_END:
			reached_end = true;
			break;
		default:
			fatal("Unknown token type!\n");
			break;
		};
		
		struct syntax_tree *next = malloc(sizeof(*next));
		memset(next, 0x00, sizeof(*next));
		current->next_sibling = next;
	} while (!reached_end);
	
	while (c != NULL) {
		struct ennel_token token = c->token;
		
		printf("Tree type: %d\n", token.type);
		
		if (token.type == TOKEN_RSVD && token.rsvd == RSVD_VAR) {
			struct syntax_tree *child = c->child;
			printf("Variable name: %s\n", child->token.str.str);
		}
		
		c = c->next_sibling;
	}
	
	parser->root = root;
}
