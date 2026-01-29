# Basic HTTP Server - ORM
```
Implementirati aplikaciju koja realizuje funkcionalnost jednostavnog HTTP (tj. WEB) servera. Server treba da omogući preuzimanje 
sadržaja tekstualnih datoteka sa računara sa unapred zadate putanje. Putanja do datoteke se zadaje u adresnoj liniji i prosleđuje
serveru putem HTTP GET zahteva. Server prima zahteve od klijenta u tekstualnom obliku korišćenjem TCP protokola i unapred zadatog
porta. U slučaju da datoteka postoji, šalje se odgovor na zahtev koji je sledećeg oblika:

HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Connection: close
...sadržaj datoteke...

U slučaju da datoteka ne postoji, šalje se odgovor

HTTP/1.1 404 Page not found
Zadatak 8 – HTTP server
Connection: close
```

Starts a locally hosted HTTP server that's capable of receiving GET and HEAD requests, formatted according to the RFC2616 referencing the HTTP/1.1 version of the protocol. It's capable of displaying only the most common types of text files, and requests for an unsupported file type are responded to with an application/octet-stream MIME type, meaning they are downloaded.

## Usage guide

### Building

In terminal, from the root directory of the project run ``cmake -B build`` to generate build files. After that, run ``cmake --build build`` to compile the project.

### Running

To run the server launch the ``HTTPServer`` executable from the ``build/`` directory. The server is hosted on the localhost IP address. While the server is running all HTTP ``GET`` and ``HEAD`` requests directed at it will look for the requested path inside the ``server_data/`` directory.

## Starting the server

HTTP protocol uses TCP as the communication protocol. In C this means we have to create a server socket using the TCP standard (SOCK_STREAM).

``` 
// SERVER INFORMATION
int server_fd;
struct sockaddr_in server_addr;

// SOCKET CREATION
if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    error("Socket creation failed");
}
printf("Socket created successfully\n");
```

Next, we bind it to our IP address using an open port, both of which are stored inside the server_addr variable.

```
// SOCKET CONFIGURATION
server_addr.sin_addr.s_addr = INADDR_ANY;
server_addr.sin_family = AF_INET;           // IPv4 spec
server_addr.sin_port = htons(PORT);

// BINDING THE SOCKET TO THE PORT
if (bind(server_fd, (struct sockaddr*)&server_addr, (socklen_t)sizeof(server_addr)) == -1) {
    error("Socket binding failed");
}
printf("Socket successfully bound to %s:%hu\n\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
```

After binding the socket to the IP, we have to call listen to start listening for connections.

```
// LISTENING FOR CONNECTIONS (max queue size = 10)
if (listen(server_fd, MAX_QUEUE_SIZE) == -1) {
    error("Listen failed");
}
printf("Waiting for requests...(<C-c> to stop)\n");
```

## Processing clients

After successfully starting the server we have to process incoming clients. HTTP works by receiving an HTTP request, parsing it to see if it's formatted according to the protocol (RFC2616), and sending an adequate HTTP response. With this in mind we can say that HTTP is a stateless protocol, which means we have to close the connection after the client receives the response (not always the case, true for this project). This means we are free to process each client we receive in it's separate thread, and are free to detach them from the main thread. This is all done in a while(1) loop since we want to accept connections as long as the server is running.

```
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
```

The threads are calling the process_client function. First we allocate the memory required for the request buffer, then we fill the buffer by receiving the HTTP request from the client.

```
void* process_client(void* ptr_fd) {
    int client_fd = *((int*)ptr_fd);
    char* buffer = malloc(HEADER_SIZE_ESTIMATE * sizeof(char));

    // RECIEVING THE REQUEST FROM CLIENT
    ssize_t bytes_received = recv(client_fd, buffer, HEADER_SIZE_ESTIMATE, 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
        close(client_fd);
        free(ptr_fd);
        free(buffer);
        return NULL;
    }
    if (bytes_received == 0) return NULL;
```

### Parsing the request line

After that, we have to parse the request. First we separate the request line from the headers using ``strtok_r()``, then we forward the request line to the ``parse_request()`` function.

```
    // PARSING REQUEST LINE
    char* saveptr;
    char* request_line = strtok_r(buffer, "\r\n", &saveptr);        // this saves the rest of the buffer into the saveptr, allows for parsing of the headers in the future

    struct request_line response;
    parse_request(request_line, bytes_received, &response);
```

The ``parse_request()`` function separates the forwarded request line into the 3 segments, and fills the forwarded struct with them. If parsing of the headers were to be 
required, it would be easily done just by forwarding the entire request buffer, and filling a hash map with all of the headers from the request, however, this 
project in particular implements only ``GET`` and ``HEAD`` HTTP methods, to which only the request line is important.

```
void parse_request(char* buffer, ssize_t buf_len, struct request_line* msg) {
    char *saveptr; // necessary because of thread safety
    msg->method = strtok_r(buffer, " ", &saveptr);
    msg->path = strtok_r(NULL, " ", &saveptr);
    // TOKENIZES THE PROTOCOL VERSION BY THE SLASH, saveptr POINTS TO ONLY THE NUMBERS AFTER IT
    strtok_r(NULL, "/", &saveptr);
    msg->protocol_version = saveptr;

    // ABLE TO IMPLEMENT PARSING OF HEADERS, UNNECESSARY IN OUR CASE
}
```

Upon parsing the request line, we start handling the HTTP request. The server forbids access to the root directory.

```
    // RESPONSE BUFFER
    char* response_buf;

    // DENYING ACCESS TO ROOT
    if (strcmp(response.path, "/") == 0) {
        response_buf = generate_response(response, 403, "text/html", ERR_403);
        send_response(client_fd, response_buf, buffer, ptr_fd);
        return NULL;
    }
```

### Opening the requested file

After that, it opens the requested file, and if it fails to do so, we return a 404.

```
    // OPENING THE FILE
    FILE *searched_file = fopen(relative_path(response.path), "rb");
    if (searched_file == NULL) {
        //FILE NOT FOUND
        response_buf = generate_response(response, 404, "text/html", ERR_404);
        send_response(client_fd, response_buf, buffer, ptr_fd);
        return NULL;
    }
```

If the requested file is found, it calculates its size using ``ftell()`` and stores it into a long. The server then fills a buffer with the contents of the file, and forwards it to the ``generate_response()`` function, filling the response buffer.

```
    //READING FILE CONTENT
    fseek(searched_file, 0, SEEK_END);
    long file_byte_count = ftell(searched_file);
    rewind(searched_file);


    //FILLING UP FILE BUFFER FOR SENDING
    char *file_buf = malloc(file_byte_count);
    fread(file_buf, sizeof(char), file_byte_count, searched_file);

    response_buf = generate_response(response, 200, get_mime_type(response.path), file_buf);
```

### Generating HTTP response

The ``generate_response()`` function returns a pointer to the finalized response buffer, ready to be sent to the client. The body is catenated to the response depending on the request method.

```
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
```

The server frees all the used up memory necessary for the creation of the response buffer, and sends the response back to the client using the ``send_response()`` function.

```
    free(file_buf);
    fclose(searched_file);

    send_response(client_fd, response_buf, buffer, ptr_fd);
    return NULL;
}
```

### Sending the response

The ``send_response()`` function, in addition to sending the HTTP response to the client, closes the connection and frees up the remaining dynamically allocated memory.

```
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
```
