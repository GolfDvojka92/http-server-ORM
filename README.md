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

Starts a locally hosted http server that's capable of receiving GET and HEAD requests, formatted according to the RFC2616 referencing
the HTTP/1.1 version of the protocol. It's capable of displaying only the most common types of text files, and requests for an unsupported
file type are responded to with an application/octet-stream MIME type, meaning they are downloaded.
