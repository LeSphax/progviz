#include <stdio.h>
#include <jansson.h>
#include <string.h>
#include "parser.h"

Module* load_json_file(const char* filename, size_t* num_modules_out) {
    json_error_t error;
    json_t *root = json_load_file("file.json", 0, &error);

    if (!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
    }

    if(!json_is_array(root)) {
        fprintf(stderr, "error: root is not an array\n");
        json_decref(root);
        return NULL;
    }

    size_t num_modules = json_array_size(root);
    Module* modules = malloc(num_modules * sizeof(Module));

    for(size_t i = 0; i < num_modules; ++i) {
        json_t *module_json, *source, *dependencies;
        module_json = json_array_get(root, i);

        if(!json_is_object(module_json)) {
            fprintf(stderr, "error: module %zu is not an object\n", i + 1);
            json_decref(root);
            return NULL;
        }

        source = json_object_get(module_json, "source");
        if(!json_is_string(source)) {
            fprintf(stderr, "error: module %zu source is not a string\n", i + 1);
            json_decref(root);
            return NULL;
        }

        dependencies = json_object_get(module_json, "dependencies");
        if(!json_is_array(dependencies)) {
            fprintf(stderr, "error: module %zu dependencies is not an array\n", i + 1);
            json_decref(root);
            return NULL;
        }

        modules[i].source = strdup(json_string_value(source));
        modules[i].num_dependencies = json_array_size(dependencies);
        modules[i].dependencies = malloc(modules[i].num_dependencies * sizeof(char*));
        for(size_t j = 0; j < modules[i].num_dependencies; ++j) {
            json_t* dependency = json_array_get(dependencies, j);
            if(!json_is_string(dependency)) {
                fprintf(stderr, "error: module %zu dependency %zu is not a string\n", i + 1, j + 1);
                json_decref(root);
                return NULL;
            }

            modules[i].dependencies[j] = strdup(json_string_value(dependency));
        }
    }

    json_decref(root);

    return modules;
}
