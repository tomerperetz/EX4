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


// Functions --------------------------------------------------------------------
int main(int argc, char *argv[])
{	/*
	Description: main. recieves user args and parse it to structs.
	parameters:
			 - int argc - num of args
			 - char* argv[] - args
	Returns: 0 if succeded
	*/

	int len = 0, ret_val = TRUE;

	// Checks whether the given arguments are valid  
	if (ensureArgs(argc, SERVER_EXPECTED_ARGC, argv) != TRUE) {
		raiseError(1, __FILE__, __func__, __LINE__, ERROR_ID_1_ARGS);
		return TRUE;
	}

	MainServer(argv[1]);

	return 0;
}