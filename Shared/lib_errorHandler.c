
/*
====================================================================================================================
Description:
this lib is used for error handeling.
====================================================================================================================
*/

#include "lib_errorHandler.h"


void raiseError(int error_id,const char *file_path, const char *function_name, int line, const char *description)
{
	/*
	Description: Error handling function. prints error description and location and exists the program.
	parameters:
			 - int error id
			 - char* function name
			 - char* file path - path to the script containing the error
			 - int line - line in the code where the error appeared
			 - description - error description.
	Returns: exit with ERR
	*/
	printf("=====================================================================\n");
	printf("FATAL ERROR: ERROR NUMBER %d\n", error_id);
	printf("File: %s\n", file_path);
	printf("Function: %s\n", function_name);
	printf("Line: %d\n", line);
	printf("Description: %s\n", description);
	printf("=====================================================================\n");
}
