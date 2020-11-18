#include "tool.h"

typedef enum {
    WRONG_ARGS = 1,
    WRONG_ID_INFO = 2,
    INVALID_PORT = 3,
    INVALID_MAP = 4
} Status;

//print the error message to stderr
Status ct_error_message(Status status) {
    const char* message[] = {
            "",
            "Usage: control2310 id info [mapper]\n",
            "Invalid char in parameter\n",
            "Invalid port\n",
            "Can not connect to map\n"
            };
    fputs(message[status], stderr);
    fflush(stderr);
    return status;
}

/*
 * this function check the number of argvs
 * parameter: argc: the number of argvs
 * return true if argvs are valid
 *      else return false
 */
bool is_valid_args(int argc) {
    if (argc < 3) {
        return false;
    }
    return true;
}

/*
 * this function check the range of port
 * parameter: argv
 * return true if argv is valid
 *      else return false
 */
bool is_valid_port(char** argv) {
    if (atoi(argv[3]) <= 0 || atoi(argv[3]) > 65535) {
        return false;
    }
    return true;
}

/*
 * check wether the string include "\n", "\r", ":"
 * parameter:
 *          char* string : check this string
 * return true if string no
 * else return false
 */
bool invalid_char(char* string) {
    if (strstr(string, "\n") == NULL) {
        if (strstr(string, "\r") == NULL) {
            if (strstr(string, ":") == NULL) {
                return false;
            }
        }
    }
    return true;
}

/*
 * this function check whther the port is wait for connection
 * parameter: int index: index of argv
 *              char** argv
 *              RocInfo* rocInfo
 * return true if the port is waiting
 *      else return false
 */
bool is_valid_mapper(int index, char** argv, ControlInfo* controlInfo) {
    struct sockaddr_in servAddr;
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(atoi(argv[index]));
    inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr);
    int connectFd = connect(socketFd, (struct sockaddr*)(&servAddr),
            sizeof(servAddr));
    if (connectFd < 0) {
        close(socketFd);
        return false;
    } else {
        controlInfo->socketFd = socketFd;
        return true;
    }
}

/*
 * connect to mapper and send info "!abc:123\n"
 * parameters:
 *              ControlInfo* controlInfo,
 *              char** argv
 * return void
 */
void client_connect(ControlInfo* controlInfo, char** argv) {
    FILE* stream = fdopen(controlInfo->socketFd, "w");
    char num[6]; //save port number
    sprintf(num, "%d", ntohs(controlInfo->client.sin_port));
    int length = strlen(argv[1]) + strlen(argv[2]) + 3;
    char* message = malloc(sizeof(char) * length);
    memset(message, 0, length);
    strcat(message, "!");
    strcat(message, argv[1]);
    strcat(message, ":");
    strcat(message, num);
    strcat(message, "\n");
    fprintf(stream, "%s", message);
    fflush(stream);
    free(message);
    fclose(stream);
    close(controlInfo->socketFd);
}

/*
 * when get input "log\n", print the log
 * parameters:
 *          ControlInfo* controlInfo,
 *          FILE* printOut: where to print
 * return void
 */
void log_command(ControlInfo* controlInfo, FILE* printOut) {
    for (int i = 0; i < controlInfo->airport.planeCount; ++i) {
        fprintf(printOut, "%s", controlInfo->airport.log[i]);
        fflush(printOut);
    }
    fprintf(printOut, ".\n");
    fflush(printOut);
    pthread_mutex_unlock(&controlInfo->controlMutex);
}

/*
 * print the airport information to FILE* write
 * parameters:
 *          char* command: information to save
 *          ControlInfo* controlInfo
 *          FILE* write: where to print
 * return void
 */
void control_command(char* command, ControlInfo* controlInfo, FILE* write) {
    if (controlInfo->airport.planeCount == 0) {
        controlInfo->airport.log = malloc(sizeof(char*) * 1);
        controlInfo->airport.log[0] = malloc(sizeof(char) *
                strlen(command));
        strcpy(controlInfo->airport.log[0], command);
        memset(command, 0, strlen(command));
        controlInfo->airport.planeCount += 1;
    } else {
        controlInfo->airport.log = realloc(controlInfo->airport.log,
                controlInfo->airport.planeCount + 1);
        controlInfo->airport.log[controlInfo->airport.planeCount] =
                malloc(sizeof(char) * strlen(command));
        strcpy(controlInfo->airport.log[controlInfo->airport.planeCount],
                command);
        memset(command, 0, strlen(command));
        controlInfo->airport.planeCount += 1;
    }
    fprintf(write, "%s\n", controlInfo->airport.airportInfo);
    fflush(write);
    fclose(write);
    pthread_mutex_unlock(&controlInfo->controlMutex);
}

/*
 * when function get a command, do something
 * parameter:
 *          void* controlInformation
 *  return NULL;
 */
void* control_action(void* controlInformation) {
    ControlInfo* controlInfo = (ControlInfo*)controlInformation;
    pthread_mutex_lock(&controlInfo->controlMutex);
    char* command = malloc(sizeof(char) * 255);
    FILE* read = fdopen(controlInfo->acceptFd, "r");
    FILE* write = fdopen(controlInfo->acceptFd, "w");
    fgets(command, 254, read);
    fflush(read);
    if (strcmp(command, "log\n") == 0) {
        log_command(controlInfo, write);
        while (1) {
            fgets(command, 254, read);
            fflush(read);
            log_command(controlInfo, write);
            memset(command, 0, 255);
        }
    } else {
        control_command(command, controlInfo, write);
        memset(command, 0, 255);
        fclose(read);
        free(command);
        return NULL;
    }
}

/*
 * wait for connection, and create thread
 * parameters:
 *          ControlInfo* controlInfo
 *          int argc
 *          char** argv
 * return number
 */
int listening(ControlInfo* controlInfo, int argc, char** argv) {
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;        // IPv4  for generic could use AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // Because we want to bind with it
    int err;
    if ((err = getaddrinfo("localhost", 0, &hints, &ai))) {
        freeaddrinfo(ai);
        fprintf(stderr, "%s\n", gai_strerror(err));
        fflush(stderr);
        return 1;   // could not work out the address
    }
    int serv = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(serv, (struct sockaddr*)ai->ai_addr, sizeof(struct sockaddr))) {
        perror("Binding");
        return 3;
    }
    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(struct sockaddr_in));
    socklen_t len = sizeof(struct sockaddr_in);
    if (getsockname(serv, (struct sockaddr*)&ad, &len)) {
        perror("sockname");
        return 4;
    }
    controlInfo->client.sin_port = ad.sin_port;
    printf("%u\n", ntohs(ad.sin_port));
    fflush(stdout);
    if (listen(serv, 100)) {// allow up to 100 connection requests to queue
        perror("Listen");
        return 4;
    }
    if (argc == 4) { //send information to mapper
        client_connect(controlInfo, argv);
    }
    while (1) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int acceptFd = accept(serv, (struct sockaddr*)&client, &len);
        controlInfo->acceptFd = acceptFd;
        controlInfo->client = client;
        pthread_t threadID;
        pthread_create(&threadID, NULL, control_action, controlInfo);
        pthread_detach(threadID);
    }
    close(serv);
    return 0;
}

int main(int argc, char** argv) {
    ControlInfo controlInfo;
    if (pthread_mutex_init(&controlInfo.controlMutex, NULL) != 0) {
        return 0;
    }
    for (int i = 0; i < argc; ++i) {
        if (strlen(argv[i]) == 0) {
            exit(ct_error_message(1));
        }
    }
    if (!is_valid_args(argc)) {
        exit(ct_error_message(1));
    } else if (invalid_char(argv[1])) {
        exit(ct_error_message(2));
    } else if (invalid_char(argv[2])) {
        exit(ct_error_message(2));
    } else if (argc == 4 && !is_valid_port(argv)) {
        exit(ct_error_message(3));
    } else if (argc == 4 && !is_valid_mapper(3, argv, &controlInfo)) {
        exit(ct_error_message(4));
    } else {
        controlInfo.airport.airportID = malloc(sizeof(char) *
                strlen(argv[1]));
        memset(controlInfo.airport.airportID, 0, strlen(argv[1]));
        strcpy(controlInfo.airport.airportID, argv[1]);
        controlInfo.airport.airportInfo = malloc(sizeof(char) *
                strlen(argv[2]));
        memset(controlInfo.airport.airportInfo, 0, strlen(argv[2]));
        strcpy(controlInfo.airport.airportInfo, argv[2]);
        controlInfo.airport.planeCount = 0;
        listening(&controlInfo, argc, argv);
    }
    pthread_mutex_destroy(&controlInfo.controlMutex);
    return 0;
}