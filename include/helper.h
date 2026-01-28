#ifndef HELPER_H
#define HELPER_H

#include "util.h"
#include "global.h"

// helper funcion declarations
const char* get_status_text(int status_code);
const char* get_mime_type(char *file_name);
char* relative_path(char* file_path);

#endif // !HELPER_H
