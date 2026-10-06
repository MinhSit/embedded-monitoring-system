#include "cli.h"

size_t cli_tokenize(char *line, char *argv[], size_t max_args)
{
    size_t argc = 0U;
    size_t i = 0;
    int in_token = 0;
    while(argc < max_args && line[i] != '\0'){
        if(line[i] == ' ' || line[i] == '\t'){
            line[i] = '\0';
            in_token = 0;
        }
        else if (in_token == 0){
            argv[argc++] = &line[i];
            in_token = 1;
        }
        i++;
    }
    return argc;
}

void cli_line_init(cli_line_t *l){
    l->buf[0] = '\0';
    l->len = 0;
}

bool cli_line_feed(cli_line_t *l, char c){
    if(c == '\r' || c == '\n'){
        if(l->len == 0){
            return false;
        }
        else{
            l->buf[l->len] = '\0';
            l->len = 0;
            return true;
        }
    }
    else{
        if(l->len < CLI_LINE_MAX - 1){
            l->buf[l->len++] = c;
            return false;
        }
    }
    return false;
}