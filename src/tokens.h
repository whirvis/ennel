#ifndef ENNEL_TOKENS_H_
#define ENNEL_TOKENS_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

enum token_type;

/**
 * A symbol token in the Ennel programming language.
 *
 * Symbol tokens are special tokens, which usually take the form of characters
 * that are viewed as symbols in English (among other spoken languages). These
 * symbols are special in that they can "touch" other tokens (that being, not
 * having any whitespace that separates them) and still be parsed. For this
 * reason, reserved words are not considered symbols.
 */
enum ennel_symb;

/**
 * An identifier reserved by the Ennel programming language.
 *
 * Some identifiers for word tokens are reserved by the Ennel, as they serve
 * a special purpouse. As such, they cannot be used for identifiers in other
 * parts of the language. For example, "var" cannot be the name of a variable,
 * as it is used to indicate a new variable in the program.
 */
enum ennel_rsvd;

/**
 * A parsed word token. This may either be a generic identifier, or a keyword
 * reserved by the Ennel programming language.
 */
struct ennel_word;

struct ennel_str;

struct ennel_num;

struct ennel_token;

/**
 * Reads an Ennel token from the file at its current position. If no token can
 * be read, the file will be seeked to its location before this function call.
 *
 * @param f the file to read from.
 */
struct ennel_token
read_token(FILE *f);

#endif /* ENNEL_TOKENS_H_ */
