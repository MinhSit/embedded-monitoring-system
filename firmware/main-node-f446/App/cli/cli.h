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

/* Ham xu ly 1 lenh: argv[0] la ten lenh, argc la so token. */
typedef void (*cli_handler_t)(size_t argc, char *argv[]);

typedef struct{
    const char *name;      /* ten lenh go vao, vd "help" */
    cli_handler_t handler; /* ham chay khi gap lenh */
    const char *help;      /* mo ta 1 dong, in ra boi lenh help */
}cli_cmd_t;

/* Tra argv[0] trong table (n phan tu). Gap lenh thi goi handler, tra true.
 * argc == 0 hoac khong tim thay thi tra false. */
bool cli_dispatch(const cli_cmd_t *table, size_t n, size_t argc, char *argv[]);

/* In moi lenh trong table, moi lenh 1 dong: "name - help\r\n" (dung printf). */
void cli_print_help(const cli_cmd_t *table, size_t n);

#endif