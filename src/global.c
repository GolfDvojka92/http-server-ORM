#include "../include/global.h"

const struct mime_map types[] = {
    {"txt", "text/plain"},
    {"css", "text/css"},
    {"html", "text/html"},
    {"js", "text/javascipt"},
    {"md", "text/markdown"},
    {"csv", "text/csv"},
    {"", "application/octet-stream"}
};

const char* methods[] = {
    "GET",
    "HEAD",
    ""
};

