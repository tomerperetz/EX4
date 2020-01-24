
/*
====================================================================================================================
Authors:
	- Segev Elmalem, ID: 203149000
	- Tomer Peretz, ID: 305002206
Project: group77_ex4_client
Input: <server ip> <port num> <user name>
Outputs: 0
Description: Connect to server and play "Rock, paper, scissors, lizard, spock" vs cpu or vs other player. 
====================================================================================================================
*/

// Includes -------------------------------------------------------------------->
#include "../Shared/hardCodedData.h"
#include "SocketClient.h"

// Functions -------------------------------------------------------------------->
int main(int argc, char *argv[])
{	/*
	Description: main. recieves user args [<server ip> <port num> <user name>] and uses it to init connection to server
	parameters:
			 - int argc - num of args
			 - char* argv[] - args
	Returns: 0 
	*/

	// Checks whether the given arguments are valid  
	int exit_code = RECONNECT;

	if (ensureArgs(argc, CLIENT_EXPECTED_ARGC, argv) != TRUE) {
		raiseError(1, __FILE__, __func__, __LINE__, ERROR_ID_1_ARGS);
		return 0;
	}
	// to reconnect loop: if connection failed with reconnect code we will restart program, o.w exit
	int try_to_reconnect = FALSE;
	while (exit_code == RECONNECT)
	{
		exit_code = MainClient(argv[1], argv[2], argv[3], try_to_reconnect);
		try_to_reconnect = TRUE;
	}		
	printf("Thank You for playing, we hope that you had a good time :).\n");

	return 0;
}