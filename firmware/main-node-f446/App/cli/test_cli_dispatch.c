#include <stdio.h>
#include <string.h>
#include "cli.h"

static int called_a, called_b;
static size_t got_argc;

static void cmd_a(size_t argc, char *argv[]){ (void)argv; called_a++; got_argc = argc; }
static void cmd_b(size_t argc, char *argv[]){ (void)argc; (void)argv; called_b++; }

static const cli_cmd_t table[] = {
    { "a", cmd_a, "lenh a" },
    { "b", cmd_b, "lenh b" },
};
#define N (sizeof(table) / sizeof(table[0]))

static int fails;
#define CHECK(c) do{ if(!(c)){ printf("FAIL: %s (dong %d)\n", #c, __LINE__); fails++; } }while(0)

int main(void){
    char l1[] = "b";
    char l2[] = "a 1 2";
    char l3[] = "zzz";
    char *argv[CLI_MAX_ARGS];
    size_t argc;

    argc = cli_tokenize(l1, argv, CLI_MAX_ARGS);
    CHECK(cli_dispatch(table, N, argc, argv) == true);
    CHECK(called_b == 1 && called_a == 0);

    argc = cli_tokenize(l2, argv, CLI_MAX_ARGS);
    CHECK(cli_dispatch(table, N, argc, argv) == true);
    CHECK(called_a == 1 && got_argc == 3);

    argc = cli_tokenize(l3, argv, CLI_MAX_ARGS);
    CHECK(cli_dispatch(table, N, argc, argv) == false);
    CHECK(cli_dispatch(table, N, 0, argv) == false);
    CHECK(called_a == 1 && called_b == 1);

    cli_print_help(table, N);

    if(fails == 0){ printf("ALL PASS\n"); }
    return fails;
}