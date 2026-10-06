#ifndef CLI_H
#define CLI_H

#include <stddef.h>
#include <stdbool.h>

#define CLI_MAX_ARGS 4U
#define CLI_LINE_MAX 64U   /* do dai toi da 1 dong, gom ca '\0' */

/* Tach 1 dong thanh cac token (cach nhau bang dau cach), sua truc tiep line.
 * argv[i] tro vao line. Tra ve so token (0 neu dong rong). */
size_t cli_tokenize(char *line, char *argv[], size_t max_args);

/* Bo ghep dong: nhan tung ky tu, gom vao buf cho den khi gap CR hoac LF. */
typedef struct{
    char buf[CLI_LINE_MAX];
    size_t len;            /* so ky tu dang co trong buf */
}cli_line_t;

void cli_line_init(cli_line_t *l);

/* Day 1 ky tu vao. Tra ve true khi vua hoan thanh 1 dong (buf da co '\0').
 * Dong rong (vd chuoi "\r\n" thi ky tu '\n' sau '\r') tra ve false. */
bool cli_line_feed(cli_line_t *l, char c);

#endif