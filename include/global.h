#ifndef GLOBAL_H
#define GLOBAL_H

#include "structs.h"

#define PORT 8080
#define DEFAULT_BUFF_SIZE 2048
#define MAX_QUEUE_SIZE 10
#define HEADER_SIZE_ESTIMATE 256

#define ERR_403 "<html><body><h1>403 FORBIDDEN</h1></body></html>"
#define ERR_404 "<html><body><h1>404 NOT FOUND</h1></body></html>"
#define ERR_501 "<html><body><h1>501 NOT IMPLEMENTED</h1></body></html>"

extern const struct mime_map types[];
extern const char* methods[];

#endif // !GLOBAL_H
