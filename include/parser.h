#ifndef ENNEL_PARSER_H_
#define ENNEL_PARSER_H_

/* Ennel project */
#include "tokens.h"

/* STD-C libraries */
#include <stdio.h>

struct syntax_tree
{
	struct ennel_token token;
	struct syntax_tree *child;
	struct syntax_tree *next_sibling;
};

struct ennel_parser
{
	struct syntax_tree *root;
};

void
parse(FILE *f, struct ennel_parser *parser);

#endif /* ENNEL_PARSER_H_ */
