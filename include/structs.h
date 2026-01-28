#ifndef STRUCTS_H
#define STRUCTS_H

struct message {
    struct request_line* req_line;
    // struct map* headers;
    char* body;
};

struct request_line {
    char* method;
    char* path;
    char* protocol_version;
    
};

struct mime_map {
    const char* ext;
    const char* type;
};

#endif
