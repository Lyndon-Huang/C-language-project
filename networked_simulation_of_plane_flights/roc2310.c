#include "tool.h"

typedef enum {
    NORMAL_OPERATION = 0,
    WRONG_ARGS = 1,
    WRONG_MAPPER_PORT = 2,
    INVALID_PORT_NUM = 3,
    FAILED_CONNECT_MAPPER = 4,
    NO_DESTINATION = 5,
    FAILED_CONNECT_DES_PORT = 6
} Status;

/*
 * print the error message
 */
Status roc_error_message(Status status) {
    const char* message[] = {
            "",
            "Usage: roc2310 id mapper {airports}\n",
            "Invalid mapper port\n",
            "Mapper required\n",
            "Failed to connect to mapper\n",
            "No map entry for destination\n",
            "Failed to connect to at least one destination\n"
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
bool is_valid_port_range(char** argv) {
    if (atoi(argv[2]) <= 0 || atoi(argv[2]) > 65535) {
        return false;
    }
    return true;
}

/*
 * this function check whether the port is number or not
 * parameter: argc, argv
 * return true if the port include non-digit char
 *      else return false
 */
bool is_valid_port_num(int argc, char** argv) {
    if (argc < 4) {
        return false;
    }
    int index = 3;
    for (int i = 0; i < argc - 3; ++i) {
        for (int j = 0; j < strlen(argv[index]); ++j) {
            if (!isdigit(argv[index][j])) {
                return false;
            }
        }
        index++;
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
bool is_valid_mapper(int index, char** argv, RocInfo* rocInfo) {
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servAddr;
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
        rocInfo->socketFd = socketFd;
        return true;
    }
}

/*
 * this function connect to control to get the infomation
 * parameter: void* rocInformation
 * return NULL
 */
void* roc_to_control(void* rocInformation) {
    RocInfo* rocInfo = (RocInfo*)rocInformation;
    pthread_mutex_lock(&rocInfo->rocMutex);
    char* message = malloc(sizeof(char) * 1024);
    memset(message, 0, 1024);
    FILE* read = fdopen(rocInfo->socketFd, "r");
    FILE* write = fdopen(rocInfo->socketFd, "w");
    fprintf(write, "%s\n", rocInfo->argv[1]);
    fflush(write);
    fgets(message, 1023, read);
    fflush(read);
    if (strcmp(message, "") == 0) {
        exit(roc_error_message(5));
    } else {
        if (rocInfo->outputSize == 0) {
            rocInfo->outputSize += strlen(message);
            rocInfo->output = malloc(sizeof(char) * rocInfo->outputSize);
            strcat(rocInfo->output, message);
        } else {
            rocInfo->outputSize += strlen(message);
            rocInfo->output = realloc(rocInfo->output, rocInfo->outputSize);
            strcat(rocInfo->output, message);
        }
    }
    free(message);
    fclose(read);
    fclose(write);
    pthread_mutex_unlock(&rocInfo->rocMutex);
    return NULL;
}

/*
 * if the mapper port is "-", then run this function
 * parameters:int argc: the number of argvs
 *            char** argv: argument
 *            RocInfo* rocInfo
 *            int index: to get the port in argv
 * return void
 */
void no_mapper(int argc, char** argv, RocInfo* rocInfo, int index) {
    bool err = false;
    int numOfThread = argc - 3;
    pthread_t* threadIDs = malloc(sizeof(pthread_t) * numOfThread);
    for (int i = 0; i < numOfThread; ++i) {
        int socketFd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in servAddr;
        bzero(&servAddr, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(atoi(argv[index++]));
        inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr);
        rocInfo->socketFd = socketFd;
        rocInfo->serv = servAddr;
        int temp = connect(socketFd, (struct sockaddr*)&servAddr,
                sizeof(servAddr));
        if (temp < 0) {
            err = true;
        } else {
            pthread_create(&threadIDs[i], NULL, roc_to_control, rocInfo);
            pthread_join(threadIDs[i], NULL);
        }
    }
    if (err) {
        exit(roc_error_message(6));
    }
    fprintf(stdout, "%s", rocInfo->output);
    fflush(stdout);
}

/*
 * connect to the port, create thread, connect to control
 * parameters:
 *          char* message: may include port number, ";\n", ""
 *          RocInfo* rocInfo,
 *          bool flag: if true, message has port number
 *                      else, connect to argv's port
 *          int index: index of argv
 * return true if connect fail
 * else return false
 */
bool get_destinations(char* message, RocInfo* rocInfo, bool flag, int index) {
    if (strcmp(message, ";\n") == 0) {
        exit(roc_error_message(5));
    }
    bool err = false;
    pthread_t threadID;
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servAddr;
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    if (flag) {
        servAddr.sin_port = htons(atoi(message));
    } else {
        servAddr.sin_port = htons(atoi(rocInfo->argv[index]));
    }
    inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr);
    rocInfo->socketFd = socketFd;
    rocInfo->serv = servAddr;
    int connectFd = connect(socketFd, (struct sockaddr*)&servAddr,
            sizeof(servAddr));
    if (connectFd < 0) {
        err = true;
    }
    pthread_create(&threadID, NULL, roc_to_control, rocInfo);
    pthread_join(threadID, NULL);
    return err;
}

/*
 * free and close connect
 * parameters:
 *          char* message: free
 *          FILE* read, FILE* write : fclose
 *          const int* socketFd: close
 * return void
 */
void free_close(char* message, FILE* read, FILE* write, const int* socketFd) {
    free(message);
    fclose(read);
    fclose(write);
    close(*socketFd);
}

/*
 * if the mapper is not "-", then connect to mapper for get port
 * parameters:
 *          int index: the index of argv
 *          int argc: number of argv
 *          char** argv
 *          RocInfo* rocInfo
 * return void
 */
void roc_to_mapper(int index, int argc, char** argv, RocInfo* rocInfo) {
    bool err = false;
    char* message = malloc(sizeof(char) * 80);
    memset(message, 0, 80);
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servAddr;
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(atoi(argv[index]));
    inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr);
    rocInfo->socketFd = socketFd;
    rocInfo->serv = servAddr;
    connect(socketFd, (struct sockaddr*)&servAddr, sizeof(servAddr));
    FILE* read = fdopen(rocInfo->socketFd, "r");
    FILE* write = fdopen(rocInfo->socketFd, "w");
    bool isNum = true;
    for (int i = 0; i < argc - 3; ++i) {
        isNum = true;
        for (int j = 0; j < strlen(argv[3 + i]); ++j) {
            if (isalpha(argv[3 + i][j])) {
                strcat(message, "?");
                strcat(message, argv[3 + i]);
                strcat(message, "\n");
                fprintf(write, "%s", message);
                fflush(write);
                memset(message, 0, 80);
                fgets(message, 79, read);
                fflush(read);
                if (get_destinations(message, rocInfo, true,
                        3 + i)) {
                    err = true;
                }
                memset(message, 0, 80);
                isNum = false;
                break;
            }
        }
        if (isNum) {
            if (get_destinations(message, rocInfo, false,
                    3 + i)) {
                err = true;
            }
        }
    }
    if (err) {
        exit(roc_error_message(6));
    }
    fprintf(stdout, "%s", rocInfo->output);
    fflush(stdout);
    free_close(message, read, write, &socketFd);
}

int main(int argc, char** argv){
    RocInfo rocInfo;
    rocInfo.argv = argv;
    rocInfo.outputSize = 0;
    if (pthread_mutex_init(&rocInfo.rocMutex, NULL) != 0) {
        return 0;
    }
    if (!is_valid_args(argc)) {
        exit(roc_error_message(1));
    } else if (strcmp(argv[2], "-") != 0 && !is_valid_port_range(argv)) {
        exit(roc_error_message(2));
    } else if (strcmp(argv[2], "-") == 0 && !is_valid_port_num(argc, argv)){
        exit(roc_error_message(3));
    } else if (strcmp(argv[2], "-") != 0 && !is_valid_mapper(2,
            argv, &rocInfo)) {
        exit(roc_error_message(4));
    } else if (strcmp(argv[2], "-") == 0) {
        no_mapper(argc, argv, &rocInfo, 3);
    } else {
        roc_to_mapper(2, argc, argv, &rocInfo);
    }
    pthread_mutex_destroy(&rocInfo.rocMutex);
    return 0;
}