# Basic HTTP Server - ORM

Implementirati aplikaciju koja realizuje funkcionalnost jednostavnog HTTP (tj. WEB) servera. Server treba da omogući preuzimanje sadržaja tekstualnih datoteka sa računara sa unapred zadate putanje. Putanja do datoteke se zadaje u adresnoj liniji i prosleđuje serveru putem HTTP GET zahteva. Server prima zahteve od klijenta u tekstualnom obliku korišćenjem TCP protokola i unapred zadatog porta. U slučaju da datoteka postoji, šalje se odgovor na zahtev koji je sledećeg oblika:

HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Connection: close
...sadržaj datoteke...

U slučaju da datoteka ne postoji, šalje se odgovor

HTTP/1.1 404 Page not found
Zadatak 8 – HTTP server
Connection: close
