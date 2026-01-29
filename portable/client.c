#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define SERVER_IP_ADDRESS "192.168.100.3"
#define IP_ADDRESS "192.168.1.34"
#define PORT 8080
#define DEFAULT_BUFF_SIZE 2048
#define HEADER_SIZE_ESTIMATE 256
#define PROTOCOL_VERSION "HTTP/1.1"

void error(void* msg) {
    perror((char*)msg);
    exit(EXIT_FAILURE);
}

char* http_get_request_gen(char* path) {
    char* request = malloc((HEADER_SIZE_ESTIMATE) * sizeof(char));
    snprintf(request, HEADER_SIZE_ESTIMATE * sizeof(char),
            "GET %s %s\r\n"
            "Host: %s\r\n",
            path, PROTOCOL_VERSION, SERVER_IP_ADDRESS);
    // GET method headers are HOST and the first line, no body

    return request;
}

int main(int argc, char** argv) {
    if (argc != 2)
        error("Invalid call format!\nSecond parameter is the name of the desired file!\n");

    int client_fd;
    struct sockaddr_in server_addr;
    
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
        error("Socket creation failed.\n");

    printf("Socket successfully created!\n");

    server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        error("Failed to connect to server.\n");

    printf("Successfully connected to the server!\n");

    char *request_buf = http_get_request_gen(argv[1]);

    if (send(client_fd, request_buf, strlen(request_buf), 0) == -1)
        perror("Failed to send HTTP request to server.\n");

    printf("Sent the HTTP request to server.\n");

    char *recv_message = malloc(DEFAULT_BUFF_SIZE * sizeof(char));
    int message_len;

    if ( (message_len = recv(client_fd, recv_message, DEFAULT_BUFF_SIZE, 0)) > 0) {
        recv_message[message_len] = '\0';
        printf("Recieved response from server.\n");
        printf("Message Length: %d\n", message_len);
        printf("Message Content:\n%s\n", recv_message);
    }
    
    free(recv_message);
    free(request_buf);
    close(client_fd);
    return 0;
}