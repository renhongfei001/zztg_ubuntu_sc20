/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 */

#ifndef JSON_H
#define JSON_H

struct jsonObject
{
    char* key;
    char* value;
};

void set(struct jsonObject *jso, const char* key,const char* value);

int jsonWrapper(char** res_str, struct jsonObject *jso);

int finishJsonObject(char** res_str);

int finishJsonArray(char** res_str);

int saveJsonObjectToFile(char *path, char* res_str);

#endif
