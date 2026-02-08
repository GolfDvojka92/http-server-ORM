#ifndef CLIENT_H
#define CLIENT_H

#include "util.h"
#include "global.h"

#define SERVER_IP_ADDRESS "192.168.100.3"
#define IP_ADDRESS "192.168.1.34"
#define PROTOCOL_VERSION "HTTP/1.1"

char* http_get_request_gen(char* method, char* path);

#endif // !CLIENT_H