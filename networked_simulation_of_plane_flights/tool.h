#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

//mapper struct
typedef struct {
    char* mapperID;
    int mapperPort;
} Entry;

typedef struct {
    Entry** entries;
    int numOfMap;
} Mapper;

typedef struct {
    int connectFd;
    struct sockaddr_in client;
    Mapper* mapper;
    pthread_mutex_t mutex;
} ClientInfo;

//control struct
typedef struct {
    char* airportID;
    char* airportInfo;
    unsigned int airportPort;
    char** log;
    int planeCount;
} Airport;

typedef struct {
    int acceptFd;
    struct sockaddr_in client;
    Airport airport;
    int socketFd;
    pthread_mutex_t controlMutex;
} ControlInfo;

//roc struct
typedef struct {
    int socketFd;
    struct sockaddr_in serv;
    char** argv;
    char* output;
    unsigned long outputSize;
    pthread_mutex_t rocMutex;
} RocInfo;
