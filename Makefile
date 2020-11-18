OPTS=-std=c99 -pedantic -Wall -Werror -Wextra -g
push2310: XO_game.c
	gcc $(OPTS) XO_game.c -o push2310
