/*
====================================================================================================================
Description:
this lib is a parser for arguments recieved from user.
====================================================================================================================
*/

#include "argparser.h"


int ensureArgs(int argc, int expected_argc, char *argv[])
{
	/*
	Description: recieves user args and verify they are valid.
	parameters:
			 - int argc - number of args recieved.
			 - int expected_argc - number of args expected.
			 - char argv - args
	Returns: IS_TRUE if the args are valid, ERR o.w
	*/

	if (argc == 2 && (strcmp(argv[1], "-h") == EQUAL || strcmp(argv[1], "--help") == EQUAL) )
	{
		callHelper();
		return IS_TRUE;
	}
	else if (argc == expected_argc){
		if (isArgsValid(argc, argv) == IS_TRUE)
			return IS_TRUE;
		else
			return ERR;
	}
	else {
		return ERR;
	}

}

int isDirectory(const char *path) {
	/*
	Description: recieves a path and verify they are valid.
	parameters:
			 - const char *path - directory path
	Returns: IS_TRUE if the path is valid, ERR o.w
	*/
	struct stat statbuf;
	if (stat(path, &statbuf) != 0)
		return ERR;
	return IS_TRUE;
}

int isArgsValid(int argc, char *argv[])
{
	/*
	Description: future function for this lib. will be used in the following projects for more complexed inputs.
	*/
	if (isDirectory(argv[1]) != IS_TRUE)
	{
		return ERR;
	}
	return IS_TRUE;
}

void callHelper() {
	/*
	Description: prints help for the program and exits.
	parameters: none
	Returns: none
	*/

	printf(HELPER);
	exit(IS_FALSE);
}