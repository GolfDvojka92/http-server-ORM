#include "../include/helper.h"

const char* get_status_text(int status_code) {
    switch(status_code) {
        case 200:
            return "OK";
        case 204:
            return "No Content";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 408:
            return "Request Timeout";
        case 501:
            return "Not Implemented";
        default:
            return "";
    }
}

const char* get_mime_type(char *file_name) {
    // FINDS THE LAST INSTACE OF A DOT AND RETURNS A POINTER TO IT
    char *ext = strrchr(file_name, '.') + 1;

    // IF NO POINTS ARE FOUND, IT'S A DEFAULT OCTET STREAM
    if (ext == NULL)
        return "application/octet-stream";

    // O(n) search through the implemented mime types in an array of struct mime_map
    // if not implemented, returns octet-stream
    int idx;
    for (idx = 0; ; idx++) {
        if (strcmp(types[idx].ext, "") == 0) break;
        if (strcmp(types[idx].ext, ext) == 0) break;
    }
    return types[idx].type;
}
