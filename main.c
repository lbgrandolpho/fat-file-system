#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fat.h"

#define STR_EQ(x, y) (strcmp(x, y)==0)


int main() {

    char *entrada = (char*) malloc(CLUSTER_SIZE * sizeof(char));
    char *cmd, *arg1, *arg2;

    printf("> ");
    while (fgets(entrada, CLUSTER_SIZE, stdin)) {

        entrada[strlen(entrada)-1] = '\0';

        cmd = strtok(entrada, " ");

        fflush(stdin);
        if (STR_EQ(cmd, "init")) init();
        else if (STR_EQ(cmd, "load")) load();
        else if (STR_EQ(cmd, "mkdir")){
            arg1 = strtok(NULL, " ");
            mkdir(arg1, 1);
        }
        else if (STR_EQ(cmd, "ls")){
            arg1 = strtok(NULL, " ");
            ls(arg1);
        }
        else if (STR_EQ(cmd, "create")){
            arg1 = strtok(NULL, " ");
            create(arg1);
        }
        else if (STR_EQ(cmd, "read")){
            arg1 = strtok(NULL, " ");
            read(arg1);
        }
        else if(STR_EQ(cmd, "unlink")){
            arg1 = strtok(NULL, " ");
            unlink(arg1);
        }
        else if (STR_EQ(cmd, "append")){
            arg2 = strtok(NULL, "\"");
            arg1 = strtok(NULL, " ");
            append(arg1, arg2, 1);
        }
        else if (STR_EQ(cmd, "write")) {
            arg2 = strtok(NULL, "\"");
            arg1 = strtok(NULL, " ");
            write(arg1, arg2);
        }

        else if (STR_EQ(cmd, "quit")) break;

        printf("> ");

    }

    free(entrada);
}