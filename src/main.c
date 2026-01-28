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
