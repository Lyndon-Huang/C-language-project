#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include "player.h"

//player error message
PlayerError pr_message(PlayerError errorMessage) {
    const char* message[] = {
        "",
        "Usage: player pcount ID\n",
        "Invalid player count\n",
        "Invalid ID\n",
        "Invalid path\n",
        "Early game over\n",
        "Communications error\n"
    };
    fputs(message[errorMessage], stderr);
    return errorMessage;
}

//check the number of argv
bool is_valid_pl_argc(int argc) {
    return argc == 3;
}

//check the number of player
bool is_valid_pcount(char** argv) {
    return atoi(argv[1]) >= 1;
}

//check the id of player
bool is_valid_player_id(char** argv) {
    if (isdigit(argv[2][0]) > 0) {
        if (atoi(argv[2]) < atoi(argv[1])) {
            return atoi(argv[2]) >= 0;
        }
    }
    return false;
}

/*
 * check all of the arguments
 * return true when all the arguments are right
 */
bool check_argv(int argc, char** argv) {
    if (!is_valid_pl_argc(argc)) {
        exit(pr_message(PR_WRONG_NUM_ARGS));
    }
    if (!is_valid_pcount(argv)) {
        exit(pr_message(INVALID_PLAYER_COUNT));
    }
    if (!is_valid_player_id(argv)) {
        exit(pr_message(INVALID_PAYERID));
    }
    return true;
}

/*
 * check the path sent from dealer
 * if the path is valid, then initiate
 * board: for saving path
 * path: the string save path infomation
 */
void check_path(Board* board, char** path) {
    board->path.pathSize = atoi(*path) * 3;
    board->path.site = malloc(sizeof(Site) *
            (board->path.pathSize / 3));
    int pathSize = atoi(*path);
    if (pathSize < 2) {
        exit(pr_message(ERROR_PATH));
    }
    int index = 0;
    while (pathSize != 0) {
        pathSize /= 10;
        index++;
    }
    pathSize = atoi(*path);
    index++;
    if ((*path)[index] != ':' || (*path)[index + 1] != ':') {
        exit(pr_message(ERROR_PATH));
    }
    if ((*path)[pathSize * 3 - 2 + index] != ':' ||
            (*path)[pathSize * 3 - 3 + index] != ':') {
        exit(pr_message(ERROR_PATH));
    }
    for (int j = 0; j < pathSize; ++j) {
        board->path.site[j].site1 = (*path)[index];
        board->path.site[j].site2 = (*path)[index + 1];
        if ((*path)[index + 2] == '-') {
            board->path.site[j].capacity = board->numPlayer;
        } else {
            board->path.site[j].capacity = atoi(&(*path)[index + 2]);
        }
        board->path.site[j].occupied = 0;
        index += 3;
    }
}

/*
 * print the path to stdout ot stderr
 * board: provide path info
 * output: stdout or stderr
 */
void print_path(Board* board, FILE* output) {
    for (int j = 0; j < board->path.pathSize / 3; ++j) {
        fprintf(output, "%c", board->path.site[j].site1);
        fprintf(output, "%c", board->path.site[j].site2);
        fprintf(output, " ");
    }
    fprintf(output, "\n");
}

/*
 * print the players after printing path
 * board: provide path info
 * output: stdout or stderr
 */
void print_player(Board* board, FILE* output) {
    int row = 0;
    int count = 0;
    for (int i = 0; i < board->numPlayer; ++i) {
        for (int j = 0; j < board->path.pathSize / 3; ++j) {
            if (board->path.playerBoard[i][j].playerID >= 0) {
                count++;
            }
        }
        if (count > 0) {
            count = 0;
            row++;
        }
    }
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < board->path.pathSize / 3; ++j) {
            if (board->path.playerBoard[i][j].playerID != -1) {
                fprintf(output, "%d",
                        board->path.playerBoard[i][j].playerID);
                fprintf(output, " ");
                fprintf(output, " ");
            } else {
                fprintf(output, " ");
                fprintf(output, " ");
                fprintf(output, " ");
            }
        }
        fprintf(output, "\n");
    }
}

/*
 * initiate the players info to board
 * board: save the players infomation
 */
void init_player(Board* board) {
    board->path.site[0].capacity = board->numPlayer;
    board->path.site[0].occupied = board->numPlayer;
    int playerID = board->numPlayer;
    board->path.playerBoard = malloc(
            sizeof(PlayerSite*) * board->numPlayer);
    for (int j = 0; j < board->numPlayer; ++j) {
        board->path.playerBoard[j] = malloc(
                sizeof(PlayerSite) * board->path.pathSize / 3);
        for (int i = 0; i < board->path.pathSize / 3; ++i) {
            if (i == 0) {
                board->path.playerBoard[j][i].playerID = playerID - 1;
                playerID--;
            } else {
                board->path.playerBoard[j][i].playerID = -1;
            }
        }
    }
    playerID = 0;
    int position = board->numPlayer - 1;
    board->players = malloc(sizeof(Player) * board->numPlayer);
    for (int i = 0; i < board->numPlayer; ++i) {
        board->players[i].playerID = playerID;
        board->players[i].money = 7;
        board->players[i].changeMoney = 0;
        board->players[i].siteV1 = 0;
        board->players[i].siteV2 = 0;
        board->players[i].point = 0;
        board->players[i].additionalPoint = 0;
        board->players[i].card = 0;
        board->players[i].handCard.cardA = 0;
        board->players[i].handCard.cardB = 0;
        board->players[i].handCard.cardC = 0;
        board->players[i].handCard.cardD = 0;
        board->players[i].handCard.cardE = 0;
        board->players[i].handCard.numCard = 0;
        board->players[i].position.row = position;
        board->players[i].position.column = 0;
        board->players[i].position.oldRow = position;
        board->players[i].position.oldColumn = 0;
        board->players[i].pid = getpid();
        playerID++;
        position--;
    }
    board->currentPlayer = &board->players[0];
    board->labelPlayer = &board->players[0];
}

/*
 * draw one card from the deck
 * board: save the card to board
 * index: the index of player(is player ID)
 */
void draw_card(Board* board, int index) {
    char card = board->deck[board->deckIndex % board->deckSize];
    if (card == 'A') {
        board->players[index].card = 1;
    } else if (card == 'B') {
        board->players[index].card = 2;
    } else if (card == 'C') {
        board->players[index].card = 3;
    } else if (card == 'D') {
        board->players[index].card = 4;
    } else if (card == 'E') {
        board->players[index].card = 5;
    }
    board->deckIndex += 1;
}

/*
 * the action of different sites
 * board: the board to save data
 * flag: true or false, determine which action will do
 * index: determine which player(is player ID)
 */
void site_action(Board* board, bool flag, int index) {
    if (board->path.site[board->players[index].
            position.tempPos].site1 == 'M' &&
            board->path.site[board->players[index].
            position.tempPos].site2 == 'o') {
        if (!flag) {
            board->players[index].changeMoney = 3;
        }
    } else if (board->path.site[board->players[index].
            position.tempPos].site1 == 'V' &&
            board->path.site[board->players[index].
            position.tempPos].site2 == '1') {
        if (flag) {
            board->players[index].siteV1 += 1;
        }
    } else if (board->path.site[board->players[index].
            position.tempPos].site1 == 'V' &&
            board->path.site[board->players[index].
            position.tempPos].site2 == '2') {
        if (flag) {
            board->players[index].siteV2 += 1;
        }
    } else if (board->path.site[board->players[index].
            position.tempPos].site1 == 'D' &&
            board->path.site[board->players[index].
            position.tempPos].site2 == 'o') {
        if (!flag) {
            board->players[index].changeMoney = 0 -
                    board->players[index].money;
            board->players[index].additionalPoint =
                    board->players[index].money / 2;
        }
    } else if (board->path.site[board->players[index].
            position.tempPos].site1 == 'R' &&
            board->path.site[board->players[index].
            position.tempPos].site2 == 'i') {
        if (!flag) {
            draw_card(board, index);
        }
    }
}

/*
 * at the end of game, get the points based on item cards
 * board: save the info of the card
 */
void card_point(Board* board) {
    int count = 0;
    for (int i = 0; i < board->numPlayer; ++i) {
        while (board->players[i].handCard.numCard != 0) {
            if (board->players[i].handCard.cardA != 0) {
                count += 1;
                board->players[i].handCard.cardA -= 1;
                board->players[i].handCard.numCard -= 1;
            }
            if (board->players[i].handCard.cardB != 0) {
                count += 1;
                board->players[i].handCard.cardB -= 1;
                board->players[i].handCard.numCard -= 1;
            }
            if (board->players[i].handCard.cardC != 0) {
                count += 1;
                board->players[i].handCard.cardC -= 1;
                board->players[i].handCard.numCard -= 1;
            }
            if (board->players[i].handCard.cardD != 0) {
                count += 1;
                board->players[i].handCard.cardD -= 1;
                board->players[i].handCard.numCard -= 1;
            }
            if (board->players[i].handCard.cardE != 0) {
                count += 1;
                board->players[i].handCard.cardE -= 1;
                board->players[i].handCard.numCard -= 1;
            }
            if (count == 5) {
                board->players[i].point += 10;
            } else if (count == 4) {
                board->players[i].point += 7;
            } else if (count == 3) {
                board->players[i].point += 5;
            } else if (count == 2) {
                board->players[i].point += 3;
            } else if (count == 1) {
                board->players[i].point += 1;
            }
            count = 0;
        }
    }
}

/*
 * determine the next player
 * board: provide the position of players
 */
void who_turn(Board* board) {
    Player* nextPlayer = board->currentPlayer;
    for (int i = 0; i < board->numPlayer; ++i) {
        if (board->players[i].position.column ==
                nextPlayer->position.column) {
            if (board->players[i].position.row >
                    nextPlayer->position.row) {
                nextPlayer = &board->players[i];
            }
        } else if (board->players[i].position.column <
                nextPlayer->position.column) {
            nextPlayer = &board->players[i];
        }
    }
    board->currentPlayer = nextPlayer;
}

/*
 * clear the variables
 */
void clear_var(Board* board, int index) {
    board->players[index].changeMoney = 0;
    board->players[index].additionalPoint = 0;
    board->players[index].card = 0;
}

/*
 * deal with the HAP message
 * board: save data
 * p, n, s, m, s: the data after HAP
 * return 4 if message is valid
 */
int hap_message(Board* board, int p, int n, int s, int m, int c) {
    if (p >= board->numPlayer || p < 0) {
        exit(pr_message(PRCOMMUNICATION_ERROR));
    } else if (n >= (board->path.pathSize / 3) || n < 0) {
        exit(pr_message(PRCOMMUNICATION_ERROR));
    } else if (c < 0 || c > 6) {
        exit(pr_message(PRCOMMUNICATION_ERROR));
    } else {
        board->players[p].position.tempPos = n;
        site_action(board, true, p);
        board->players[p].point += s;
        if (m != INT_MAX) {
            board->players[p].money += m;
        } else {
            exit(pr_message(PRCOMMUNICATION_ERROR));
        }
        if (c < 6 && c >= 0) {
            if (c == 1) {
                board->players[p].handCard.cardA += 1;
                board->players[p].handCard.numCard += 1;
            } else if (c == 2) {
                board->players[p].handCard.cardB += 1;
                board->players[p].handCard.numCard += 1;
            } else if (c == 3) {
                board->players[p].handCard.cardC += 1;
                board->players[p].handCard.numCard += 1;
            } else if (c == 4) {
                board->players[p].handCard.cardD += 1;
                board->players[p].handCard.numCard += 1;
            } else if (c == 5) {
                board->players[p].handCard.cardE += 1;
                board->players[p].handCard.numCard += 1;
            }
        } else {
            exit(pr_message(PRCOMMUNICATION_ERROR));
        }
        update_all(board, p);
        clear_var(board, p);
        return 4;
    }
}

/*
 * update the position of the player, include other informations
 * board: provide the info about players
 * index: determine which player(is player ID)
 */
void update_player_board(Board* board, int index) {
    int oldPosRow = board->players[index].position.oldRow;
    int oldPosColumn = board->players[index].position.oldColumn;
    int newRow = board->players[index].position.row;
    int newColumn = board->players[index].position.column;
    board->path.playerBoard[oldPosRow][oldPosColumn].playerID = -1;
    board->path.playerBoard[newRow][newColumn].playerID =
            board->players[index].playerID;
}

/*
 * update the infomation of the player board
 * board: provide the info about players
 * index: determine which player(is player ID)
 */
void update_all(Board* board, int index) {
    int tempPos = board->players[index].position.tempPos;
    board->players[index].position.oldRow =
            board->players[index].position.row;
    board->players[index].position.row = board->path.site[tempPos].occupied;
    board->path.site[tempPos].occupied += 1;
    board->players[index].position.oldColumn =
            board->players[index].position.column;
    board->path.site[board->players[index].position.oldColumn].occupied -= 1;
    board->players[index].position.column = tempPos;
    update_player_board(board, index);
}

/*
 * receive the message from the dealer
 * board: save the data
 * message: a string to hold the message from dealer
 * return 1, 2, 3, 4
 *      1 for YT
 *      2 for EARLY
 *      3 for DONE
 *      4 for HAPp,n,s,m,c
 */
int receive_dealer_message(Board* board, char** message) {
    int p = 0, n = 0, s = 0, m = INT_MAX, c = 6;
    char chars = 0;
    if (fgets(*message, 49, stdin) == NULL) {
        exit(pr_message(PRCOMMUNICATION_ERROR));
    } else {
        if (strncmp(*message, "YT", 2) == 0) {
            sscanf(*message, "YT%c", &chars);
            if (chars != '\n') {
                if (chars == ' ') {
                    return 10;
                } else {
                    exit(pr_message(PRCOMMUNICATION_ERROR));
                }
            } else {
                return 1;
            }
        } else if (strncmp(*message, "EARLY", 5) == 0) {
            sscanf(*message, "EARLY%c", &chars);
            if (chars != '\n') {
                exit(pr_message(PRCOMMUNICATION_ERROR));
            } else {
                return 2;
            }
        } else if (strncmp(*message, "DONE", 4) == 0) {
            sscanf(*message, "DONE%c", &chars);
            if (chars != '\n') {
                exit(pr_message(PRCOMMUNICATION_ERROR));
            } else {
                return 3;
            }
        } else if (strncmp(*message, "HAP", 3) == 0) {
            sscanf(*message, "HAP%d,%d,%d,%d,%d", &p, &n, &s, &m, &c);
            return hap_message(board, p, n, s, m, c);
        } else {
            exit(pr_message(PRCOMMUNICATION_ERROR));
        }
    }
}

/*
 * check the game is over or not
 * game over when the last site is full
 * return true when the site full, otherwise false
 */
bool is_game_over(Board* board) {
    if (board->path.site[board->path.pathSize / 3 - 1].occupied ==
            board->numPlayer) {
        return true;
    }
    return false;
}

/*
 * print the message of players
 * board: provide the information
 * output: stdout or stderr
 */
void dealer_out(Board* board, FILE* output) {
    fprintf(output, "Player %d Money=%d V1=%d V2=%d Points=%d "
            "A=%d B=%d C=%d D=%d E=%d\n",
            board->currentPlayer->playerID, board->currentPlayer->money,
            board->currentPlayer->siteV1, board->currentPlayer->siteV2,
            board->currentPlayer->point, board->currentPlayer->handCard.cardA,
            board->currentPlayer->handCard.cardB,
            board->currentPlayer->handCard.cardC,
            board->currentPlayer->handCard.cardD,
            board->currentPlayer->handCard.cardE);
}

/*
 * print the final scores
 * board: provide the information
 * output: stdout or stderr
 */
void final_scores(Board* board, FILE* output) {
    for (int i = 0; i < board->numPlayer; ++i) {
        board->players[i].point += board->players[i].siteV1 +
                board->players[i].siteV2;
    }
    card_point(board);
    fprintf(output, "Scores: ");
    for (int j = 0; j < board->numPlayer - 1; ++j) {
        fprintf(output, "%d,", board->players[j].point);
    }
    fprintf(output, "%d\n", board->players
            [board->numPlayer - 1].point);
}

