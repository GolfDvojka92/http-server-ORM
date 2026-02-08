#include "../include/server.h"
#include <stdio.h>

void parse_request(char* buffer, ssize_t buf_len, struct request_line* msg) {
    char *saveptr; // necessary because of thread safety
    msg->method = strtok_r(buffer, " ", &saveptr);
    msg->path = strtok_r(NULL, " ", &saveptr);
    // TOKENIZES THE PROTOCOL VERSION BY THE SLASH, saveptr POINTS TO ONLY THE NUMBERS AFTER IT
    strtok_r(NULL, "/", &saveptr);
    msg->protocol_version = saveptr;

    // ABLE TO IMPLEMENT PARSING OF HEADERS, UNNECESSARY IN OUR CASE
}

char* generate_response(struct request_line req_line, int status_code, const char* content_type, const char* body) {
    char* response = malloc(sizeof(char) * (HEADER_SIZE_ESTIMATE + strlen(body)));
    snprintf(response, HEADER_SIZE_ESTIMATE + strlen(body),
             "HTTP/%s %d %s\r\n"
             "Content-type: %s; charset=UTF-8\r\n"
             "Content-length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n",
             req_line.protocol_version, status_code, get_status_text(status_code),
             content_type,
             strlen(body));
    if (strcmp(req_line.method, "GET") == 0)
        strcat(response, body);
    return response;
}

void send_response(int client_fd, char* response_buf, char* buffer, void* ptr_fd) {
    //SENDING HTTP RESPONSE
    if (send(client_fd, response_buf, strlen(response_buf), 0) == -1)
        printf("Error sending file to client with socket: %d\n", client_fd);

    //CLOSING THE CONNECTION
    printf("Closing connection to client with socket: %d\n", client_fd);
    close(client_fd);
    free(response_buf);
    free(buffer);
    free(ptr_fd);
}

void* process_client(void* ptr_fd) {
    int client_fd = *((int*)ptr_fd);
    char* buffer = malloc(HEADER_SIZE_ESTIMATE * sizeof(char));

    // RECIEVING THE REQUEST FROM CLIENT
    ssize_t bytes_received = recv(client_fd, buffer, HEADER_SIZE_ESTIMATE, 0);
    if (bytes_received == -1) {
        close(client_fd);
        free(ptr_fd);
        free(buffer);
        ERROR_RETURN(NULL, "Error receiving data");
    }
    if (bytes_received == 0) return NULL;

    // PARSING REQUEST LINE
    char* saveptr;
    char* request_line = strtok_r(buffer, "\r\n", &saveptr);        // this saves the rest of the buffer into the saveptr, allows for parsing of the headers in the future

    struct request_line response;
    parse_request(request_line, bytes_received, &response);


    // RESPONSE BUFFER
    char* response_buf;

    // checking if request method is implemented
    for (int i = 0; ;i++) {
        if (strcmp(methods[i], "") == 0) {
            response_buf = generate_response(response, 501, "text/html", ERR_501);
        }
        if (strcmp(methods[i], response.method) == 0) {
            break;
        }
    }

    // DENYING ACCESS TO ROOT
    if (strcmp(response.path, "/") == 0) {
        response_buf = generate_response(response, 403, "text/html", ERR_403);
        send_response(client_fd, response_buf, buffer, ptr_fd);
        return NULL;
    }

    char dir_name[] = "../server_data";
    char* full_path = malloc(sizeof(char) * strlen(dir_name) + strlen(response.path));
    strcat(full_path, dir_name);
    strcat(full_path, response.path);

    // OPENING THE FILE
    FILE *searched_file = fopen(full_path, "rb");
    if (searched_file == NULL) {
        //FILE NOT FOUND
        response_buf = generate_response(response, 404, "text/html", ERR_404);
        free(full_path);
        send_response(client_fd, response_buf, buffer, ptr_fd);
        return NULL;
    }

    free(full_path);

    //READING FILE CONTENT
    fseek(searched_file, 0, SEEK_END);
    long file_byte_count = ftell(searched_file);
    rewind(searched_file);


    //SETTING UP FILE BUFFER FOR SENDING
    char *file_buf = malloc(file_byte_count);
    fread(file_buf, sizeof(char), file_byte_count, searched_file);

    response_buf = generate_response(response, 200, get_mime_type(response.path), file_buf);

    free(file_buf);
    fclose(searched_file);

    send_response(client_fd, response_buf, buffer, ptr_fd);
    return NULL;
}
