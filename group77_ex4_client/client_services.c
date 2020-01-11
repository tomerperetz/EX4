#include "client_services.h"

void printMsg(Messege *msg)
{
	if (strcmp(msg->type, "SERVER_MAIN_MANU") == 0)
	{
		// PRINT MAIN MANU
	}
	else if (strcmp(msg->type, "SERVER_APPROVED") == 0)
	{

	}
	else if (strcmp(msg->type, "SERVER_DENIED") == 0)
	{

	}
	else if (strcmp(msg->type, "SERVER_INVITE") == 0)
	{

	}
	else if (strcmp(msg->type, "SERVER_PLAYER_MOVE_REQUEST") == 0)
	{

	}
	else if (strcmp(msg->type, "SERVER_GAME_RESULTS") == 0)
	{

	}
	else if (strcmp(msg->type, "SERVER_GAME_OVER_MANU") == 0)
	{

	}
	else if (strcmp(msg->type, "SERVER_OPPONENT_QUIT") == 0)
	{

	}
	else if (strcmp(msg->type, "SERVER_NO_OPPONENTS") == 0)
	{

	}
	else if (strcmp(msg->type, "SERVER_LEADERBOARD") == 0)
	{

	}
	else if (strcmp(msg->type, "SERVER_LEADERBOARD_MANU") == 0)
	{

	}
	else
		printf("seems we don't handle this message type: %s", msg->type);

}