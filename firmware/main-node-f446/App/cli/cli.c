#include "cli.h"
#include <string.h>
#include <stdio.h>

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

bool cli_dispatch(const cli_cmd_t *table, size_t n, size_t argc, char *argv[]){
    if(argc == 0){
        return false;
    }
    for(size_t i = 0; i < n; i++){
        if(strcmp(table[i].name, argv[0]) == 0){
            table[i].handler(argc, argv);
            return true;
        }
    }
    return false;
}

void cli_print_help(const cli_cmd_t *table, size_t n){
    for(size_t i = 0; i < n; i++){
        printf("%s - %s\r\n", table[i].name, table[i].help);
    }
}