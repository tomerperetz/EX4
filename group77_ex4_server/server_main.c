/*
====================================================================================================================
Authors:
	- Segev Elmalem, ID: 203149000
	- Tomer Peretz, ID: 305002206
Project: group77_ex4_server
Input: <port number>
Outputs: -
Description: "Rock, Paper, Scissors, Lizard, Spock" game server. allows 2 users to connect and play vs cpu, 
				vs each other or exit program.
====================================================================================================================
*/

// Includes --------------------------------------------------------------------
#include "../Shared/hardCodedData.h"
#include "server_services.h"

// Functions --------------------------------------------------------------------
int main(int argc, char *argv[])
{	/*
	Description: main. recieves user args and init the server. looks for "GameSession.txt" file, and delete it if it exists.
	parameters:
			 - int argc - num of args
			 - char* argv[] - args
	Returns: 0
	*/

	SOCKET socket = INVALID_SOCKET;
	int len = 0, ret_val = TRUE;

	// Checks whether the given arguments are valid  
	if (ensureArgs(argc, SERVER_EXPECTED_ARGC, argv) != TRUE) {
		raiseError(1, __FILE__, __func__, __LINE__, ERROR_ID_1_ARGS);
		return 0;
	}

	if (!seekAndDestroy())
	{
		printf("Please delete manually or remove read only from  gameSession.txt file and restart program.\n");
		return 0;
	}

	MainServer(argv[1]);

	return 0;
}