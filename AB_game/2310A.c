#include <stdio.h>
#include <stdlib.h>
#include "player.h"

/*
 * the strategy of the player A
 * board: save the player information
 */
void a_strategy(Board* board) {
    if (board->currentPlayer->position.tempPos !=
            board->currentPlayer->position.column) {
        return;
    }
    board->currentPlayer->position.oldRow =
            board->currentPlayer->position.row;
    board->currentPlayer->position.oldColumn =
            board->currentPlayer->position.column;
    if (board->currentPlayer->money != 0) {
        for (int j = board->currentPlayer->position.column + 1;
                j < board->path.pathSize / 3; ++j) {
            if (((board->path.site[j].site1 == ':' &&
                    board->path.site[j].site2 == ':')
                    && j != board->path.pathSize / 3 - 1) ||
                    (board->path.site[j].site1 == 'D' &&
                    board->path.site[j].site2 == 'o')) {
                if (board->path.site[j].occupied <
                        board->path.site[j].capacity) {
                    board->currentPlayer->position.tempPos = j;
                    return;
                }
            }
        }
    }
    if (board->path.site[board->currentPlayer->
            position.column + 1].site1 == 'M' && board->path
            .site[board->currentPlayer->position.column + 1].site2 == 'o') {
        if (board->path.site[board->currentPlayer->
                position.column + 1].occupied < board->path.site
                [board->currentPlayer->position.column + 1].capacity) {
            board->currentPlayer->position.tempPos =
                    board->currentPlayer->position.column + 1;
            return;
        }
    }
    for (int j = board->currentPlayer->position.column + 1; j <
            board->path.pathSize / 3; ++j) {
        if ((board->path.site[j].site1 == ':' && board->path.site[j].site2 ==
                ':') || (board->path.site[j].site1 == 'V' &&
                board->path.site[j]
                .site2 == '1') || (board->path.site[j].site1 == 'V' &&
                board->path.site[j].site2 == '2')) {
            if (board->path.site[j].occupied < board->path.site[j].capacity) {
                board->currentPlayer->position.tempPos = j;
                return;
            }
        }
    }
}

/*
 * use the loop to get message and run the game
 * board: the board to save info
 */
void a_run_game(Board* board) {
    while (1) {
        char* message = malloc(sizeof(char) * 50);
        int messageID = receive_dealer_message(board, &message);
        fflush(stdin);
        if (messageID == 1 || messageID == 10) {
            a_strategy(board);
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
        a_run_game(&globalBoard);
    }
}
