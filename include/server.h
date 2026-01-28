#ifndef SERVER_H
#define SERVER_H

#include "util.h"
#include "global.h"
#include "helper.h"

void parse_request(char* buffer, ssize_t buf_len, struct request_line* msg);
//struct message* generate_response(struct message* request)
//char* msg_to_string(struct message* response)
char* generate_response(struct request_line req_line, int status_code, const char* content_type, const char* body);
void send_response(int client_fd, char* response_buf, char* buffer, void* ptr_fd);
void* process_client(void* ptr_fd);

#endif // !SERVER_H
