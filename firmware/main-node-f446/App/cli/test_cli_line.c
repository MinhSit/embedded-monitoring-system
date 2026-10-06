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
    cli_line_t line;
    cli_line_init(&line);

    check(cli_line_feed(&line, 'a') == false, "'a' chua het dong");
    check(cli_line_feed(&line, 'b') == false, "'b' chua het dong");
    check(cli_line_feed(&line, '\r') == true, "CR ket thuc dong");
    check(strcmp(line.buf, "ab") == 0, "buf = \"ab\"");
    check(cli_line_feed(&line, '\n') == false, "LF sau CR bi bo qua");

    check(cli_line_feed(&line, '\n') == false, "dong rong khong bao true");

    for(int i = 0; i < 70; i++){
        cli_line_feed(&line, 'x');
    }
    check(cli_line_feed(&line, '\r') == true, "buf day van ket thuc duoc dong");
    check(strlen(line.buf) == CLI_LINE_MAX - 1U, "dong dai bi cat con 63 ky tu");

    check(cli_line_feed(&line, 'z') == false, "dong moi sau dong dai: 'z'");
    check(cli_line_feed(&line, '\n') == true, "dong moi sau dong dai: LF");
    check(strcmp(line.buf, "z") == 0, "buf = \"z\"");

    printf("%s (%d fail)\n", fail_cnt == 0 ? "ALL PASS" : "FAILED", fail_cnt);
    return fail_cnt;
}