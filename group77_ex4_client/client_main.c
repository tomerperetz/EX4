
/*
====================================================================================================================
Authors:
	- Segev Elmalem, ID: 203149000
	- Tomer Peretz, ID: 305002206
Project: EX4
Input:
Outputs: <
Description:
====================================================================================================================
*/

// Includes --------------------------------------------------------------------
#include "../Shared/hardCodedData.h"
#include "SocketExampleClient.h"

// Functions --------------------------------------------------------------------
int main(int argc, char *argv[])
{	/*
	Description: main. recieves user args and parse it to structs.
	parameters:
			 - int argc - num of args
			 - char* argv[] - args
	Returns: 0 if succeded
	*/
	// Checks whether the given arguments are valid  


	if (ensureArgs(argc, CLIENT_EXPECTED_ARGC, argv) != TRUE) {
		raiseError(1, __FILE__, __func__, __LINE__, ERROR_ID_1_ARGS);
		return TRUE;
	}
	//runClientTest();
	MainClient(argv[1], argv[2]);
	return 0;
}