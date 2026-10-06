#include <stdio.h>
#include <string.h>
#include "cli.h"

static int fail_cnt = 0;

static void check(int cond, const char *name)
{
    if(cond){
        printf("PASS: %s\n", name);
    }
    else{
        printf("FAIL: %s\n", name);
        fail_cnt++;
    }
}

int main(void)
{
    char *argv[CLI_MAX_ARGS];
    size_t n;

    char l1[] = "status";
    n = cli_tokenize(l1, argv, CLI_MAX_ARGS);
    check(n == 1U && strcmp(argv[0], "status") == 0, "mot tu");

    char l2[] = "  dump   0x1000\t5 ";
    n = cli_tokenize(l2, argv, CLI_MAX_ARGS);
    check(n == 3U && strcmp(argv[0], "dump") == 0 && strcmp(argv[1], "0x1000") == 0 && strcmp(argv[2], "5") == 0, "nhieu dau cach + tab");

    char l3[] = "   ";
    n = cli_tokenize(l3, argv, CLI_MAX_ARGS);
    check(n == 0U, "dong chi co dau cach");

    return fail_cnt;
}