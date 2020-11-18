#include "tool.h"

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
 * this function check the range of port
 * parameter: argv
 * return true if argv is valid
 *      else return false
 */
bool is_valid_port_range(char* bufPort) {
    if (atoi(bufPort) <= 0 || atoi(bufPort) > 65535) {
        return false;
    }
    return true;
}

/*
 * sort the information like "!abc:123"
 * parameters:
 *          int* index: where to start sort
 *          Mapper* newMapper
 *          Mapper* oldMapper
 * return void
 */
void sort_rest(int* index, Mapper* newMapper, Mapper* oldMapper) {
    for (int i = *index + 1; i < newMapper->numOfMap; ++i) {
        newMapper->entries[i][0] = oldMapper->entries[*index][0];
        *index += 1;
    }
}

/*
 * handle the command like "?abc"
 * parameters:
 *          const char* command: the command include ?
 *          ClientInfo* clientInfo,
 *          char* bufID
 *          FILE* streams
 * return true if command is valid and action did
 * else return false
 */
bool command_question(const char* command, ClientInfo* clientInfo,
        char* bufID, FILE* streams) {
    bool flag = true;
    sscanf(command, "?%s\n", bufID);
    if (invalid_char(bufID)) {
        return false;
    } else {
        if (clientInfo->mapper->numOfMap == 0) {
            fprintf(streams, ";\n");
            fflush(streams);
        } else {
            for (int i = 0; i < clientInfo->mapper->numOfMap; ++i) {
                if (strcmp(clientInfo->mapper->entries[i][0].mapperID, bufID)
                        == 0) {
                    flag = false;
                    fprintf(streams, "%d\n",
                            clientInfo->mapper->entries[i][0].mapperPort);
                    fflush(streams);
                    break;
                }
            }
            if (flag) {
                fprintf(streams, ";\n");
                fflush(streams);
            }
        }
    }
    return true;
}

/*
 * run the action of command like "!abc:123"
 * parameters:
 *          ClientInfo* clientInfo
 *          char* bufID: save "abc"
 *          char* bufPort: save "123"
 * return true if command is valid and action did
 * else return false
 */
bool run_exclamation(ClientInfo* clientInfo, char* bufID, char* bufPort) {
    Mapper newMapper;
    newMapper.numOfMap = 0;
    for (int i = 0; i < clientInfo->mapper->numOfMap; ++i) {
        if (strcmp(clientInfo->mapper->entries[i][0].mapperID,
                bufID) == 0) {
            return false;//ignore
        }
    }
    newMapper.entries = malloc(sizeof(Entry*) *
            (clientInfo->mapper->numOfMap + 1));
    newMapper.numOfMap = clientInfo->mapper->numOfMap + 1;
    for (int i = 0; i < newMapper.numOfMap - 1; ++i) {
        newMapper.entries[i] = malloc(sizeof(Entry) * 1);
        newMapper.entries[i][0].mapperID = malloc(sizeof(char) *
                (strlen(clientInfo->mapper->entries[i][0].mapperID) + 1));
    }
    newMapper.entries[newMapper.numOfMap - 1] =
            malloc(sizeof(Entry) * 1);
    newMapper.entries[newMapper.numOfMap - 1][0].mapperID =
            malloc(sizeof(char) * (strlen(bufID) + 1));
    for (int i = 0; i < newMapper.numOfMap; ++i) {
        if (i == newMapper.numOfMap - 1) { //case for the 'biggest'
            strcpy(newMapper.entries[i][0].mapperID, bufID);
            newMapper.entries[i][0].mapperPort = atoi(bufPort);
        } else {
            if (strcmp(bufID, clientInfo->
                    mapper->entries[i]->mapperID) < 0) {
                strcpy(newMapper.entries[i][0].mapperID, bufID);
                newMapper.entries[i][0].mapperPort = atoi(bufPort);
                sort_rest(&i, &newMapper, clientInfo->mapper);
            } else {
                strcpy(newMapper.entries[i][0].mapperID,
                        clientInfo->mapper->entries[i][0].mapperID);
                newMapper.entries[i][0].mapperPort =
                        clientInfo->mapper->entries[i][0].mapperPort;
            }
        }
    }
    *clientInfo->mapper = newMapper;
    return true;
}

/*
 * handle the command like "!abc:123"
 * parameters:
 *          const char* command: the command include !
 *          ClientInfo* clientInfo,
 *          char* bufID: save "abc"
 *          char* bufPort: save "123"
 * return true if command is valid and action did
 * else return false
 */
bool command_exclamation(const char* command, ClientInfo* clientInfo,
        char* bufID, char* bufPort) {
    sscanf(command, "!%[^:]", bufID);
    sscanf(command, "%*[^:]:%s\n", bufPort);
    if (strcmp(bufPort, "") == 0)return false;
    if (!is_valid_port_range(bufPort)) {
        return false;
    }
    for (int j = 0; j < strlen(bufPort); ++j) {
        if (!isdigit(bufPort[j])) {
            return false;
        }
    }
    if (invalid_char(bufID) || invalid_char(bufPort)) {
        return false;
    } else {
        if (clientInfo->mapper->numOfMap == 0) {
            clientInfo->mapper->entries = malloc(sizeof(Entry*) * 1);
            clientInfo->mapper->entries[0] = malloc(sizeof(Entry) * 1);
            clientInfo->mapper->entries[0][0].mapperID =
                    malloc(sizeof(char) * (strlen(bufID) + 1));
            strcpy(clientInfo->mapper->entries[0][0].mapperID, bufID);
            clientInfo->mapper->entries[0][0].mapperPort = atoi(bufPort);
            clientInfo->mapper->numOfMap += 1;
        } else {
            run_exclamation(clientInfo, bufID, bufPort);
        }
    }
    return true;
}

/*
 * free buffer and unlock muyex
 * parameter:
 *          char* bufID
 *          char* bufPort
 *          ClientInfo* clientInfo
 * return void
* return true if command is valid and action did
 * else return false
 */
void free_close(char* bufID, char* bufPort, ClientInfo* clientInfo) {
    free(bufID);
    free(bufPort);
    pthread_mutex_unlock(&clientInfo->mutex);
}

/*
 * handle the command, and run corresponding function
 * parameters:
 *          const char* command: input command
 *          ClientInfo* clientInfo
 *          FILE* streams: where to print output
 */
bool command_handler(const char* command, ClientInfo* clientInfo,
        FILE* streams) {
    char* bufID = malloc(sizeof(char) * 255);
    char* bufPort = malloc(sizeof(char) * 255);
    memset(bufID, 0, 255);
    memset(bufPort, 0, 255);
    if (command[0] == '?') {
        if (command_question(command, clientInfo, bufID, streams)) {
            free_close(bufID, bufPort, clientInfo);
            return true;
        } else {
            free_close(bufID, bufPort, clientInfo);
            return false;
        }
    } else if (command[0] == '!') {
        if (command_exclamation(command, clientInfo, bufID, bufPort)) {
            free_close(bufID, bufPort, clientInfo);
            return true;
        } else {
            free_close(bufID, bufPort, clientInfo);
            return false;
        }
    } else if (command[0] == '@') {
        for (int i = 0; i < clientInfo->mapper->numOfMap; ++i) {
            fprintf(streams, "%s:%d\n",
                    clientInfo->mapper->entries[i][0].mapperID,
                    clientInfo->mapper->entries[i][0].mapperPort);
            fflush(streams);
        }
        free_close(bufID, bufPort, clientInfo);
        return true;
    } else {
        free_close(bufID, bufPort, clientInfo);
        return false;
    }
}

/*
 * get a connection, and read command, print output
 * parameter:
 *          void* clientInformation
 * return NULL
 */
void* client_action(void* clientInformation) {
    ClientInfo* clientInfo = (ClientInfo*)clientInformation;
    pthread_mutex_lock(&clientInfo->mutex);
    char* command = malloc(sizeof(char) * 1024);
    memset(command, 0, 1024);
    FILE* stream = fdopen(clientInfo->connectFd, "r");
    FILE* streams = fdopen(clientInfo->connectFd, "w");
    while (1) {
        fgets(command, 1023, stream);
        fflush(stream);
        command_handler(command, clientInfo, streams);
        memset(command, 0, 1024);
    }
    fclose(stream);
    free(clientInfo);
}

/*
 * open port and waiting connection, create thread
 * parameter:
 *          ClientInfo * clientInfo
 * return number
 */
int service_listen(ClientInfo* clientInfo) {
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
    int serv = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
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
    printf("%u\n", ntohs(ad.sin_port));
    fflush(stdout);
    if (listen(serv, 100)) {     // allow up to 10 connection requests to queue
        perror("Listen");
        return 4;
    }
    while (1) {
        pthread_t threadID;
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int connectFd = accept(serv, (struct sockaddr*)&client, &len);
        clientInfo->connectFd = connectFd;
        clientInfo->client = client;
        pthread_create(&threadID, NULL, client_action, clientInfo);
        pthread_detach(threadID);
    }
    close(serv);
    return 0;
}

int main(int argc, char** argv){
    ClientInfo clientInfo;
    if (pthread_mutex_init(&clientInfo.mutex, NULL) != 0) {
        return 0;
    }
    Mapper mapper;
    mapper.numOfMap = 0;
    clientInfo.mapper = &mapper;
    service_listen(&clientInfo);
    pthread_mutex_destroy(&clientInfo.mutex);
    return 0;
}

