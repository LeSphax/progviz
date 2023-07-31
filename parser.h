// json_parser.h
#ifndef PARSER_H
#define PARSER_H

#include <jansson.h>

typedef struct
{
    short id;
    std::string source;
    short num_dependencies;
    short *dependencies;
} Module;

Module *load_json_file(const char *filename, short *num_modules_out);

#endif // PARSER_H
