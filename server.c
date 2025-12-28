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

void error(void* msg) {
    perror((char*)msg);
    exit(EXIT_FAILURE);
}

void parseRequest(char* buffer, ssize_t buf_len, char* method, char* path, char* protocol_version) {
    char *saveptr; // necessary because of thread safety
    method = strtok_r(buffer, " ", &saveptr);
    path = strtok_r(NULL, " ", &saveptr);
    protocol_version = strtok_r(NULL, "\r\n", &saveptr);
}

char* http_response_gen(int status_code, const char* content_type, const char* body) {
    char* response;
    snprintf(response, HEADER_SIZE_ESTIMATE + sizeof(body),
             body);
    // NOT IMPLEMENTED
    return response;
}

void* processClient(void* ptr_fd) {
    int client_fd = *((int*)ptr_fd);
    char* buffer = malloc(DEFAULT_BUFF_SIZE * sizeof(char));

    ssize_t bytes_received = recv(client_fd, buffer, DEFAULT_BUFF_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(client_fd);
        free(ptr_fd);
        free(buffer);
        return NULL;
    }
    if (bytes_received == 0) return NULL;

    // REQUEST DETAILS
    char* method;
    char* path;
    char* protocol_version;
    char* mime_type;

    // RESPONSE BUFFER
    char* response_buf;
    parseRequest(buffer, bytes_received, method, path, protocol_version);
    if (strcmp(path, "/") == 0)
        response_buf = http_response_gen(404, "text/html", "<html><body><h1>404 Not Found</h1></body></html>");

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
        pthread_create(&client_thread, NULL, processClient, (void*)&client_fd);
        pthread_detach(client_thread);
    }

    return 0;
}

