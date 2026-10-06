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