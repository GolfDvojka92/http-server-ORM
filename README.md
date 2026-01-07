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
HTTP protocol uses TCP as the communication protocol. In C this means we have to create a server socket using the TCP standard (SOCK_STREAM), bind it to our IP address using an open port, all of which are stored inside the server_addr variable. After binding the socket to the IP, we have to call listen to start listening for connections. ``` int server_fd; struct sockaddr_in server_addr;

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
```
