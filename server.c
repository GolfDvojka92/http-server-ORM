#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define IP_ADDRESS "192.168.100.3"
#define PORT 8080
#define DEFAULT_BUFF_SIZE 2048
#define MAX_QUEUE_SIZE 10
#define HEADER_SIZE_ESTIMATE 256

enum FILE_CODE {TXT, HTML, CSS, JS, CSV, MD};

struct message_details {
    char* method;
    char* path;
    char* protocol_version;
    char* mime_type;
};

void error(void* msg) {
    perror((char*)msg);
    exit(EXIT_FAILURE);
}

void parseRequest(char* buffer, ssize_t buf_len, char** method, char** path, char** protocol_version) {
    char *saveptr; // necessary because of thread safety
    *method = strtok_r(buffer, " ", &saveptr);
    *path = strtok_r(NULL, " ", &saveptr);
    *protocol_version = strtok_r(NULL, "\r\n", &saveptr);

    // ABLE TO IMPLEMENT PARSING OF HEADERS, UNNECESSARY IN OUR CASE
}

const char* get_status_text(int status_code) {
    switch(status_code) {
        case 200:
            return "OK";
        break;
        case 400:
            return "Bad Request";
        break;
        case 401:
            return "Unauthorized";
        break;
        case 403:
            return "Forbidden";
        break;
        case 408:
            return "Request Timeout";
        break;
        case 501:
            return "Not Implemented";
        break;
        default:
            return "";
    }
}

int encode_file_extention(char *file_extention) {
    if (strcmp(file_extention, "txt") == 0)
        return TXT;
    if (strcmp(file_extention, "css") == 0)
        return CSS;
    if (strcmp(file_extention, "html") == 0)
        return HTML;
    if (strcmp(file_extention, "js") == 0)
        return JS;
    if (strcmp(file_extention, "md") == 0)
        return MD;
    if (strcmp(file_extention, "csv") == 0)
        return CSV;

    return -1;
}

const char* get_media_type(char *file_extention) {
    int file_code = encode_file_extention(file_extention);
    
    switch(file_code) {
        case TXT:
            return "text/plain";
        break;
        case CSS:
            return "text/css";
        break;
        case HTML:
            return "text/html";
        break;
        case JS:
            return "text/javascript";
        break;
        case CSV:
            return "text/csv";
        break;
        case MD:
            return "text/markdown";
        break;
        default:
            return "type/subtype";
    }
}

char* http_response_gen(char* protocol_version, int status_code, const char* content_type, const char* body) {
    char* response = malloc(sizeof(char) * (HEADER_SIZE_ESTIMATE + sizeof(body)));
    snprintf(response, HEADER_SIZE_ESTIMATE + sizeof(body),
             "%s %d %s\r\n"
             "Content-type: %s\r\n"
             "Content-length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             protocol_version, status_code, get_status_text(status_code),
             content_type,
             sizeof(body),
             body);
    return response;
}

void* process_client(void* ptr_fd) {
    int client_fd = *((int*)ptr_fd);
    char* buffer = malloc(HEADER_SIZE_ESTIMATE * sizeof(char));

    ssize_t bytes_received = recv(client_fd, buffer, HEADER_SIZE_ESTIMATE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(client_fd);
        //free(ptr_fd);
        // THIS LINE CREATES AN ERROR
        // Trying to free the pointer twice
        // Adding a free at the end of while should be enough and safe
        free(buffer);
        return NULL;
    }
    if (bytes_received == 0) return NULL;

    // REQUEST DETAILS
    struct message_details response_det;

    // RESPONSE BUFFER
    char* response_buf;
    parseRequest(buffer, bytes_received, &response_det.method, &response_det.path, &response_det.protocol_version);
    char *root = "/";

    if (strcmp(response_det.path, root) == 0)
        response_buf = http_response_gen(response_det.protocol_version, 404, "text/html", "<html><body><h1>404 Not Found</h1></body></html>");

    // TO IMPLEMENT FINDING, OPENING AND SENDING THE APPROPRIATE FILE, AND PARSING ITS MIME TYPE
    // AND CLOSING THE CONNECTION, ALONG WITH REQUIRED HELPER FUNCTIONS
    printf("%s\n",response_buf);
    close(client_fd);
    free(response_buf);
    free(buffer);
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr;

    // SOCKET CREATION
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        error("Socket creation failed");
    }
    printf("Socket created successfully\n");

    // SOCKET CONFIGURATION
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // BINDING THE SOCKET TO THE PORT
    if (bind(server_fd, (struct sockaddr*)&server_addr, (socklen_t)sizeof(server_addr)) == -1) {
        error("Socket binding failed");
    }
    printf("Socket successfully bound to %s:%hu\n\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    // LISTENING FOR CONNECTIONS (max queue size = 10)
    if (listen(server_fd, MAX_QUEUE_SIZE) == -1) {
        error("Listen failed");
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
            perror("Client couldn't be accepted");
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

