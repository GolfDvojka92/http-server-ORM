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


