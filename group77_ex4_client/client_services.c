#include "client_services.h"
#include "./SocketExampleClient.h"

Messege g_msg_in;

void printMsg(Messege *msg)
{

	if (strcmp(msg->type, SERVER_MAIN_MANU) == 0)
	{
		//printf(SERVER_MAIN_MANU_MSG);
	}
	else if (strcmp(msg->type, SERVER_APPROVED) == 0)
	{
		//printf(SERVER_APPROVED_MSG);
	}
	else if (strcmp(msg->type, SERVER_DENIED) == 0)
	{
		//printf(SERVER_DENIED_MSG);
	}
	else if (strcmp(msg->type, SERVER_INVITE) == 0)
	{
		// DO SOMETHING
	}
	else if (strcmp(msg->type, SERVER_PLAYER_MOVE_REQUEST) == 0)
	{
		//printf(SERVER_PLAYER_MOVE_REQUEST_MSG);
	}
	else if (strcmp(msg->type, SERVER_GAME_RESULTS) == 0)
	{
		//printf(SERVER_GAME_RESULTS_MSG);
	}
	else if (strcmp(msg->type, SERVER_GAME_OVER_MANU) == 0)
	{
		//printf(SERVER_GAME_OVER_MANU_MSG);
	}
	else if (strcmp(msg->type, SERVER_OPPONENT_QUIT) == 0)
	{
		//printf(SERVER_OPPONENT_QUIT_MSG);
	}
	else if (strcmp(msg->type, SERVER_NO_OPPONENTS) == 0)
	{
		//printf("SERVER_NO_OPPONENTS_MSG");
	}
	else if (strcmp(msg->type, SERVER_LEADERBOARD) == 0)
	{
		//printf("Print leader board\n");
	}
	else if (strcmp(msg->type, SERVER_LEADERBOARD_MANU) == 0)
	{
		//printf(SERVER_LEADERBOARD_MANU_MSG);
	}
	else {
		//printf("seems we don't handle this message type: %s", msg->type);
	}
		

}


int clientStateMachine(Messege *msg_out, Messege *g_msg_in)
{
	char *SendStr;
	int ret_val = TRUE;
	printf("++++++++++++++++++++\n");
	printMessege(g_msg_in);
	printf("%s   %s\n", g_msg_in->type, SERVER_MAIN_MANU);
	if (strcmp(g_msg_in->type, SERVER_MAIN_MANU) == 0)
	{
		printf(SERVER_MAIN_MANU_MSG);
		SendStr = getString(stdin);
		if (SendStr == NULL) {
			return ERR;
		}
		initMessege(msg_out, "CLIENT_MAIN_MANU", SendStr, NULL, NULL, NULL, NULL);
		return TRUE;
	}
	else if (strcmp(g_msg_in->type, SERVER_APPROVED) == 0)
	{
		//printf(SERVER_APPROVED_MSG);
	}
	else if (strcmp(g_msg_in->type, SERVER_DENIED) == 0)
	{
		//printf(SERVER_DENIED_MSG);
	}
	else if (strcmp(g_msg_in->type, SERVER_INVITE) == 0)
	{
		// DO SOMETHING
	}
	else if (strcmp(g_msg_in->type, SERVER_PLAYER_MOVE_REQUEST) == 0)
	{
		printf(SERVER_PLAYER_MOVE_REQUEST_MSG);
	}
	else if (strcmp(g_msg_in->type, SERVER_GAME_RESULTS) == 0)
	{
		//printf(SERVER_GAME_RESULTS_MSG);
	}
	else if (strcmp(g_msg_in->type, SERVER_GAME_OVER_MANU) == 0)
	{
		printf(SERVER_GAME_OVER_MANU_MSG);
	}
	else if (strcmp(g_msg_in->type, SERVER_OPPONENT_QUIT) == 0)
	{
		//printf(SERVER_OPPONENT_QUIT_MSG);
	}
	else if (strcmp(g_msg_in->type, SERVER_NO_OPPONENTS) == 0)
	{
		printf("SERVER_NO_OPPONENTS_MSG");
	}
	else if (strcmp(g_msg_in->type, SERVER_LEADERBOARD) == 0)
	{
		printf("Print leader board\n");
	}
	else if (strcmp(g_msg_in->type, SERVER_LEADERBOARD_MANU) == 0)
	{
		printf(SERVER_LEADERBOARD_MANU_MSG);
	}
	else
		printf("seems we don't handle this message type: %s", g_msg_in->type);

	return TRUE;
}
