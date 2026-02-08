#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define ERROR_EXIT(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)
#define ERROR_RETURN(R, ...) do { fprintf(stderr, __VA_ARGS__); return(R); } while (0)
