#ifndef CLI_H
#define CLI_H

#include <stddef.h>

#define CLI_MAX_ARGS 4U

/* Tach 1 dong thanh cac token (cach nhau bang dau cach), sua truc tiep line.
 * argv[i] tro vao line. Tra ve so token (0 neu dong rong). */
size_t cli_tokenize(char *line, char *argv[], size_t max_args);

#endif