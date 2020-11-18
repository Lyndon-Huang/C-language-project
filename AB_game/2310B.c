#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "player.h"

/*
 * sum the cards for each player
 * board: provide the player hand card info
 */
void cal_num_hand_card(Board* board) {
    for (int i = 0; i < board->numPlayer; ++i) {
        board->players[i].handCard.numCard =
                board->players[i].handCard.cardA +
                board->players[i].handCard.cardB +
                board->players[i].handCard.cardC +
                board->players[i].handCard.cardD +
                board->players[i].handCard.cardE;
    }
}

/*
 * the current player has max hand card
 * board: provide the player hand card info
 */
bool max_hand_card(Board* board) {
    int count = 0;
    cal_num_hand_card(board);
    board->currentPlayer->handCard.numCard =
            board->currentPlayer->handCard.cardA +
            board->currentPlayer->handCard.cardB +
            board->currentPlayer->handCard.cardC +
            board->currentPlayer->handCard.cardD +
            board->currentPlayer->handCard.cardE;
    for (int i = 0; i < board->numPlayer; ++i) {
        if (board->players[i].playerID != board->currentPlayer->playerID) {
            if (board->currentPlayer->handCard.numCard >
                    board->players[i].handCard.numCard) {
                count++;
            }
        }
    }
    if (count == board->numPlayer - 1) {
        return true;
    } else {
        return false;
    }
}

/*
 * the rest of player no card
 * board: provide the player info
 */
bool zero_card(Board* board) {
    cal_num_hand_card(board);
    for (int i = 0; i < board->numPlayer; ++i) {
        if (i != board->currentPlayer->playerID) {
            if (board->players[i].handCard.numCard != 0) {
                return false;
            }
        }
    }
    return true;
}

/*
 * check the site in the path
 * position: where to start, not start from 0
 * site: the site want to find
 * limit: where to stop, when meet "::" site
 */
bool check_site(Board* board, int position, const char* site, int limit) {
    for (int i = position + 1; i < limit; ++i) {
        if ((board->path.site[i].site1 == ':' &&
                board->path.site[i].site2 == ':') ||
                (board->path.site[i].site1 == site[0] &&
                board->path.site[i].site2 == site[1])) {
            if (board->path.site[i].occupied <
                    board->path.site[i].capacity) {
                if (board->path.site[i].site1 == ':' &&
                        board->path.site[i].site2 == ':') {
                    if (check_site(board, position, site, i)) {
                        board->currentPlayer->position.tempPos = i;
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    board->currentPlayer->position.tempPos = i;
                    return true;
                }
            }
        }
    }
    return false;
}

/*
 * the strategy of player B
 * board: save the information of players
 */
void b_strategy(Board* board) {
    board->currentPlayer->position.oldRow =
            board->currentPlayer->position.row;
    board->currentPlayer->position.oldColumn =
            board->currentPlayer->position.column;
    int oldPosColumn = board->currentPlayer->position.oldColumn;
    int count = 0;
    if (board->path.site[oldPosColumn + 1].occupied <
            board->path.site[oldPosColumn + 1].capacity) {
        for (int i = 0; i < board->numPlayer; ++i) {
            if (i != board->currentPlayer->playerID) {
                if (board->currentPlayer->position.column <
                        board->players[i].position.column) {
                    count++;
                } else {
                    break;
                }
            }
        }
        if (count == board->numPlayer - 1) {
            board->currentPlayer->position.tempPos =
                    board->currentPlayer->position.column + 1;
            return;
        }
    }
    if (board->currentPlayer->money % 2 != 0) {
        if (check_site(board, board->currentPlayer->position.column,
                "Mo", (board->path.pathSize / 3 - 1))) {
            return;
        }
    }
    if (max_hand_card(board) || zero_card(board)) {
        if (check_site(board, board->currentPlayer->position.column,
                "Ri", (board->path.pathSize / 3 - 1))) {
            return;
        }
    }
    if (check_site(board, board->currentPlayer->position.column,
            "V2", (board->path.pathSize / 3 - 1))) {
        return;
    }
    for (int i = oldPosColumn + 1; i < board->path.pathSize / 3; ++i) {
        if (board->path.site[i].occupied < board->path.site[i].capacity) {
            board->currentPlayer->position.tempPos = i;
            return;
        }
    }
}

/*
 * use the loop to get message and run the game
 * board: the board to save info
 */
void b_run_game(Board* board) {
    while (1) {
        char* message = malloc(sizeof(char) * 50);
        int messageID = receive_dealer_message(board, &message);
        fflush(stdin);
        if (messageID == 1 || messageID == 10) {
            b_strategy(board);
            fprintf(stdout, "DO%d",
                    board->currentPlayer->position.tempPos);
            fprintf(stdout, "\n");
            fflush(stdout);
            if (messageID == 10) {
                exit(pr_message(PRCOMMUNICATION_ERROR));
            }
            if (board->currentPlayer->playerID !=
                    board->labelPlayer->playerID) {
                exit(pr_message(PRCOMMUNICATION_ERROR));
            }
        } else if (messageID == 2) {
            exit(pr_message(EARLY_GAMEOVER));
        } else if (messageID == 3) {
            final_scores(board, stderr);
            exit(pr_message(PR_NORMAL_END));
        } else if (messageID == 4) {
            dealer_out(board, stderr);
            print_path(board, stderr);
            print_player(board, stderr);
            who_turn(board);
        }
        free(message);
    }
}

int main(int argc, char** argv) {
    if (check_argv(argc, argv)) {
        fprintf(stdout, "^");
        fflush(stdout);
        char* path = malloc(sizeof(char) * 255);
        if (fgets(path, 254, stdin) == NULL) {
            exit(pr_message(ERROR_PATH));
        }
        fflush(stdin);
        if (!globalBoard.initFlag) {
            globalBoard.numPlayer = atoi(argv[1]);
            check_path(&globalBoard, &path);
            init_player(&globalBoard);
        }
        for (int i = 0; i < globalBoard.numPlayer; ++i) {
            if (globalBoard.players[i].playerID == atoi(argv[2])) {
                globalBoard.labelPlayer = &globalBoard.players[i];
            }
        }
        print_path(&globalBoard, stderr);
        print_player(&globalBoard, stderr);
        b_run_game(&globalBoard);
    }
}