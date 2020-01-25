
/*
=================================================================================
							Description
=================================================================================
		Server services - different game functions
=================================================================================
*/

#include "server_services.h"
#include <time.h>

// Private Functions Declerations -------------------------------------------------->
void getPlayerMoveIdx(char *player_move, int *idx);
int playGame(int player1Move, int player2Move);

// Constants ------------------------------------------------------------------------>
int player1_won = 1;
int player2_won = 2;
int draw = 0;
static char option_lst[5][20] = { "ROCK", "PAPER", "SCISSORS", "LIZARD", "SPOCK" };
#define MAX_WAIT_TIME 15
#define MAX_MOVE_SIZE 10

// Functions ------------------------------------------------------------------------>

void removeEnter(char *str)
{
	/*
	Description: remove \n from a string
	parameters:
			 - char *str
	Returns: void
	*/

	for (str; *str != '\0'; str++)
	{
		if (*str == '\n')
			*str = '\0';
	}
}

int client_vs_cpu(SOCKET *socket, Player *player)
{
	/*
	Description: reads move from player, choose random move from cpu and calc winner
	parameters:
			 - Player *player
			 - SOCKET *socket
	Returns: TRUE if succeded, ERR o.w
	*/

	int cpu_move_idx = 0, player_move_idx = ERR;
	int ret_val = TRUE;
	int done = FALSE;
	int result = 0;
	char winner_name[USERNAME_MAX_LEN], server_name[10] = "Server";
	
	srand((unsigned int) time(NULL));   // Initialization, should only be called once.

	while (!done) 
	{
		Messege client_sec_msg;
		Messege client_first_msg;
		initMessege(&client_first_msg, NULL, NULL, NULL, NULL, NULL, NULL);
		initMessege(&client_sec_msg, NULL, NULL, NULL, NULL, NULL, NULL);

		cpu_move_idx = rand() % 5;      // Returns a pseudo-random integer between 0 and 5.
	
		// send move request to client
		ret_val = sendMessegeWrapper(*socket, SERVER_PLAYER_MOVE_REQUEST, NULL, NULL, NULL, NULL, NULL);
		if (ret_val != TRUE) {
			done = TRUE;
			goto MAIN_CLEANUP2;
		}

		// recv move from to client
		ret_val = decodeWrapper(&client_first_msg, socket);
		if (ret_val != TRUE || !STRINGS_ARE_EQUAL(client_first_msg.type, CLIENT_PLAYER_MOVE)) {
			ret_val = ERR;
			done = TRUE;
			goto MAIN_CLEANUP2;
		}

		// get game results
		getPlayerMoveIdx(client_first_msg.params[0], &player_move_idx);
		result = playGame(player_move_idx, cpu_move_idx);

		if (result == player1_won) 
		{
			strcpy_s(winner_name, USERNAME_MAX_LEN, player->name);
			player->win += 1;
		}
		else if (result == player2_won) 
		{
			strcpy_s(winner_name, USERNAME_MAX_LEN, server_name);
			player->loss += 1;
		}

		else if (result == draw) 
		{
			strcpy_s(winner_name, USERNAME_MAX_LEN, "DRAW");
		}

		else 
		{
			ret_val = ERR;
			done = TRUE;
			goto MAIN_CLEANUP2;
		}

		// send results to user
		ret_val = sendMessegeWrapper(*socket, SERVER_GAME_RESULTS, client_first_msg.params[0], server_name, option_lst[cpu_move_idx],
			winner_name, NULL);

		if (ret_val != TRUE) {
			done = TRUE;
			goto MAIN_CLEANUP2;
		}
		// send game over menu to user
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
		freeMessege(&client_first_msg);
	}

	return ret_val;
}

int searchPartner()
{
	/*
	Description: search 2 online players that wants to play vs other user.
	Returns: TRUE if 2 users were found, FALSE o.w
	*/

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
	/*
	Description: delete game session file
	parameters: 
	Returns: TRUE if succeded, FALSE o.w
	*/

	if (remove(GAME_SESSION_PATH) == 0)
		return TRUE;

	printf("Error deleting %s. Please Verify file is closed.\n", GAME_SESSION_PATH);
	raiseError(2, __FILE__, __func__, __LINE__, ERROR_ID_2_IO);
	return FALSE;
}

int playVsGame(char *player_move, int player_idx, int opponent_idx, SOCKET *socket)
{
	/*
	Description: writes current player move to txt file, reads opponents move, calc winner and send msg to users
	parameters:
			 - char *player_move
			 - int player_idx
			 - int opponent_idx
			 - SOCKET *socket
	Returns: TRUE if succeded, ERR o.w
	*/

	extern User usr_arr[MAX_USERS];
	extern HANDLE file_mutex;
	extern HANDLE partner_played_semaphore;
	int ret_val = TRUE;
	int player_move_idx = ERR;
	int opponent_move_idx = ERR;
	int game_results = ERR;
	int wait_time_for_opponent_move = 5 * 60 * 1000; // wait 5 minute to opponent, o.w assume he left the game
	char opponent_move[MAX_MOVE_SIZE];
	char winner_name[NAME_MAX_LEN];
	DWORD wait_code = 0;
	BOOL release_res = TRUE;
	BOOL writer_flag = FALSE;
	FILE *fp;

	// File access mutex
	wait_code = WaitForSingleObject(file_mutex, INFINITE);
	if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) 
	{
		ret_val = ERR;
		release_res = ReleaseMutex(file_mutex);
		if (release_res == FALSE)
		{
			printf("Realese semaphore error!\n");
			raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		}
		goto Realese_And_Quit;
	}

	// restart user status for next game iteration
	usr_arr[player_idx].status = STATUS_INIT;

	// communicate moves between the threads using GameSession.txt file
	if (gameSessionFileExist())
	{
		// this is the seconed user
		// read first player move
		if (fopen_s(&fp, GAME_SESSION_PATH, "r") != FALSE || fp == NULL) 
		{
			printf("Can't open file: %s\n", GAME_SESSION_PATH);

			raiseError(2, __FILE__, __func__, __LINE__, ERROR_ID_2_IO);
			return ERR;
		}
		fgets(opponent_move, MAX_MOVE_SIZE, fp);
		fclose(fp);

		// append 2nd player move
		if (fopen_s(&fp, GAME_SESSION_PATH, "a") != FALSE || fp == NULL) 
		{
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
		// 1st user, notigy with writer_flag
		writer_flag = TRUE;
		
		// use write
		if (fopen_s(&fp, GAME_SESSION_PATH, "w") != FALSE || fp == NULL) 
		{
			printf("Can't open file: %s\n", GAME_SESSION_PATH);

			raiseError(2, __FILE__, __func__, __LINE__, ERROR_ID_2_IO);
			return ERR;
		}
		fprintf_s(fp, player_move);
		fprintf_s(fp, "\n");
		fclose(fp);
	}	
	
	//Release file mutex
	release_res = ReleaseMutex(file_mutex);
	if (release_res == FALSE)
	{
		printf("Realese semaphore error!\n");
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
	}

	if (writer_flag)
	{
		// 1st user waits until 2nd user writes his move
		wait_code = WaitForSingleObject(partner_played_semaphore, wait_time_for_opponent_move);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) 
		{
			if (wait_code == WAIT_TIMEOUT) return WAIT_TIMEOUT;
			release_res = ReleaseSemaphore(partner_played_semaphore, 1, NULL);
			if (release_res == FALSE)
			{
				printf("Realese semaphore error!\n");
				raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
			}
			goto Realese_And_Quit;
		}


		// 1st user read from file after 2nd user wrote his move
		if (fopen_s(&fp, GAME_SESSION_PATH, "r") != FALSE || fp == NULL) 
		{
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
	removeEnter(opponent_move); //remove \\n from line read
	getPlayerMoveIdx(player_move, &player_move_idx);
	getPlayerMoveIdx(opponent_move, &opponent_move_idx);
	// calc game results
	game_results = playGame(player_move_idx, opponent_move_idx);

	// Check Results
	if (game_results == draw)
	{
		ret_val = sendMessegeWrapper(*socket, SERVER_GAME_RESULTS, player_move, usr_arr[opponent_idx].player_data->name, opponent_move, "DRAW", NULL);
		return ret_val;
	}

	if (game_results == player1_won) 
	{
		strcpy_s(winner_name, USERNAME_MAX_LEN, usr_arr[player_idx].player_data->name);
		usr_arr[player_idx].player_data->win += 1;
		usr_arr[opponent_idx].player_data->loss += 1;
	}
	else if (game_results == player2_won) {
		strcpy_s(winner_name, USERNAME_MAX_LEN, usr_arr[opponent_idx].player_data->name);
	}
	else ret_val = ERR;

	// send results to user
	ret_val = sendMessegeWrapper(*socket, SERVER_GAME_RESULTS, player_move, usr_arr[opponent_idx].player_data->name, opponent_move, winner_name, NULL);

Realese_And_Quit:
	return ret_val;
}

int client_vs_client(SOCKET *socket, User *usr)
{
	/*
	Description: Client vs Client game, recives move from user and play vs other thread
	parameters:
			 - SOCKET *socket - Current thread socket
			 - User *usr - User struct
	Returns: TRUE if suceeded, NO_PARTNER if couldn't get partner
	*/
	
	extern User usr_arr[MAX_USERS];
	char opponent_name[NAME_MAX_LEN];
	int ret_val = TRUE;
	int game_results = ERR;
	int oponnent_idx = ERR;
	int done = FALSE;
	int game_number = 0;

	// get opponent idx
	if (usr->idx == 0)
		oponnent_idx = 1;
	else
		oponnent_idx = 0;
	
	// Game loop
	while (!done) 
	{
		Messege client_reply_move;
		Messege client_reply_game_over_menu;
		game_number++;
		// delete old game session file if exists
		if (!seekAndDestroy())
		{
			printf("Please delete manually or remove read only from  gameSession.txt file and restart program.\n");
			return FALSE;

		}

		// search for partner for 15 secondes, else return
		for (int i = 0; i < MAX_WAIT_TIME; i++)
		{
			ret_val = searchPartner();
			
			// if oppenent chose to return to main menu
			if ((game_number > 1) && (usr_arr[oponnent_idx].play_vs_again == FALSE)&&(usr_arr[oponnent_idx].status == STATUS_LEFT_GAME))
			{
				ret_val = sendMessegeWrapper(*socket, SERVER_OPPONENT_QUIT, opponent_name, NULL, NULL, NULL, NULL);
				if (ret_val != TRUE) goto MAIN_CLEANUP;
				usr_arr[usr->idx].status = STATUS_INIT;
				usr_arr[usr->idx].play_vs_again = DONT_KNOW;
				usr_arr[oponnent_idx].play_vs_again = DONT_KNOW;
				return TRUE;
			}

			// if oppenent chose to return to main menu and replied before we answered -> he is offline
			if ((game_number > 1) && (!usr_arr[oponnent_idx].online))
			{
				OPPONENT_LEFT:
				ret_val = sendMessegeWrapper(*socket, SERVER_OPPONENT_QUIT, opponent_name, NULL, NULL, NULL, NULL);
				if (ret_val != TRUE) goto MAIN_CLEANUP;
				usr_arr[usr->idx].status = STATUS_INIT;
				return TRUE;
			}
			if (ret_val)
			{
				strcpy_s(opponent_name, NAME_MAX_LEN, usr_arr[oponnent_idx].player_data->name);
				break;
			}
			
			Sleep(1000);
		}

		// if couldn't get an opponent
		if (!ret_val) return NO_PARTNER;

		// Send opponent name to user
		ret_val = sendMessegeWrapper(*socket, SERVER_INVITE, usr_arr[oponnent_idx].player_data->name, NULL, NULL, NULL, NULL);
		if (ret_val != TRUE) goto MAIN_CLEANUP;

		// ask user for his move

		// send move request to player
		ret_val = sendMessegeWrapper(*socket, SERVER_PLAYER_MOVE_REQUEST, NULL, NULL, NULL, NULL, NULL);
		if (ret_val != TRUE) goto MAIN_CLEANUP;

		// recv move from player
		initMessege(&client_reply_move, NULL, NULL, NULL, NULL, NULL, NULL);
		ret_val = decodeWrapper(&client_reply_move, socket);
		if (ret_val != TRUE || !STRINGS_ARE_EQUAL(client_reply_move.type, CLIENT_PLAYER_MOVE)) 	
		{
			freeMessege(&client_reply_move);
			ret_val = ERR;
			goto MAIN_CLEANUP;
		}

		// play Game
		ret_val = playVsGame(client_reply_move.params[0], usr->idx, oponnent_idx, socket);
		if (ret_val != TRUE)
		{
			freeMessege(&client_reply_move);
			if (ret_val == WAIT_TIMEOUT) goto OPPONENT_LEFT;
			goto MAIN_CLEANUP;
		}
		freeMessege(&client_reply_move);

		// send game over menu
		ret_val = sendMessegeWrapper(*socket, SERVER_GAME_OVER_MENU, CLIENT_CPU, NULL, NULL, NULL, NULL);
		if (ret_val != TRUE) goto MAIN_CLEANUP;

		// recv game over menu answer from user
		initMessege(&client_reply_game_over_menu, NULL, NULL, NULL, NULL, NULL, NULL);
		ret_val = decodeWrapper(&client_reply_game_over_menu, socket);
		if (ret_val != TRUE) goto MAIN_CLEANUP;

		// if user wants to go back to main menu break loop
		if (STRINGS_ARE_EQUAL(client_reply_game_over_menu.type, CLIENT_MAIN_MENU) || ret_val != TRUE) {
			if (ret_val == TRUE) freeMessege(&client_reply_game_over_menu);
			usr->play_vs_again = FALSE;
			usr_arr[usr->idx].status = STATUS_LEFT_GAME;
			done = TRUE;
			goto MAIN_CLEANUP;
		}

		// else if we got wrong reply
		else if (!(STRINGS_ARE_EQUAL(client_reply_game_over_menu.type, CLIENT_REPLAY))) {
			printf("The messege type - %s - is invalid\n", client_reply_game_over_menu.type);
			done = TRUE;
			freeMessege(&client_reply_game_over_menu);
			goto MAIN_CLEANUP;
		}
		
		// else we start loop again
		freeMessege(&client_reply_game_over_menu);
		usr->status = STATUS_CLIENT_VS;
	}

MAIN_CLEANUP:
	return ret_val;
}

void getPlayerMoveIdx(char *player_move, int *idx)
{
	/*
	Description: convert player move from char to equivelent int
	parameters:
			- char *player_move
			- int idx
	Returns: void
	*/

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

int playGame(int player1Move, int player2Move) 
{

	/*
	Description: get winner from 2 players according to game rules.
	parameters:
			 - int player1Move
			 - int player2Move
	Returns: int winner - player 1, player 2 or draw.
	*/

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

void initUser(User *new_user, Player *p_player_data, int status, int idx, BOOL online, int play_vs_again)
{
	/*
	Description: initialize user struct
	parameters:
			 - User *new_user - user struct dst variable
			 - Player *p_player_data - player data (name, win, loss)
			 - int status - is he intersted in playing vs other client
			 - int idx - thread idx, same as usr_arr idx
			 - BOOL online - TRUE if user is connected
	Returns: void
	*/

	new_user->player_data = p_player_data;
	new_user->status = status;
	new_user->online = online;
	new_user->idx = idx;
	new_user->play_vs_again = play_vs_again;
	return;
}
 
int seekAndDestroy()
{
	/*
	Description: if GameSession.txt file exists - delete it
	Returns: TRUE if succeded, FALSE o.w
	*/

	int ret = TRUE;
	if (gameSessionFileExist())
		ret = deleteGameSessionFile();
	return ret;
}