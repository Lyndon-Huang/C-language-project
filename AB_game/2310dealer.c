#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "player.h"

typedef enum {
    NORMAL_END = 0,
    WRONG_NUM_ARGS = 1,
    INVALID_DECK = 2,
    INVALID_PATH = 3,
    ERROR_PLAYERS_START = 4,
    COMMUNICATION_ERROR = 5,
} DealerError;

//the dealer error message
DealerError dr_message(DealerError errorMessage) {
    const char* message[] = {
        "",
        "Usage: 2310dealer deck path p1 {p2}\n",
        "Error reading deck\n",
        "Error reading path\n",
        "Error starting process\n",
        "Communications error\n"
    };
    fputs(message[errorMessage], stderr);
    return errorMessage;
}

//check the number of args
bool is_valid_args(int argc) {
    return argc > 3;
}

//signal handler
void dealer_handler() {
    for (int i = 0; i < globalBoard.numPlayer; i++) {
        if (globalBoard.players[i].pid != 0) {
            kill(globalBoard.players[i].pid, 9);
        }
        waitpid(globalBoard.players[i].pid, 0, 0);
    }
    exit(99);
}

/*
 * set the pipe signal
 * set the SIGHUP
 */
void signal_set() {
    struct sigaction sigPipe;
    memset(&sigPipe, 0, sizeof(struct sigaction));
    sigPipe.sa_handler = SIG_IGN;
    sigPipe.sa_flags = SA_RESTART;
    sigaction(SIGPIPE, &sigPipe, NULL);

    struct sigaction dealerHup;
    memset(&dealerHup, 0, sizeof(struct sigaction));
    dealerHup.sa_handler = dealer_handler;
    dealerHup.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &dealerHup, NULL);
}

/*
 * load the deck from the deck file
 *
 * parameters:
 * board: saving the game data(player info, path info)
 */
void load_deck(Board* board, char** argv) {
    char ch;
    FILE* deckFile = fopen(argv[1], "r");
    if (deckFile == NULL) {
        exit(dr_message(INVALID_DECK));
    }
    char* tempString = malloc(sizeof(char) * 20); //str for get the number
    fgets(tempString, 18, deckFile);
    fclose(deckFile);
    board->deckSize = atoi(tempString);
    board->deckIndex = 0;
    if (board->deckSize < 4) {
        exit(dr_message(INVALID_DECK));
    }
    int index = 0;
    int deckSize = board->deckSize;
    while (deckSize != 0) {
        deckSize /= 10;
        index++;
    }
    deckFile = fopen(argv[1], "r");
    board->deck = malloc(sizeof(char) * (board->deckSize + 1));
    for (int k = 0; k < index; ++k) {
        getc(deckFile);
    }
    for (int j = 0; j < board->deckSize; ++j) {
        if ((ch = fgetc(deckFile)) != EOF) {
            if (ch >= 'A' && ch <= 'E') {
                board->deck[j] = (char)ch;
            } else {
                exit(dr_message(INVALID_DECK));
            }
        }
    }
    if (getc(deckFile) != '\n') {
        exit(dr_message(INVALID_DECK));
    }
    fclose(deckFile);
    free(tempString);
}

/*
 * put the sites to the path
 * parameter:
 * board: save the path
 * tempPath: string for saving the path
 * index: skip the number & ;
 */
void init_path(Board* board, char* tempPath, int* index) {
    board->path.site = malloc(sizeof(Site) *
            (board->path.pathSize / 3));
    *index += 1;
    for (int j = 0; j < board->path.pathSize / 3; ++j) {
        board->path.site[j].site1 = tempPath[*index];
        board->path.site[j].site2 = tempPath[*index + 1];
        if (tempPath[*index] == ':') {
            board->path.site[j].capacity = board->numPlayer;
        } else {
            board->path.site[j].capacity = atoi(
                    &tempPath[*index + 2]);
        }
        board->path.site[j].occupied = 0;
        *index += 3;
    }
    free(tempPath);
}

/*
 * read the path from the path file
 * parameter:
 * board: board for saving data
 * argv: save the name of the path file
 */
void load_path(Board* board, char** argv) {
    int index = 0;
    int pathSize = 0;
    FILE* pathFile = fopen(argv[2], "r");
    if (pathFile == NULL) {
        exit(dr_message(INVALID_PATH));
    }
    char* tempString = malloc(sizeof(char) * 20); //str for get the number
    fgets(tempString, 18, pathFile);
    fclose(pathFile);
    pathFile = fopen(argv[2], "r");
    char ch;
    pathSize = atoi(tempString);
    board->path.pathSize = pathSize * 3;
    if (board->path.pathSize < 2 * 3) {      //site with limit > 1
        exit(dr_message(INVALID_PATH));
    }
    while (pathSize != 0) {
        pathSize /= 10;
        index++;
    }
    char* tempPath = malloc(sizeof(char) *
            (board->path.pathSize + index + 1));
    for (int j = 0; j < board->path.pathSize + index + 1; ++j) {
        if ((ch = fgetc(pathFile)) != EOF) {
            tempPath[j] = (char)ch;
        }
    }
    if (tempPath[index + 1] != ':' || tempPath[index + 2] != ':') {
        exit(dr_message(INVALID_PATH));
    }
    if (tempPath[board->path.pathSize + index - 1] != ':' ||
            tempPath[board->path.pathSize + index - 2] != ':') {
        exit(dr_message(INVALID_PATH));
    }
    if (getc(pathFile) != '\n') {
        exit(dr_message(INVALID_PATH));
    }
    fclose(pathFile);
    init_path(board, tempPath, &index);
    free(tempString);
}

/*
 * send the path to the player
 * parameters:
 * board: save data
 * argv: path file name
 * index: send to which player
 */
void send_path(Board* board, char** argv, int index) {
    char* path = malloc(sizeof(char) * 255);
    FILE* pathFile = fopen(argv[2], "r");
    fgets(path, 254, pathFile);
    fprintf(board->players[index].write, "%s", path);
    fflush(board->players[index].write);
    free(path);
}

/*
 * receive message from players
 * board: save data
 * message: from players
 */
bool receive_player_message(Board* board, char** message) {
    if (fgets(*message, 19, board->currentPlayer->read) == NULL) {
        fprintf(board->currentPlayer->write, "EARLY");
        fflush(board->currentPlayer->write);
        exit(dr_message(COMMUNICATION_ERROR));
    } else {
        if (strncmp(*message, "DO", 2) == 0) {
            sscanf(*message, "DO%d",
                    &board->currentPlayer->position.tempPos);
            if (board->currentPlayer->position.tempPos >
                    board->path.pathSize / 3 - 1) {
                fprintf(board->currentPlayer->write, "EARLY");
                fflush(board->currentPlayer->write);
                exit(dr_message(COMMUNICATION_ERROR));
            }
        } else {
            fprintf(board->currentPlayer->write, "EARLY");
            fflush(board->currentPlayer->write);
            exit(dr_message(COMMUNICATION_ERROR));
        }
    }
    if (strlen(*message) == 0) {
        exit(dr_message(COMMUNICATION_ERROR));
    } else {
        return true;
    }
}

/*
 * creat the pipe for send communication
 * board: save data
 * argv: determine 2310A or 2310B
 */
void creat_pipe(Board* board, char** argv) {
    char numPlayer[12], playerID[12], temp[12];
    sprintf(temp, "%d", board->numPlayer);
    sscanf(temp, "%s", numPlayer);
    memset(temp, 0, sizeof(temp));
    for (int i = 0; i < board->numPlayer; ++i) {
        sprintf(playerID, "%d", board->players[i].playerID);
        sscanf(temp, "%s", playerID);
        memset(temp, 0, sizeof(temp));
        int dealerToPl[2], plToDealer[2];
        if (!pipe(dealerToPl) && !pipe(plToDealer)) {
            pid_t pid = fork();
            if (pid < 0) {
                fprintf(stderr, "fork error!\n");
                exit(0);
            } else if (pid == 0) { //children
                dup2(dealerToPl[0], 0);
                dup2(plToDealer[1], 1);
                int fdError = open("/dev/null", O_WRONLY, S_IWUSR);
                dup2(fdError, 2);
                close(dealerToPl[1]);
                close(plToDealer[0]);
                if (access((argv + 3)[i], 0) != 0) {
                    exit(99);
                }
                if (execlp((argv + 3)[i], (argv + 3)[i],
                        numPlayer, playerID, NULL) < 0) {
                    exit(98);
                }
                exit(97);
            } else { //parent
                close(dealerToPl[0]);
                close(plToDealer[1]);
                board->players[i].pid = pid;
                board->players[i].write =
                        fdopen(dealerToPl[1], "w");
                board->players[i].read =
                        fdopen(plToDealer[0], "r");
            }
            if (fgetc(board->players[i].read) != '^') {
                for (int j = 0; j < i; ++j) {
                    kill(board->players[j].pid, SIGKILL);
                }
                exit(dr_message(ERROR_PLAYERS_START));
            } else {
                fflush(board->players[i].read);
                send_path(board, argv, i);
            }
        }
    }
}

/*
 * start the game
 * board: save data
 */
void gaming(Board* board) {
    print_path(board, stdout);
    print_player(board, stdout);
    char* message = malloc(sizeof(char) * 20); //hold "DOn"
    memset(message, 0, 20 * sizeof(char));
    bool isGameOver = false;
    while(!isGameOver) {
        fprintf(board->currentPlayer->write, "YT\n");
        fflush(board->currentPlayer->write);
        if(receive_player_message(board, &message)) {
            site_action(board, false, board->currentPlayer->playerID);
            for (int i = 0; i < board->numPlayer; ++i) {
                fprintf(board->players[i].write, "HAP%d,%d,%d,%d,%d",
                        board->currentPlayer->playerID,
                        board->currentPlayer->position.tempPos,
                        board->currentPlayer->additionalPoint,
                        board->currentPlayer->changeMoney,
                        board->currentPlayer->card);
                fprintf(board->players[i].write, "\n");
                fflush(board->players[i].write);
            }
            hap_message(board, board->currentPlayer->playerID,
                    board->currentPlayer->position.tempPos,
                    board->currentPlayer->additionalPoint,
                    board->currentPlayer->changeMoney,
                    board->currentPlayer->card);
            dealer_out(board, stdout);
            print_path(board, stdout);
            print_player(board, stdout);
            who_turn(board);
        }
        if (is_game_over(board)) {
            isGameOver = true;
            for (int i = 0; i < board->numPlayer; ++i) {
                fprintf(board->currentPlayer->write, "DONE");
                fprintf(board->currentPlayer->write, "\n");
                fflush(board->currentPlayer->write);
            }
        }
    }
}

int main(int argc, char** argv) {
    if (!is_valid_args(argc)) {
        exit(dr_message(WRONG_NUM_ARGS));
    } else {
        load_deck(&globalBoard, argv);
        load_path(&globalBoard, argv);
        globalBoard.numPlayer = argc - 3;
        init_player(&globalBoard);
        globalBoard.initFlag = true;
        signal_set();
        creat_pipe(&globalBoard, argv);
        gaming(&globalBoard);
    }
    final_scores(&globalBoard, stdout);
    return dr_message(NORMAL_END);
}

