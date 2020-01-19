#include "./server_services.h"
#include <time.h>

/*oOoOoOoOoOoOoOoOoOoOoOoOoO Private Functions Declerations OoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
void getPlayerMoveIdx(char *player_move, int *idx);
static char option_lst[5][20] = { "ROCK", "PAPER", "SCISSORS", "LIZARD", "SPOCK" };
int playGame(int player1Move, int player2Move);

/*oOoOoOoOoOoOoOoOoOoOoOoOoO Constants OoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
int player1_won = 1;
int player2_won = 2;
int draw = 0;


int client_vs_cpu(SOCKET *socket, Player *player)
{
	int cpu_move_idx = 0, player_move_idx = ERR;
	int ret_val = TRUE;
	int done = FALSE;
	int result = 0;
	char winner_name[USERNAME_MAX_LEN], server_name[10] = "Server";
	Messege client_first_msg; 

	initMessege(&client_first_msg, CLIENT_PLAYER_MOVE, "ROCK", NULL, NULL, NULL, NULL);
	
	srand((unsigned int) time(NULL));   // Initialization, should only be called once.

	while (!done) {

		Messege client_sec_msg;
		initMessege(&client_sec_msg, NULL, NULL, NULL, NULL, NULL, NULL);

		cpu_move_idx = rand() % 5;      // Returns a pseudo-random integer between 0 and 5.
	
		ret_val = sendMessegeWrapper(*socket, SERVER_PLAYER_MOVE_REQUEST, NULL, NULL, NULL, NULL, NULL);
		if (ret_val != TRUE) {
			done = TRUE;
			goto MAIN_CLEANUP2;
		}
		ret_val = decodeWrapper(&client_first_msg, socket);
		if (ret_val != TRUE || !STRINGS_ARE_EQUAL(client_first_msg.type, CLIENT_PLAYER_MOVE)) {
			ret_val = ERR;
			done = TRUE;
			goto MAIN_CLEANUP2;
		}
		getPlayerMoveIdx(client_first_msg.params[0], &player_move_idx);
		result = playGame(player_move_idx, cpu_move_idx);

		if (result == player1_won) {
			strcpy_s(winner_name, USERNAME_MAX_LEN, player->name);
			player->win += 1;

		}
		else if (result == player2_won) {
			strcpy_s(winner_name, USERNAME_MAX_LEN, server_name);
			player->win -= 1;
		}
		else if (result == draw) {
			strcpy_s(winner_name, USERNAME_MAX_LEN, "DRAW");
		}
		else {
			ret_val = ERR;
			done = TRUE;
			goto MAIN_CLEANUP2;
		}

		ret_val = sendMessegeWrapper(*socket, SERVER_GAME_RESULTS, client_first_msg.params[0], server_name, option_lst[cpu_move_idx],
			winner_name, NULL);

		if (ret_val != TRUE) {
			done = TRUE;
			goto MAIN_CLEANUP2;
		}
		ret_val = sendMessegeWrapper(*socket, SERVER_GAME_OVER_MENU, CLIENT_CPU, NULL, NULL, NULL, NULL);
		if (ret_val != TRUE) {
			done = TRUE;
			goto MAIN_CLEANUP2;
		}
		ret_val = decodeWrapper(&client_sec_msg, socket);
		if (STRINGS_ARE_EQUAL(client_sec_msg.type, CLIENT_MAIN_MENU) || ret_val != TRUE) {
			done = TRUE;
			goto MAIN_CLEANUP2;
		}
		else if (!(STRINGS_ARE_EQUAL(client_sec_msg.type, CLIENT_REPLAY))) {
			printf("The messege type - %s - is invalid\n", client_sec_msg.type);
			done = TRUE;
			goto MAIN_CLEANUP2;
		}
MAIN_CLEANUP2:
		freeMessege(&client_sec_msg);
	}

	freeMessege(&client_first_msg);
	printf("segev has left the building\n");
	return ret_val;
}

void getPlayerMoveIdx(char *player_move, int *idx)
{
	if (STRINGS_ARE_EQUAL(player_move, option_lst[0]))
		*idx = 0;
	else if (STRINGS_ARE_EQUAL(player_move, option_lst[1]))
		*idx = 1;
	else if (STRINGS_ARE_EQUAL(player_move, option_lst[2]))
		*idx = 2;
	else if (STRINGS_ARE_EQUAL(player_move, option_lst[3]))
		*idx = 3;
	else if (STRINGS_ARE_EQUAL(player_move, option_lst[4]))
		*idx = 4;
	else 
		*idx = ERR;
}

int playGame(int player1Move, int player2Move) {
	printf("%d %d\n", player1Move, player2Move);
	switch (player1Move)
	{
	case (ROCK):
		if (player2Move == PAPER || player2Move == SPOCK)
			return player2_won;
		else if (player2Move == ROCK)
			return draw;
		else return player1_won;
	case (PAPER):
		if (player2Move == SCISSORS || player2Move == LIZARD)
			return player2_won;
		else if (player2Move == PAPER)
			return draw;
		else return player1_won;
	case (SCISSORS):
		if (player2Move == ROCK || player2Move == SPOCK)
			return player2_won;
		else if (player2Move == SCISSORS)
			return draw;
		else return player1_won;
	case (LIZARD):
		if (player2Move == ROCK || player2Move == SCISSORS)
			return player2_won;
		else if (player2Move == LIZARD)
			return draw;
		else return player1_won;
	case (SPOCK):
		if (player2Move == PAPER || player2Move == LIZARD)
			return player2_won;
		else if (player2Move == SPOCK)
			return draw;
		else return player1_won;
	}
	return ERR;
}