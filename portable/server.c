#include "../include/server.h"

int main() {
    // SERVER INFORMATION
    int server_fd;
    struct sockaddr_in server_addr;

    // SOCKET CREATION
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        ERROR_EXIT("Socket creation failed");
    }
    printf("Socket created successfully\n");

    // SOCKET CONFIGURATION
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // BINDING THE SOCKET TO THE PORT
    if (bind(server_fd, (struct sockaddr*)&server_addr, (socklen_t)sizeof(server_addr)) == -1) {
        ERROR_EXIT("Socket binding failed");
    }
    printf("Socket successfully bound to %s:%hu\n\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    // LISTENING FOR CONNECTIONS (max queue size = 10)
    if (listen(server_fd, MAX_QUEUE_SIZE) == -1) {
        ERROR_EXIT("Listen failed");
    }
    printf("Waiting for requests...(<C-c> to stop)\n");

    // as long as the server is running it's accepting new clients
    while (1) {
        // CLIENT INFORMATION
        int* client_fd = malloc(sizeof(int));
        struct sockaddr_in client_addr;
        socklen_t client_addr_length = sizeof(client_addr);

        // ACCEPTING CLIENT CONNECTION
        if ((*client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_length)) == -1) {
            fprintf(stderr, "Client couldn't be accepted");
            free(client_fd);
            continue;
        }

        // PROCESSING CLIENTS REQUEST IN SEPARATE THREAD
        // Considering the fact that HTTP is a stateless protocol we can detach processing of
        // clients from the main thread since the connection breaks when the response is sent,
        // therefore it's memory safe, and network safe (locally)
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, process_client, (void*)client_fd);
        pthread_detach(client_thread);
    }

    return 0;
}

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

char* relative_path(char* file_path) {
    if (*file_path == '/')
        file_path++;

    char *structured_file_path = malloc(19 * sizeof(char) + strlen(file_path));

    snprintf(structured_file_path, 19 * sizeof(char) + strlen(file_path),
            "../../server_data/%s", file_path);

    return structured_file_path;
}

void parse_request(char* buffer, ssize_t buf_len, struct request_line* msg) {
    char *saveptr; // necessary because of thread safety
    msg->method = strtok_r(buffer, " ", &saveptr);
    msg->path = strtok_r(NULL, " ", &saveptr);
    // TOKENIZES THE PROTOCOL VERSION BY THE SLASH, saveptr POINTS TO ONLY THE NUMBERS AFTER IT
    strtok_r(NULL, "/", &saveptr);
    msg->protocol_version = saveptr;

    // ABLE TO IMPLEMENT PARSING OF HEADERS, UNNECESSARY IN OUR CASE
}

//struct message* generate_response(struct message* request)
//char* msg_to_string(struct message* response)

char* generate_response(struct request_line req_line, int status_code, const char* content_type, const char* body) {
    char* response = malloc(sizeof(char) * (HEADER_SIZE_ESTIMATE + strlen(body)));
    snprintf(response, HEADER_SIZE_ESTIMATE + strlen(body),
             "HTTP/%s %d %s\r\n"
             "Content-type: %s; charset=UTF-8\r\n"
             "Content-length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n\0",
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

    char *file_path = relative_path(response.path);
    // OPENING THE FILE
    FILE *searched_file = fopen(file_path, "rb");
    free(file_path);
    if (searched_file == NULL) {
        //FILE NOT FOUND
        response_buf = generate_response(response, 404, "text/html", ERR_404);
        send_response(client_fd, response_buf, buffer, ptr_fd);
        return NULL;
    }

    //READING FILE CONTENT
    fseek(searched_file, 0, SEEK_END);
    long file_byte_count = ftell(searched_file);
    rewind(searched_file);


    //SETTING UP FILE BUFFER FOR SENDING
    char *file_buf = malloc((file_byte_count + 1) * sizeof(char));
    fread(file_buf, sizeof(char), file_byte_count, searched_file);
    file_buf[file_byte_count] = '\0';

    response_buf = generate_response(response, 200, get_mime_type(response.path), file_buf);

    free(file_buf);
    fclose(searched_file);

    send_response(client_fd, response_buf, buffer, ptr_fd);
    return NULL;
}
