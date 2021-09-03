/* Ennel project */
#include "parser.h"
#include "tokens.h"

/* STD-C libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fatal(...)       \
    printf(__VA_ARGS__); \
	exit(EXIT_FAILURE)

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fatal("Error: expecting source file\n");
	}
	
	struct ennel_parser parser;
	memset(&parser, 0x00, sizeof(parser));
	
	for(int i = 1; i < argc; i++) {
		char *path = argv[i];
		FILE *f = fopen(path, "r");
		if (f == NULL) {
			fatal("Error: no such file %s\n", path);
		}
		parse(f, &parser);
	}
	return 0;
}
