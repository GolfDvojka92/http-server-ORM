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
To build the portable version of the project, from ```portable/build/``` directory run ```cmake --build .``` to compile the portable project variant.

### Running

To run the server launch the ``HTTPServer`` executable from the ``build/`` directory. The server is hosted on the localhost IP address. While the server is running all HTTP ``GET`` and ``HEAD`` requests directed at it will look for the requested path inside the ``server_data/`` directory. To run the client launch the ``Client`` executable form the ``build`` directory with two arguments, first the method either GET or HEAD and second the name of the file you want to search for on the server side.

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

Upon parsing the request line, we start handling the HTTP request. The server checks if the request method is implemented, and if not returns a 501. It also forbids access to the root directory.

```
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
```

### Opening the requested file

After that, it opens the requested file, and if it fails to do so, we return a 404. All data is stored in the ``server_data/`` directory, so it updates the path accordingly before attempting to open it. (NOTE: the binary is run from ``build/`` directory so the path to file must go one directory back and into the ``server_data/`` directory!)

```
    char dir_name[] = "../server_data";
    char* full_path = malloc(sizeof(char) * strlen(dir_name) + strlen(response.path));
    strcat(full_path, dir_name);
    strcat(full_path, response.path);

    // OPENING THE FILE
    FILE *searched_file = fopen(relative_path(full_path), "rb");
    if (searched_file == NULL) {
        //FILE NOT FOUND
        response_buf = generate_response(response, 404, "text/html", ERR_404);
        free(full_path);
        send_response(client_fd, response_buf, buffer, ptr_fd);
        return NULL;
    }

    free(full_path);
```

If the requested file is found, it calculates its size using ``ftell()`` and stores it into a long. The server then fills a buffer with the contents of the file, and forwards it to the ``generate_response()`` function, filling the response buffer. (NOTE: Inserting char ```'\0'``` is a memory-safe precaution due to later use of method ```strcat()```!)

```
    //READING FILE CONTENT
    fseek(searched_file, 0, SEEK_END);
    long file_byte_count = ftell(searched_file);
    rewind(searched_file);


    //FILLING UP FILE BUFFER FOR SENDING
    char *file_buf = malloc(file_byte_count + 1);
    fread(file_buf, sizeof(char), file_byte_count, searched_file);
    file_buf[file_byte_count] = '\0';

    response_buf = generate_response(response, 200, get_mime_type(response.path), file_buf);
```

### Generating HTTP response

The ``generate_response()`` function returns a pointer to the finalized response buffer, ready to be sent to the client. The body is catenated to the response depending on the request method. (NOTE: Inserting char ```'\0'``` is mandatory due to the fact that ```strcat()``` behaviour is undefined if both passed strings don't end with the null terminator!)

```
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

## Client starting

When using the client, we first need to ensure that the right paramethers and in the right order are passed to the program. That is why the first lines are related to the potential user errors that can occur.

```c
if (argc != 3)
    ERROR_EXIT("Invalid call format!\nUsage: ./client [METHOD] [FILE_NAME]\nSupported methods: GET, HEAD\n");

if (strcmp(argv[1], "GET") != 0 && strcmp(argv[1], "HEAD") != 0)
    ERROR_EXIT("Invalid method used!\nSupported methods: GET, HEAD\n");
```

Then simmilar to the procedure on the server side, we need to create a socket, configure it and the connect to the server with the known local IP adress.

```c
int client_fd;
struct sockaddr_in server_addr;
    
// Socket creation
client_fd = socket(AF_INET, SOCK_STREAM, 0);
    
if (client_fd == -1)
    ERROR_EXIT("Socket creation failed.\n");

printf("Socket successfully created!\n");

// Socket configuration
server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(PORT);

// Connecting to server
if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    ERROR_EXIT("Failed to connect to server.\n");

printf("Successfully connected to the server!\n");
```

### Formatting the request message

The next step is to send the message that is generated using the ```http_get_request_gen(char*, char*)``` method. The method formats the request according to the HTTP request message rules. They specify the first line contains the method name, full path to the file and HTTP version name, all seperated by a space. After the first line there is a list of header metadata in the format of ```Header_name: header_value```, one per line. The implemented methods ```GET``` and ```HEAD``` require only the header ```Host``` which specifies the IP adress to which the request is sent. In some cases the request message contains a body, however in the case of ```GET``` and ```HEAD``` methods, it is discouraged.

```c
char* http_get_request_gen(char* method, char* path) {
    char* request = malloc((HEADER_SIZE_ESTIMATE) * sizeof(char));
    snprintf(request, HEADER_SIZE_ESTIMATE * sizeof(char),
            "%s /%s %s\r\n"
            "Host: %s\r\n\0",
            method, path, PROTOCOL_VERSION, SERVER_IP_ADDRESS);
    // GET method headers are HOST and the first line, no body
    // HEAD method shares the same headers

    return request;
}
```

After formatting the message, it is sent to the server, subsequently the response message is recieved and displayed in the terminal. The client closes after this procedure.

```c
// Forming request message
char *request_buf = http_get_request_gen(argv[1], argv[2]);

// Sending request    
if (send(client_fd, request_buf, strlen(request_buf), 0) == -1)
    ERROR_EXIT("Failed to send HTTP request to server.\n");

printf("Sent the HTTP request to server.\n");

char *recv_message = malloc(DEFAULT_BUFF_SIZE * sizeof(char));
int message_len;

// Receiving answer
if ( (message_len = recv(client_fd, recv_message, DEFAULT_BUFF_SIZE, 0)) > 0) {
    recv_message[message_len] = '\0';
    printf("Recieved response from server.\n");
    printf("Message Length: %d\n", message_len);
    printf("Message Content:\n%s\n", recv_message);
}
    
// Closing client
free(recv_message);
free(request_buf);
close(client_fd);
return 0;
```