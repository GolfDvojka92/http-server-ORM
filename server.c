#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define IP_ADDRESS "192.168.100.3"
#define PORT 8080
#define DEFAULT_BUFF_SIZE 512
#define MAX_QUEUE_SIZE 10

void error(void* msg) {
    perror((char*)msg);
    exit(EXIT_FAILURE);
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
    printf("Waiting for requests...\n");
    while (1) {
        // CLIENT INFORMATION
        int* client_fd = malloc(sizeof(int));
        struct sockaddr_in client_addr;
        socklen_t client_addr_length = sizeof(client_addr);

    }

    return 0;
}

