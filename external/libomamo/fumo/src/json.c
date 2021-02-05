/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#include "json.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void set(struct jsonObject *jso, const char* key,const char* value)
{
    jso->key = key;
    jso->value = value;
}

int jsonWrapper(char** res_str, struct jsonObject *jso)
{
    if(*res_str == NULL)
    {
        *res_str = (char*)calloc(strlen(jso->key)+strlen(jso->value) + 7, sizeof(char));
        strcpy(*res_str, "\"");
        strcat(*res_str, jso->key);
        strcat(*res_str, "\": ");
        strcat(*res_str, "\"");
        strcat(*res_str, jso->value);
        strcat(*res_str, "\"");
    }
    else
    {
        char *temp_str = (char*)calloc(strlen(*res_str) + strlen(jso->key) + strlen(jso->value) + 9, sizeof(char));
        strcpy(temp_str, *res_str);
        strcat(temp_str, ",\n\"");
        strcat(temp_str, jso->key);
        strcat(temp_str, "\": ");
        strcat(temp_str, "\"");
        strcat(temp_str, jso->value);
        strcat(temp_str, "\"");
        free(*res_str);

        *res_str = (char*)calloc(strlen(temp_str) + 1, sizeof(char));
        strcpy(*res_str, temp_str);
        free(temp_str);
    }
    return 0;
}

int finishJsonObject(char** res_str)
{
    char *temp_str = (char*)calloc(strlen(*res_str) + 5, sizeof(char));
    strcpy(temp_str, "{\n");
    strcat(temp_str, *res_str);
    strcat(temp_str, "\n}");
    free(*res_str);

    *res_str = (char*)calloc(strlen(temp_str) + 1, sizeof(char));
    strcpy(*res_str, temp_str);
    free(temp_str);

    return 0;
}

int finishJsonArray(char** res_str)
{
    char *temp_str = (char*)calloc(strlen(*res_str) + 17, sizeof(char));
    strcpy(temp_str, "\"updateInfo\": [");
    strcat(temp_str, *res_str);
    strcat(temp_str, "]");
    free(*res_str);

    *res_str = (char*)calloc(strlen(temp_str) + 1, sizeof(char));
    strcpy(*res_str, temp_str);
    free(temp_str);

    return 0;
}

int saveJsonObjectToFile(char *path, char* res_str)
{
    FILE *fd;
	remove(path);  //just store one history
    fd = fopen(path, "r");
    if (fd == NULL) {
        finishJsonArray(&res_str);
        finishJsonObject(&res_str);
        fd = fopen(path, "w");
        fprintf(fd, "%s", res_str);
    }
    else
    {
        int i = 0;
        char *string_from_file = (char*)calloc(256, sizeof(char));
        char *general_str = (char*)calloc(4096, sizeof(char));
		memset(general_str, 0, 4096);
        while (feof(fd) == 0)
        {
			i++;
            fgets(string_from_file, 256, fd);
            strncat(general_str, string_from_file, 256);
        }
        if(i == 1)
        {
            finishJsonArray(&res_str);
            finishJsonObject(&res_str);
            fclose(fd);
            fd = fopen(path, "w");
            fprintf(fd, "%s", res_str);
        } else {
            char *general_new_str = (char*)calloc(strlen(general_str) - 5, sizeof(char));
            strcpy(general_new_str, "");
            strncat(general_new_str, general_str, strlen(general_str) - 5);
            fclose(fd);
            fd = fopen(path,"w");
            fprintf(fd, "%s\n}, \n%s]\n}", general_new_str, res_str);
            free(general_new_str);
        }
        free(string_from_file);
        free(general_str);
        free(res_str);
    }

    fclose(fd);

    return 0;
}
