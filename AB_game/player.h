#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

typedef enum {
    PR_NORMAL_END = 0,
    PR_WRONG_NUM_ARGS = 1,
    INVALID_PLAYER_COUNT = 2,
    INVALID_PAYERID = 3,
    ERROR_PATH = 4,
    EARLY_GAMEOVER = 5,
    PRCOMMUNICATION_ERROR = 6,
} PlayerError;

typedef struct {
    int cardA;
    int cardB;
    int cardC;
    int cardD;
    int cardE;
    int numCard;
} HandCard;

typedef struct {
    int row;
    int column;
    int oldRow;
    int oldColumn;
    int tempPos;
} Position;

typedef struct {
    int playerID;
    int money;
    int changeMoney;
    int siteV1;
    int siteV2;
    int point;
    int additionalPoint;
    int card;
    Position position;
    HandCard handCard;
    FILE* read;
    FILE* write;
    pid_t pid;
} Player;

typedef struct {
    char site1;
    char site2;
    int capacity;
    int occupied;
} Site;

typedef struct {
    int playerID;
} PlayerSite;

typedef struct {
    int pathSize;
    Site* site;
    PlayerSite** playerBoard;
} Path;

typedef struct {
    bool initFlag;
    int numPlayer;
    char* deck;
    int deckSize;
    int deckIndex;
    Player* players;
    Player* currentPlayer;
    Player* labelPlayer;
    Path path;
} Board;

//global board for A and B
Board globalBoard;

PlayerError pr_message(PlayerError errorMessage);
void init_player(Board* board);
void print_player(Board* board, FILE* output);
void update_all(Board* board, int index);
void who_turn(Board* board);
void site_action(Board* board, bool flag, int index);
bool check_argv(int argc, char** argv);
bool is_game_over(Board* board);
int receive_dealer_message(Board* board, char** message);
void check_path(Board* board, char** path);
int hap_message(Board* board, int p, int n, int s, int m, int c);
void dealer_out(Board* board, FILE* output);
void print_path(Board* board, FILE* output);
void final_scores(Board* board, FILE* output);