#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DOT '.'
#define BLANK ' '
#define PLAYERO 'O'
#define PLAYERX 'X'


typedef enum {
    GG = 0,
    WRONGARGE = 1,
    WRONGPLAYER = 2,
    CANNOTREADFILE = 3,
    WRONGFILECONTENTS = 4,
    ENDOFFILE = 5,
    NOCELLS = 6,
} Status;

/*
 * parameter: status
 * return the stderr message
 */
Status message(Status status) {
    const char* messages[] = {"",
            "Usage: push2310 typeO typeX fname\n",
            "Invalid player type\n",
            "No file to load from\n",
            "Invalid file contents\n",
            "End of file\n",
            "Full board in load\n"};
    fputs(messages[status], stderr);
    return status;
}

typedef struct {
    char number;
    char playerPos;
} Cell;

/*
 * the structure of the board
 */
typedef struct {
    int row, column;        // the row and column of the board
    int humanRow, humanColumn;  //the human player input row and column
    int computerRow, computerColumn;//the computer player input row and column
    int scoreO, scoreX;     //the score of X and O
    char player;            //the initail player
    Cell** grid;            //the grid of the board
} Board;

/*
 * parameter: the pointer of the board
 * calculate the final score of player
 */
void sum_score(Board* board) {

    int scoreO = 0;
    int scoreX = 0;
    for (int i = 1; i <= (board->row - 2); ++i) {
        for (int j = 1; j <= (board->column - 2); ++j) {
            if (board->grid[i][j].playerPos == PLAYERX) {
                scoreX += board->grid[i][j].number;
            } else {
                scoreO += board->grid[i][j].number;
            }
        }
    }
    if (scoreO > scoreX) {
        fprintf(stdout, "Winners: %c\n", PLAYERO);
    } else if (scoreO < scoreX) {
        fprintf(stdout, "Winners: %c\n", PLAYERX);
    } else {
        fprintf(stdout, "Winners: %c %c\n", PLAYERO, PLAYERX);
    }
    exit(message(0));
}

/*
 * parameter: the pointer of the board
 * check whether the board is full
 * return bool(full return true, else return false)
 */
bool check_full_board(Board* board) {
    unsigned int count = 0;
    unsigned int spaceNum = (board->row - 2) * (board->column - 2);
    for (int i = 1; i <= (board->row - 2); ++i) {
        for (int j = 1; j <= (board->column - 2); ++j) {
            if (board->grid[i][j].playerPos != DOT) {
                count++;
            }
        }
    }
    if (count == spaceNum) {
        return false;
    } else {
        return true;
    }
}

/*
 * parameter: the pointer of board
 * print the board
 * return void
 */
void print_board(Board* board) {
    int height = board->row;
    int width = board->column;
    for(int h = 0; h < height; h++) {
        for(int w = 0; w < width; w++) {
            printf("%c", board->grid[h][w].number);
            printf("%c", board->grid[h][w].playerPos);
        }
        printf("\n");
    }
}

/*
 * parameter: the unsigned number -> str
 * return: the pointer of sting
 */
char* unint_to_str(unsigned int number) {
    char* str = malloc(sizeof(char) * 20);
    char* buffer = malloc(sizeof(char) * 20);
    int i = 0;
    int len = 0;
    if (str == NULL) {
        return NULL;
    }
    while(number) {
        buffer[i++] = (number % 10) + '0';
        number = number / 10;
    }
    str[i] = '\0';
    while(i >= 0) {
        i--;
        str[len++] = buffer[i];
    }
    free(buffer);
    return str;
}

/*
 * parameter: Board * board, char * fileName, char player
 * save the board to the file
 */
bool save_game(Board* board, char* fileName, char player) {
    FILE* saveFile = fopen(fileName, "w");
    if (unint_to_str(board->row) == NULL) {
        fprintf(stderr, "Save failed\n");
        return false;
    }
    if (unint_to_str(board->column) == NULL) {
        fprintf(stderr, "Save failed\n");
        return false;
    }
    char* holdPlayer = &player;
    fputs(unint_to_str(board->row), saveFile);
    fputs(" ", saveFile);
    fputs(unint_to_str(board->column), saveFile);
    fputs("\n", saveFile);
    fputs(holdPlayer, saveFile);
    fputs("\n", saveFile);
    for (int i = 0; i < board->row; ++i) {
        for (int j = 0; j < board->column; ++j) {
            putc(board->grid[i][j].number, saveFile);
            putc(board->grid[i][j].playerPos, saveFile);
        }
        putc('\n', saveFile);
    }
    fclose(saveFile);
    return true;
}

/*
 * ask the input from the human player
 * parameter: Board* board, char* input, char player
 * return void
 */
void ask_input(Board* board, char* input, char player) {
    memset(input, 0, strlen(input));   // clear input
    //the maximum size of file name
    char* saveFileName = malloc(sizeof(char) * 255);
    printf("%c:(R C)> ", player);
    fgets(input, 49, stdin);
    if (strlen(input) == 0) {      // input: control D
        exit(message(5));
    }
    while (sscanf(input, "%d %d", &board->humanRow,
            &board->humanColumn) != 2 && input[0] != 's') {
        memset(input, 0, strlen(input));// clear input
        printf("%c:(R C)> ", player);
        fgets(input, 49, stdin);
        if (strlen(input) == 0) {    // input: control D
            exit(message(5));
        }
    }
    if(input[0] == 's') {
        sscanf(input, "s%s", saveFileName);
        //not allow "/" inside file name
        if (strstr(saveFileName, "/") != NULL) {
            fprintf(stderr, "Save failed\n");
            memset(saveFileName, 0, strlen(saveFileName));//clear buffer
        }
        if (strlen(saveFileName) == 0) {// check file name is empty or not
            printf("%c:(R C)> ", player);
            fgets(input, 49, stdin);
            sscanf(input, "s%s", saveFileName);
            if (strlen(saveFileName) == 0) {
                exit(message(ENDOFFILE));
            }
            if (strstr(saveFileName, "/") != NULL) {
                exit(message(ENDOFFILE));
            }
        }
        //if cannot save the file, continue the game
        if (!save_game(board, saveFileName, player)) {
            ask_input(board, input, player);
        }
        //clear file name buffer
        memset(saveFileName, 0, strlen(saveFileName));
    }
    free(saveFileName);
}

/*
 * compare the score when the player move the stone
 * parameter: Board* board, char* numBuffer,
 *            char* playerBuffer, char* currentPlayer
 * if the score is larger, return true, else, false
 */
bool compare_score(Board* board, char* numBuffer,
        char* playerBuffer, char* currentPlayer) {
    int oldXScore1 = 0;     //to hold the old score X
    int oldOScore1 = 0;     //to hold the old score O
    board->scoreO = 0;
    board->scoreX = 0;
    unsigned int j = 0;
    for (unsigned int i = 0; i < strlen(numBuffer); ++i) {//save the oldscore
        if (playerBuffer[i] == PLAYERX) {
            oldXScore1 += numBuffer[i] - '0';
        } else if (playerBuffer[i] == PLAYERO) {
            oldOScore1 += numBuffer[i] - '0';
        }
    }
    for (; j < strlen(numBuffer); ++j) {//save the new score
        if (playerBuffer[j] == PLAYERX) {
            board->scoreX += numBuffer[j + 1] - '0';
        } else if (playerBuffer[j] == PLAYERO) {
            board->scoreO += numBuffer[j + 1] - '0';
        } else if (playerBuffer[j] == DOT) {
            j++;
            break;
        }
    }
    while(j != strlen(numBuffer)) {//if meet '.', count like old score
        for (; j < strlen(numBuffer); ++j) {
            if (playerBuffer[j] == PLAYERX) {
                board->scoreX += numBuffer[j] - '0';
            } else if (playerBuffer[j] == PLAYERO) {
                board->scoreO += numBuffer[j] - '0';
            } else {
                j++;
                break;
            }
        }
    }
    if (*currentPlayer == PLAYERX) {
        if (board->scoreX > oldXScore1 && board->scoreO < oldOScore1) {
            return true;
        } else {
            return false;
        }
    } else {
        if (board->scoreO > oldOScore1 && board->scoreX < oldXScore1) {
            return true;
        } else {
            return false;
        }
    }
}

/*
 * when the stone's row is 0, do the action
 * parameter: Board* board, int* row, int* column, char* currentPlayer
 * return: if action work return 1, else 0
 */
int stone_row_0(Board* board, int* row, int* column, char* currentPlayer) {
    char* playerBuffer = malloc(sizeof(char) * (board->row + 1));
    char* numBuffer = malloc(sizeof(char) * (board->row + 1));
    for (int i = 0; i < board->row; ++i) {
        playerBuffer[i] = board->grid[i][*column].playerPos;
        numBuffer[i] = board->grid[i][*column].number;
    }
    playerBuffer[*row] = *currentPlayer;
    if (compare_score(board, numBuffer,
            playerBuffer, currentPlayer)) {
        for (int j = 0; j < (board->row - 1); ++j) {
            if (playerBuffer[j] != DOT) {
                board->grid[j + 1]
                        [*column].playerPos = playerBuffer[j];
            } else {
                break;
            }
        }
        free(playerBuffer);
        free(numBuffer);
        return 1;
    }
    return 0;
}

/*
 * when the stone's column is 0, do the action
 * parameter: Board* board, int* row, int* column, char* currentPlayer
 * return: if action work return 1, else 0
 */
int stone_column_0(Board* board, int* row, int* column, char* currentPlayer) {
    char* playerBuffer = malloc(sizeof(char) * (board->column + 1));
    char* numBuffer = malloc(sizeof(char) * (board->column + 1));
    for (int i = 0; i < board->column; ++i) {
        playerBuffer[i] = board->grid[*row][i].playerPos;
        numBuffer[i] = board->grid[*row][i].number;
    }
    playerBuffer[*column] = *currentPlayer;
    if (compare_score(board, numBuffer,
            playerBuffer, currentPlayer)) {
        for (int j = 0; j < (board->column - 1); ++j) {
            if (playerBuffer[j] != DOT) {
                board->grid[*row][j + 1].playerPos = playerBuffer[j];
            } else {
                break;
            }
        }
        free(playerBuffer);
        free(numBuffer);
        return 1;
    }
    return 0;
}

/*
 * when the stone's row is last row, do the action
 * parameter: Board* board, int* row, int* column, char* currentPlayer
 * return: if action work return 1, else 0
 */
int stone_last_row(Board* board, int* column, char* currentPlayer) {
    char* playerBuffer = malloc(sizeof(char) * (board->row + 1));
    char* numBuffer = malloc(sizeof(char) * (board->row + 1));
    int index = 0;
    for (int i = board->row - 1; i >= 0; --i) {
        playerBuffer[index] = board->grid[i][*column].playerPos;
        numBuffer[index] = board->grid[i][*column].number;
        index++;
    }
    playerBuffer[0] = *currentPlayer;
    if (compare_score(board, numBuffer,
            playerBuffer, currentPlayer)) {
        int index = board->row - 2;
        for (int j = 0; j < board->row - 1; ++j) {
            if (playerBuffer[j] != DOT) {
                board->grid[index]
                        [*column].playerPos = playerBuffer[j];
                index--;
            } else {
                break;
            }
        }
        free(playerBuffer);
        free(numBuffer);
        return 1;
    }
    return 0;
}

/*
 * when the stone's column is rightmost, do the action
 * parameter: Board* board, int* row, int* column, char* currentPlayer
 * return: if action work return 1, else 0
 */
int stone_rightmost_column(Board* board, int* row,
        char* currentPlayer) {
    char* playerBuffer = malloc(sizeof(char) * (board->column - 1));
    char* numBuffer = malloc(sizeof(char) * (board->column - 1));
    int index = 0;
    for (int i = board->column - 1; i >= 0; --i) {
        playerBuffer[index] = board->grid[*row][i].playerPos;
        numBuffer[index] = board->grid[*row][i].number;
        index++;
    }
    playerBuffer[0] = *currentPlayer;
    if (compare_score(board, numBuffer,
            playerBuffer, currentPlayer)) {
        int index = board->column - 2;
        for (int j = 0; j < (board->column - 1); ++j) {
            if (playerBuffer[j] != DOT) {
                board->grid[*row][index].playerPos = playerBuffer[j];
                index--;
            } else {
                break;
            }
        }
        free(playerBuffer);
        free(numBuffer);
        return 1;
    }
    return 0;
}


/*
 * parameter: Board * board, int row, int column, char player, char
 * when the player put the stone, the board need to check and do the action
 * There are four situations of stone(top, left, right, bottom)
 * return the number 0 and 1, if can move stone return 1, else 0
 */
int stone(Board* board, int* row, int* column, char* currentPlayer) {
    while (*row == 0 || *column == 0 || *row == (board->row - 1) ||
            *column == (board->column - 1)) {
        if (*row == 0) {         //top
            return stone_row_0(board, row, column, currentPlayer);
        } else if (*column == 0) {    //left
            return stone_column_0(board, row, column, currentPlayer);
        } else if (*row == (board->row - 1)) {    //bottom
            return stone_last_row(board, column, currentPlayer);
        } else if (*column == (board->column - 1)) {      //right
            return stone_rightmost_column(board, row, currentPlayer);
        } else {
            return 0;
        }
    }
    return 0;
}

/*
 * human player put stone to row 0, do action
 * parameter: Board* board, int* row, int* column, char* currentPlayer
 * return 1 when action success, else 0;
 */
int human_stone_row_0(Board* board, int* row,
        int* column, char* currentPlayer) {
    if ((board->grid[board->row - 1][*column].playerPos != DOT)
            || (board->grid[*row + 1][*column].playerPos == DOT)
            || (board->grid[*row][*column].playerPos != DOT)) {
        return 0;
    } else {
        char* playerBuffer = malloc(sizeof(char) * (board->row + 1));
        for (int i = 0; i < board->row; ++i) {
            playerBuffer[i] = board->grid[i][*column].playerPos;
        }
        playerBuffer[*row] = *currentPlayer;
        for (int j = 0; j < (board->row - 1); ++j) {
            if (playerBuffer[j] != DOT) {
                board->grid[j + 1]
                        [*column].playerPos = playerBuffer[j];
            } else {
                break;
            }
        }
        free(playerBuffer);
        return 1;
    }
}

/*
 * human player put stone to column 0, do action
 * parameter: Board* board, int* row, int* column, char* currentPlayer
 * return 1 when action success, else 0;
 */
int human_stone_column_0(Board* board, int* row,
        int* column, char* currentPlayer) {
    if ((board->grid[*row][board->column - 1].playerPos != DOT)
            || (board->grid[*row][*column + 1].playerPos == DOT)
            || (board->grid[*row][*column].playerPos != DOT)) {
        return 0;
    } else {
        char* playerBuffer = malloc(sizeof(char)
                * (board->column + 1));
        for (int i = 0; i < board->column; ++i) {
            playerBuffer[i] = board->grid[*row][i].playerPos;
        }
        playerBuffer[*column] = *currentPlayer;
        for (int j = 0; j < (board->column - 1); ++j) {
            if (playerBuffer[j] != DOT) {
                board->grid[*row][j + 1].playerPos = playerBuffer[j];
            } else {
                break;
            }
        }
        free(playerBuffer);
        return 1;
    }
}

/*
 * human player put stone to last row , do action
 * parameter: Board* board, int* row, int* column, char* currentPlayer
 * return 1 when action success, else 0;
 */
int human_stone_last_row(Board* board, int* row,
        int* column, char* currentPlayer) {
    if ((board->grid[0][*column].playerPos != DOT)
            || (board->grid[*row - 1][*column].playerPos == DOT)
            || (board->grid[*row][*column].playerPos != DOT)) {
        return 0;
    } else {
        char* playerBuffer = malloc(sizeof(char) * (board->row + 1));
        int index = 0;
        for (int i = board->row - 1; i >= 0; --i) {
            playerBuffer[index] = board->grid[i][*column].playerPos;
            index++;
        }
        playerBuffer[0] = *currentPlayer;
        index = board->row - 2;
        for (int j = 0; j < board->row - 1; ++j) {
            if (playerBuffer[j] != DOT) {
                board->grid[index]
                        [*column].playerPos = playerBuffer[j];
                index--;
            } else {
                break;
            }
        }
        free(playerBuffer);
        return 1;
    }
}

/*
 * human player put stone to rightmost column, do action
 * parameter: Board* board, int* row, int* column, char* currentPlayer
 * return 1 when action success, else 0;
 */
int human_stone_rightmost_column(Board* board, int* row,
        int* column, char* currentPlayer) {
    if ((board->grid[*row][board->column - 1].playerPos != DOT)
            || (board->grid[*row][*column - 1].playerPos == DOT)
            || (board->grid[*row][*column].playerPos != DOT)) {
        return 0;
    } else {
        char* playerBuffer = malloc(sizeof(char) *
                (board->column - 1));
        int index = 0;
        for (int i = board->column - 1; i >= 0; --i) {
            playerBuffer[index] = board->grid[*row][i].playerPos;
            index++;
        }
        playerBuffer[0] = *currentPlayer;
        index = board->column - 2;
        for (int j = 0; j < (board->column - 1); ++j) {
            if (playerBuffer[j] != DOT) {
                board->grid[*row][index].playerPos = playerBuffer[j];
                index--;
            } else {
                break;
            }
        }
        free(playerBuffer);
        return 1;
    }
}

/*
 * parameter: Board * board, int row, int column, char player
 * when the human player put the stone,
 * the board need to check and do the action
 * There are four situations of stone(top, left, right, bottom)
 * return number 0 and 1, if can move stone return 1, else 0
 */
int human_stone(Board* board, int* row, int* column, char* currentPlayer) {
    while (*row == 0 || *column == 0 ||
            *row == (board->row - 1) || *column == (board->column - 1)) {
        if (*row == 0) {         //top
            return human_stone_row_0(board, row, column, currentPlayer);
        } else if (*column == 0) {    //left
            return human_stone_column_0(board, row, column, currentPlayer);
        } else if (*row == (board->row - 1)) {    //bottom
            return human_stone_last_row(board, row, column, currentPlayer);
        } else if (*column == (board->column - 1)) {      //right
            return human_stone_rightmost_column(board, row,
                    column, currentPlayer);
        } else {
            return 0;
        }
    }
    return 0;
}

/*
 * computer player move the stone at row 0
 * parameter: Board* board, char* currentPlayer, char* oppoPlayer
 * move the stone, return 1
 */
int type_one_row_0(Board* board, char* currentPlayer, char* oppoPlayer) {
    for (int i = 1; i < (board->column - 1); ++i) {
        for (int j = 0; j < (board->row - 1); ++j) {
            if ((board->grid[j][i].number > board->grid[j + 1][i].number)
                    && (board->grid[j][i].playerPos == *oppoPlayer)
                    && (board->grid[1][i].playerPos != DOT)
                    && (board->grid[board->row - 1][i].playerPos == DOT)
                    && (board->grid[0][i].playerPos == DOT)) {
                board->computerRow = 0;
                board->computerColumn = i;
                if (stone(board, &board->computerRow,
                        &board->computerColumn, currentPlayer) == 1) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*
 * computer player move the stone at rightmost column
 * parameter: Board* board, char* currentPlayer, char* oppoPlayer
 * move the stone, return 1
 */
int type_one_rightmost_column(Board* board, char* currentPlayer,
        char* oppoPlayer) {
    for (int k = 1; k < (board->row - 1); ++k) {
        for (int i = (board->column - 1); i > 1; --i) {
            if ((board->grid[k][i - 1].number > board->grid[k][i - 2].number)
                    && (board->grid[k][i - 1].playerPos == *oppoPlayer)
                    && (board->grid[k][board->column - 2].playerPos != DOT)
                    && (board->grid[k][0].playerPos == DOT)
                    && (board->grid[k]
                    [board->column - 1].playerPos == DOT)) {
                board->computerRow = k;
                board->computerColumn = board->column - 1;
                if (stone(board, &board->computerRow,
                        &board->computerColumn, currentPlayer) == 1) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*
 * computer player move the stone at bottom row
 * parameter: Board* board, char* currentPlayer, char* oppoPlayer
 * move the stone, return 1
 */
int type_one_bottom_row(Board* board, char* currentPlayer, char* oppoPlayer) {
    for (int l = (board->column - 2); l > 0; --l) {
        for (int i = board->row - 1; i > 0; --i) {
            if ((board->grid[i][l].number > board->grid[i - 1][l].number)
                    && (board->grid[i][l].playerPos == *oppoPlayer)
                    && (board->grid[board->row - 2][l].playerPos != DOT)
                    && (board->grid[0][l].playerPos == DOT)
                    && (board->grid[board->row - 1][l].playerPos == DOT)) {
                board->computerRow = board->row - 1;
                board->computerColumn = l;
                if (stone(board, &board->computerRow,
                        &board->computerColumn, currentPlayer) == 1) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*
 * computer player move the stone at leftmost column
 * parameter: Board* board, char* currentPlayer, char* oppoPlayer
 * move the stone, return 1
 */
int type_one_leftmost_column(Board* board, char* currentPlayer,
        char* oppoPlayer) {
    for (int m = board->row - 2; m > 0; --m) {
        for (int i = 1; i < board->column - 1; ++i) {
            if ((board->grid[m][i].number > board->grid[m][i + 1].number)
                    && (board->grid[m][i].playerPos == *oppoPlayer)
                    && (board->grid[m][1].playerPos != DOT)
                    && (board->grid[m][board->column - 1].playerPos == DOT)
                    && (board->grid[m][0].playerPos == DOT)) {
                board->computerRow = m;
                board->computerColumn = 0;
                if (stone(board, &board->computerRow,
                        &board->computerColumn, currentPlayer) == 1) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*
 * parameters: Board * board, char player, char * input
 * the algorithm of the type one case
 * return the number 0 and 1
 */
int type_one_alg(Board* board, char* currentPlayer) {
    char oppoPlayer;
    if (*currentPlayer != PLAYERO) {
        oppoPlayer = PLAYERO;
    } else {
        oppoPlayer = PLAYERX;
    }
    if(type_one_row_0(board, currentPlayer, &oppoPlayer) != 1) {
        if (type_one_rightmost_column(board, currentPlayer,
                &oppoPlayer) != 1) {
            if (type_one_bottom_row(board, currentPlayer, &oppoPlayer) != 1) {
                if (type_one_leftmost_column(board, currentPlayer,
                        &oppoPlayer) != 1) {
                    //find the max number
                    int temp = 0;
                    for (int r = 1; r <= (board->row - 2); ++r) {
                        for (int c = 1; c <= (board->column - 2); ++c) {
                            if ((board->grid[r][c].playerPos == DOT) &&
                                    (board->grid[r][c].number > temp)) {
                                temp = board->grid[r][c].number;
                                board->computerRow = r;
                                board->computerColumn = c;
                            }
                        }
                    }
                    board->grid[board->computerRow][board->computerColumn].
                            playerPos = *currentPlayer;
                    return 1;
                }
            }
        }
    }
    return 1;
}

/*
 * function for auto player, player O start form start to end
 *                          player X start from end to start
 * parameter: Board* board, char player
 * return void
 */
void auto_player(Board* board, char player) {
    int rowO = 1, columnO = 1;
    int rowX = board->row - 2, columnX = board->column - 2;
    if (player == PLAYERO) {
        if (board->grid[rowO][columnO].playerPos == DOT) {
            printf("Player %c placed at %d %d\n", PLAYERO, rowO, columnO);
            board->grid[rowO][columnO].playerPos = PLAYERO;
        } else {
            while(board->grid[rowO][columnO].playerPos != DOT) {
                if (columnO == board->column - 2 && rowO < board->row - 2) {
                    rowO++;
                    columnO = 1;
                } else {
                    columnO++;
                }
            }
            printf("Player %c placed at %d %d\n", PLAYERO, rowO, columnO);
            board->grid[rowO][columnO].playerPos = PLAYERO;
        }

    } else if (player == PLAYERX) {
        if (board->grid[rowX][columnX].playerPos == DOT) {
            printf("Player %c placed at %d %d\n", PLAYERX, rowX, columnX);
            board->grid[rowX][columnX].playerPos = PLAYERX;
        } else {
            while(board->grid[rowX][columnX].playerPos != DOT) {
                if (columnX == 1 && rowX > 1) {
                    rowX--;
                    columnX = board->column - 2;
                } else {
                    columnX--;
                }
            }
            printf("Player %c placed at %d %d\n", PLAYERX, rowX, columnX);
            board->grid[rowX][columnX].playerPos = PLAYERX;
        }
    }
}

/*
 * both player type is 0
 * parameter: the pointer of board
 * automatic play game
 * return void
 */
void auto_game(Board* board, char player) {
    int rowO = 1, columnO = 1;
    int rowX = board->row - 2, columnX = board->column - 2;
    while(check_full_board(board)) {
        if (player == PLAYERO &&
                board->grid[rowO][columnO].playerPos == DOT) {
            printf("Player %c placed at %d %d\n", PLAYERO, rowO, columnO);
            board->grid[rowO][columnO].playerPos = PLAYERO;
            if (columnO == board->column - 2 && rowO < board->row - 2) {
                rowO++;
                columnO = 1;
            } else {
                columnO++;
            }
            print_board(board);
        } else if (player == PLAYERX &&
                board->grid[rowX][columnX].playerPos == DOT) {
            printf("Player %c placed at %d %d\n", PLAYERX, rowX, columnX);
            board->grid[rowX][columnX].playerPos = PLAYERX;
            if (columnX == 1 && rowX > 1) {
                rowX--;
                columnX = board->column - 2;
            } else {
                columnX--;
            }
            print_board(board);
        }
        if (player == PLAYERO) {
            player = PLAYERX;
        } else {
            player = PLAYERO;
        }
    }
}

/*
 * the action for the human player in the type 1 game
 * parameter: Board* board, char currentPlayer, char* input
 * return 0 or 1, if the action success, return 1, else 0
 */
int human_player(Board* board, char currentPlayer, char* input) {
    //the maximum size of file name
    char* saveFileName = malloc(sizeof(char) * 255);
    ask_input(board, input, currentPlayer);
    //stone
    while(check_full_board(board)) {
        //check row & column, stone???
        if (board->humanRow == 0 || board->humanColumn == 0 ||
                board->humanRow == (board->row - 1) ||
                board->humanColumn == (board->column - 1)) {
            if (!human_stone(board, &board->humanRow,
                    &board->humanColumn, &currentPlayer)) {
                ask_input(board, input, currentPlayer);
            } else {
                return 1;
            }
        } else {
            if (board->humanRow >= 0 && board->humanColumn >= 0
                    && board->grid[board->humanRow]
                    [board->humanColumn].playerPos == DOT) {
                board->grid[board->humanRow]
                        [board->humanColumn].playerPos = currentPlayer;
                break;
            } else {
                ask_input(board, input, currentPlayer);
            }
        }
    }
    free(saveFileName);
    return 0;
}

/*
 * when one player is one and the other one is 0 or human
 * parameters: Board * board, char** argv
 * return void
 */
void type_one_game(Board* board, char** argv) {
    //space for hold human input information
    char* input = malloc(sizeof(char) * 255);
    char currentPlayer = board->player;
    while (check_full_board(board)) {
        if (currentPlayer == PLAYERO) {
            if (!strcmp(argv[1], "0")) {
                auto_player(board, currentPlayer);
            } else if (!strcmp(argv[1], "1")) {
                type_one_alg(board, &currentPlayer);
                printf("Player %c placed at %d %d\n", currentPlayer,
                        board->computerRow, board->computerColumn);
            } else {
                human_player(board, currentPlayer, input);
            }
        } else {
            if (!strcmp(argv[2], "0")) {
                auto_player(board, currentPlayer);
            } else if (!strcmp(argv[2], "1")) {
                type_one_alg(board, &currentPlayer);
                printf("Player %c placed at %d %d\n", currentPlayer,
                        board->computerRow, board->computerColumn);
            } else {
                human_player(board, currentPlayer, input);
            }
        }
        print_board(board);
        if (currentPlayer == PLAYERO) {
            currentPlayer = PLAYERX;
        } else {
            currentPlayer = PLAYERO;
        }
    }
    free(input);
}

/*
 * when both of the player are human
 * parameters: Board * board, char** argv
 * return void
 */
void human_game(Board* board) {
    char currentPlayer = board->player;
    char* input = malloc(sizeof(char) * 50);
    while(check_full_board(board)) {
        ask_input(board, input, currentPlayer);
        //check row & column, stone???
        while (1) {
            if (board->humanRow == 0 || board->humanColumn == 0 ||
                    board->humanRow == (board->row - 1) ||
                    board->humanColumn == (board->column - 1)) {
                if (!human_stone(board, &board->humanRow,
                        &board->humanColumn, &currentPlayer)) {
                    ask_input(board, input, currentPlayer);
                }
            } else {
                if (board->humanRow >= 0 && board->humanColumn >= 0
                        && board->grid[board->humanRow]
                        [board->humanColumn].playerPos == DOT) {
                    board->grid[board->humanRow][board->humanColumn].
                            playerPos = currentPlayer;
                    break;
                } else {
                    ask_input(board, input, currentPlayer);
                }
            }
        }
        print_board(board);
        if (currentPlayer == PLAYERO) {
            currentPlayer = PLAYERX;
        } else {
            currentPlayer = PLAYERO;
        }
    }
    free(input);
}

/*
 * check the load file is valid or not
 * parameter: char* file, int numberOfCharacter, Board* board
 * return number corresponds to different stderr message
 */
int check_file(char* file, int numberOfCharacter, Board* board) {
    int row = 0;
    int column = 0;
    FILE* loadfile = fopen(file, "r");
    if (loadfile == NULL) {
        return 3;
    }
    char* line = malloc(sizeof(char) * numberOfCharacter);
    fgets(line, numberOfCharacter - 1, loadfile);
    if (sscanf(line, "%d %d", &board->row, &board->column) != 2) {
        return 4;
    }
    if (board->row < 3 || board->column < 3) {
        return 4;
    }
    fgets(line, numberOfCharacter - 1, loadfile);
    if ((sscanf(line, "%c", &board->player) != 1) || strlen(line) != 2
            || !(board->player == PLAYERO || board->player == PLAYERX)) {
        return 4;
    }
    while (!feof(loadfile)) {
        fgets(line, numberOfCharacter - 1, loadfile);
        row++;
        for (unsigned int i = 0; i < strlen(line); ++i) {
            if (!(line[i] == '\r' || line[i] == '\n')) {
                column++;
            }
        }
        if (column != board->column * 2) {
            return 4;
        } else {
            column = 0;
        }
    }
    if ((row - 1) != board->row) {
        return 4;
    }
    free(line);
    return -1;
}

/*
 * check the corners of board
 * parameter: Boad* board
 * if vaild return 4
 */
int check_corners(Board* board) {
    //check four corners
    if ((board->grid[0][0].number != BLANK &&
            board->grid[0][0].playerPos != BLANK) &&
            (board->grid[0][board->column - 1].number != BLANK &&
            board->grid[0][board->column - 1].playerPos != BLANK) &&
            (board->grid[board->row - 1][0].number != BLANK &&
            board->grid[board->row - 1][0].playerPos != BLANK) &&
            (board->grid[board->row - 1][board->column - 1].number != BLANK &&
            board->grid[board->row - 1]
            [board->column - 1].playerPos != BLANK)) {
        return 4;
    }
    return -1;
}

/*
 * check the interior and cells
 * parameter: Boad* board
 * if vaild return 4
 */
int check_interior_cells(Board* board) {
    for (int c = 0; c < board->column; ++c) {
        if (!(board->grid[0][c].number == BLANK ||
                board->grid[0][c].number == '0' ||
                board->grid[0][c].playerPos == BLANK ||
                board->grid[0][c].playerPos == DOT ||
                board->grid[board->row - 1][c].number == BLANK ||
                board->grid[board->row - 1][c].number == '0' ||
                board->grid[board->row - 1][c].playerPos == BLANK ||
                board->grid[board->row - 1][c].playerPos == DOT ||
                board->grid[board->row - 1][c].playerPos == PLAYERX ||
                board->grid[board->row - 1][c].playerPos == PLAYERO)) {
            return 4;
        }
    }
    for (int r = 0; r < board->row; ++r) {
        if (!(board->grid[r][0].number == BLANK ||
                board->grid[r][0].number == '0' ||
                board->grid[r][0].playerPos == BLANK ||
                board->grid[r][0].playerPos == DOT ||
                board->grid[r][board->column - 1].number == BLANK ||
                board->grid[r][board->column - 1].number == '0' ||
                board->grid[r][board->column - 1].playerPos == BLANK ||
                board->grid[r][board->column - 1].playerPos == DOT ||
                board->grid[r][board->column - 1].playerPos == PLAYERX ||
                board->grid[r][board->column - 1].playerPos == PLAYERO)) {
            return 4;
        }
    }
    for (int r = 1; r < board->row - 1; ++r) {
        for (int c = 0; c < board->column - 1; ++c) {
            if (!(!isdigit(board->grid[r][c].number) ||
                    board->grid[r][c].playerPos == DOT ||
                    board->grid[r][c].playerPos == PLAYERX ||
                    board->grid[r][c].playerPos == PLAYERO)) {
                return 4;
            }
        }
    }
    return -1;
}

/*
 * load the board from the file and check the board
 * parameter: Board* board, const char* spaceForHold
 * return number corresponds to different stderr message
 */
int load_check_board(Board* board, const char* spaceForHold) {
    //load the file contents to the board
    int checkCell = 0; // check whether any space for playing game
    int index = 0;
    board->humanColumn = 0;
    board->humanRow = 0;
    board->grid = malloc(sizeof(Cell*) * board->row);
    for (int i = 0; i < board->row; ++i) {
        board->grid[i] = malloc(sizeof(Cell) * board->column);
        for (int j = 0; j < board->column; ++j) {
            board->grid[i][j].number = spaceForHold[index];
            index++;
            board->grid[i][j].playerPos = spaceForHold[index];
            index++;
            if (!(board->grid[i][j].playerPos == DOT ||
                    board->grid[i][j].playerPos == BLANK)) {
                checkCell++;
            }
        }
    }
    if (check_corners(board) == 4) {
        return 4;
    }
    if (check_interior_cells(board) == 4) {
        return 4;
    }
    // check whether the board is full
    if (checkCell >= ((board->row - 2) * (board->column - 2))) {
        return 6;
    }
    return -1;
}

/*
 * count the character in the file
 * parameter char** argv, int* numberOfCharacter
 * return void
 */
int count_character(char** argv) {
    int numberOfCharacter = 0;
    unsigned int row = 0;
    unsigned int column = 0;
    FILE* file = fopen(argv[3], "r");
    char ch;
    if (file == NULL) {
        exit(message(CANNOTREADFILE));
    } else {
        if ((ch = fgetc(file)) == EOF) {     //file is empty
            exit(message(ENDOFFILE));
        }
        while((ch = fgetc(file)) != EOF) {
            if (!(ch == '\n' || ch == '\r')) {
                numberOfCharacter++;//count the number of char in the file
                column++;
            } else {
                row++;
            }
        }
    }
    fclose(file);
    return numberOfCharacter;
}

/*
 * the function for loading game,
 * initial the board,and check file and board
 * parameter: char** argv
 * return the status of the game
 */
Status load_game(char** argv) {
    int numberOfCharacter = 0;
    int temp = -2;
    char ch;
    Board board;
    board.computerColumn = 0;
    board.computerRow = 0;
    board.scoreX = 0;
    board.scoreO = 0;
    numberOfCharacter = count_character(argv);
    if ((temp = check_file(argv[3], numberOfCharacter, &board)) != -1) {
        exit(message(temp));
    }
    char* spaceForHold = malloc(sizeof(char) * (numberOfCharacter + 1));
    //use the numOfChar to creat the space to hold the file contents
    FILE* file = fopen(argv[3], "r");
    if (file == NULL) {
        exit(message(CANNOTREADFILE));
    }
    fgets(spaceForHold, numberOfCharacter - 1, file);
    fgets(spaceForHold, numberOfCharacter - 1, file);
    for (int j = 0; j < numberOfCharacter + 1; ) {
        ch = fgetc(file);
        if (!(ch == '\n' || ch == '\r')) {
            spaceForHold[j] = (char)ch;
            ++j;
        }
    }
    fclose(file);
    file = NULL;
    if ((temp = load_check_board(&board, spaceForHold)) != -1) {
        exit(message(temp));
    }
    print_board(&board);
    if (!(strcmp(argv[1], "0")) && !(strcmp(argv[2], "0"))) {
        auto_game(&board, board.player);
    } else if (!(strcmp(argv[1], "H")) && !(strcmp(argv[2], "H"))) {
        human_game(&board);
    } else {
        type_one_game(&board, argv);
    }
    sum_score(&board);
    return message(0);
}

/*
 * check the parameters
 * if the number of parameters less ot more than 4
 * return error, else, load the game
 */
int main(int argc, char** argv) {
    if (argc != 4) {
        return message(WRONGARGE);
    } else {
        if (!((strcmp(argv[1], "0") && strcmp(argv[1], "1") &&
                strcmp(argv[1], "H")) || (strcmp(argv[2], "0") &&
                strcmp(argv[2], "1") && strcmp(argv[2], "H")))) {
            load_game(argv);
        } else {
            return message(WRONGPLAYER);
        }
    }
    return 0;
}
