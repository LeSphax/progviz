// json_parser.h
#ifndef PARSER_H
#define PARSER_H

#include <jansson.h>

typedef struct {
    char* source;
    size_t num_dependencies;
    char** dependencies;
} Module;

Module* load_json_file(const char* filename, size_t* num_modules_out);

#endif // PARSER_H
