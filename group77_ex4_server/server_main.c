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
	// Checks whether the given arguments are valid  
	char test[5] = { 'a', 'b','c','d', '\n' };
	char test_decoder[20] = { 't', '1',':', 'p', '1',';','p', '2','p', '3','p', '4','\n' };
	int len = 0, ret_val = TRUE;
	Messege msg;



	if (ensureArgs(argc, SERVER_EXPECTED_ARGC, argv) != TRUE) {
		raiseError(1, __FILE__, __func__, __LINE__, ERROR_ID_1_ARGS);
		return TRUE;
	}


	ret_val = initMessege(&msg, "Type", "Param1", "Param2", NULL, NULL, NULL);
	printMessege(&msg);
	getEncodeMessegeLength(&msg, &len);
	printf("Encoded Messege Length: %d\n", len);

	freeMessege(&msg);

	//MainServer(argv[1]);

	decodeMsg(test_decoder, &msg);
	printMessege(&msg);

	return 0;
}