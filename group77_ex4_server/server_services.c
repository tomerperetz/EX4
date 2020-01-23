#include "./server_services.h"
#include <time.h>

// Private Functions Declerations -------------------------------------------------->
void getPlayerMoveIdx(char *player_move, int *idx);
static char option_lst[5][20] = { "ROCK", "PAPER", "SCISSORS", "LIZARD", "SPOCK" };
int playGame(int player1Move, int player2Move);

// Constants ------------------------------------------------------------------------>
int player1_won = 1;
int player2_won = 2;
int draw = 0;
#define GAME_SESSION_PATH "GameSession.txt"
#define MAX_WAIT_TIME 30
#define MAX_MOVE_SIZE 10

void removeEnter(char *str)
{
	for (str; *str != '\0'; str++)
	{
		if (*str == '\n')
			*str = '\0';
	}
}

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
	return ret_val;
}

int searchPartner()
{
	extern User usr_arr[MAX_USERS];

	BOOL usr_1_status = FALSE;
	BOOL usr_2_status = FALSE;

	usr_1_status = (usr_arr[0].online && (usr_arr[0].status == STATUS_CLIENT_VS));
	usr_2_status = (usr_arr[1].online && (usr_arr[1].status == STATUS_CLIENT_VS));

	return (usr_1_status && usr_2_status);

}

int gameSessionFileExist()
{
	/*
	Description: check if given path to a file exists.
	parameters:
			 - char *filename - path to file
	Returns: TRUE if exists
	*/
	struct stat buffer;
	return (stat(GAME_SESSION_PATH, &buffer) == 0);
		
}

int deleteGameSessionFile()
{
	if (remove(GAME_SESSION_PATH) == 0)
		return TRUE;

	printf("Error deleting %s. Please Verify file is closed.\n", GAME_SESSION_PATH);
	raiseError(2, __FILE__, __func__, __LINE__, ERROR_ID_2_IO);
	return FALSE;
}

int playVsGame(char *player_move, int player_idx, int opponent_idx, SOCKET *socket)
{
	extern User usr_arr[MAX_USERS];
	int ret_val = TRUE;
	DWORD wait_code = 0;
	BOOL release_res = TRUE;
	BOOL writer_flag = FALSE;
	extern HANDLE file_mutex;
	extern HANDLE partner_played_semaphore;
	FILE *fp;
	char opponent_move[MAX_MOVE_SIZE];
	int player_move_idx = ERR;
	int opponent_move_idx = ERR;
	int game_results=ERR;
	char winner_name[NAME_MAX_LEN];

	// File access mutex
	wait_code = WaitForSingleObject(file_mutex, INFINITE);
	if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) {
		ret_val = ERR;
		goto Realese_And_Quit;
	}

	usr_arr[player_idx].status = STATUS_INIT;

	if (gameSessionFileExist())
	{
		// this is the seconed user
		// read first player move
		if (fopen_s(&fp, GAME_SESSION_PATH, "r") != FALSE || fp == NULL) {
			printf("Can't open file: %s\n", GAME_SESSION_PATH);

			raiseError(2, __FILE__, __func__, __LINE__, ERROR_ID_2_IO);
			return ERR;
		}
		
		fgets(opponent_move, MAX_MOVE_SIZE, fp);
		
		fclose(fp);
		// append 2nd player move
		if (fopen_s(&fp, GAME_SESSION_PATH, "a") != FALSE || fp == NULL) {
			printf("Can't open file: %s\n", GAME_SESSION_PATH);

			raiseError(2, __FILE__, __func__, __LINE__, ERROR_ID_2_IO);
			return ERR;
		}

		fprintf_s(fp, player_move);
		fprintf_s(fp, "\n");

		fclose(fp);

		// Release semaphore for 1st user to read
		release_res = ReleaseSemaphore(partner_played_semaphore, 1, NULL);
		if (release_res == FALSE)
		{
			printf("Realese semaphore error!\n");
			raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
			goto Realese_And_Quit;
		}
	}
	else
	{
		//this is for the 1st user, so use write
		writer_flag = TRUE;
		
		if (fopen_s(&fp, GAME_SESSION_PATH, "w") != FALSE || fp == NULL) {
			printf("Can't open file: %s\n", GAME_SESSION_PATH);

			raiseError(2, __FILE__, __func__, __LINE__, ERROR_ID_2_IO);
			return ERR;
		}
		
		fprintf_s(fp, player_move);
		fprintf_s(fp, "\n");

		fclose(fp);
	}	

	release_res = ReleaseMutex(file_mutex);

	if (writer_flag)
	{
		// 1st user waits until 2nd user wrote his move
		wait_code = WaitForSingleObject(partner_played_semaphore, INFINITE);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) {
			ret_val = ERR;
			goto Realese_And_Quit;
		}

		// 1st user read from file after 2nd user wrote his move
		if (fopen_s(&fp, GAME_SESSION_PATH, "r") != FALSE || fp == NULL) {
			printf("Can't open file: %s\n", GAME_SESSION_PATH);

			raiseError(2, __FILE__, __func__, __LINE__, ERROR_ID_2_IO);
			return ERR;
		}

		// trash the first line
		fgets(opponent_move, MAX_MOVE_SIZE, fp);
		
		// save the 2nd line - opponent move
		fgets(opponent_move, MAX_MOVE_SIZE, fp);

		fclose(fp);
		// 1st user deletes the file
		if (!deleteGameSessionFile()) return ERR;
	}

	// Game
	removeEnter(opponent_move);
	getPlayerMoveIdx(player_move, &player_move_idx);
	getPlayerMoveIdx(opponent_move, &opponent_move_idx);

	game_results = playGame(player_move_idx, opponent_move_idx);

	// Check Results
	if (game_results == draw)
	{
		printf(SERVER_GAME_RESULTS_DRAW_MSG, player_move, usr_arr[opponent_idx].player_data->name, opponent_move);
		ret_val = sendMessegeWrapper(*socket, SERVER_GAME_RESULTS, player_move, usr_arr[opponent_idx].player_data->name, opponent_move, "DRAW", NULL);
		return TRUE;
	}

	if (game_results == player1_won) 
	{
		strcpy_s(winner_name, USERNAME_MAX_LEN, usr_arr[player_idx].player_data->name);
		usr_arr[player_idx].player_data->win += 1;
	}
	else if (game_results == player2_won) {
		strcpy_s(winner_name, USERNAME_MAX_LEN, usr_arr[opponent_idx].player_data->name);
		usr_arr[opponent_idx].player_data->win -= 1;
	}
	else
		ret_val = ERR;

	// send results to user
	printf("winner: %s\n", winner_name);
	ret_val = sendMessegeWrapper(*socket, SERVER_GAME_RESULTS, player_move, usr_arr[opponent_idx].player_data->name, opponent_move, winner_name, NULL);



Realese_And_Quit:

	return TRUE;
}

int client_vs_client(SOCKET *socket, User *usr)
{
	extern User usr_arr[MAX_USERS];
	int ret_val = TRUE;
	int game_results = ERR;
	char *player_move_temp = "ROCK";
	int oponnent_idx = ERR;
	int done = FALSE;

	if (usr->idx == 0)
		oponnent_idx = 1;
	else
		oponnent_idx = 0;
	while (!done) {
		for (int i = 0; i < MAX_WAIT_TIME; i++)
		{
			ret_val = searchPartner();
			if (ret_val) break;
			Sleep(1000);
		}
		// if couldn't get an opponent
		if (!ret_val) return NO_PARTNER;

		// Send opponent name to user
		ret_val = sendMessegeWrapper(*socket, SERVER_INVITE, usr_arr[oponnent_idx].player_data->name, NULL, NULL, NULL, NULL);

		if (ret_val != TRUE) {
			goto MAIN_CLEANUP;
		}

		// ask user for his move
		Messege client_reply;
		Messege client_reply_2;
		initMessege(&client_reply, NULL, NULL, NULL, NULL, NULL, NULL);
		ret_val = sendMessegeWrapper(*socket, SERVER_PLAYER_MOVE_REQUEST, NULL, NULL, NULL, NULL, NULL);
		if (ret_val != TRUE) {
			goto MAIN_CLEANUP;
		}
		ret_val = decodeWrapper(&client_reply, socket);
		if (ret_val != TRUE || !STRINGS_ARE_EQUAL(client_reply.type, CLIENT_PLAYER_MOVE)) {
			if (ret_val == TRUE) freeMessege(&client_reply);
			ret_val = ERR;
			goto MAIN_CLEANUP;
		}
		// Game
		ret_val = playVsGame(client_reply.params[0], usr->idx, oponnent_idx, socket);
		if (ret_val != TRUE)
		{
			freeMessege(&client_reply);
			goto MAIN_CLEANUP;
		}

		ret_val = sendMessegeWrapper(*socket, SERVER_GAME_OVER_MENU, CLIENT_CPU, NULL, NULL, NULL, NULL);
		if (ret_val != TRUE) 
		{
			freeMessege(&client_reply);
			goto MAIN_CLEANUP;
		}
		ret_val = decodeWrapper(&client_reply_2, socket);
		if (STRINGS_ARE_EQUAL(client_reply_2.type, CLIENT_MAIN_MENU) || ret_val != TRUE) {
			if (ret_val == TRUE) freeMessege(&client_reply_2);
			done = TRUE;
			freeMessege(&client_reply);
			goto MAIN_CLEANUP;
		}
		else if (!(STRINGS_ARE_EQUAL(client_reply_2.type, CLIENT_REPLAY))) {
			printf("The messege type - %s - is invalid\n", client_reply_2.type);
			done = TRUE;
			freeMessege(&client_reply);
			freeMessege(&client_reply_2);
			goto MAIN_CLEANUP;
		}
		freeMessege(&client_reply);
		freeMessege(&client_reply_2);
		usr->status = STATUS_CLIENT_VS;
	}
MAIN_CLEANUP:
	return TRUE;
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

void initUser(User *new_user, Player *p_player_data, int status, int idx, BOOL online)
{
	// Initializtion
	new_user->player_data = p_player_data;
	new_user->status = status;
	new_user->online = online;
	new_user->idx = idx;
	return;
}
 
int seekAndDestroy()
{
	int ret = TRUE;
	if (gameSessionFileExist())
		ret = deleteGameSessionFile();
	return ret;
}