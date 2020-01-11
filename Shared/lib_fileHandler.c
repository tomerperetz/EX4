/*
====================================================================================================================
Description:
File handeling functions
====================================================================================================================
*/

// Includes --------------------------------------------------------------------
#include "lib_fileHandler.h"


// Functions --------------------------------------------------------------------

void freeFilesList(char **files, int max_files)
{
	/*
	Description: free memory allocation for files path array
	parameters:
			- char **files - array of strings
	Returns: VOID
	*/

	for (int i = 0; i < max_files; i++)
	{
		if (files[i] != NULL)
			free(files[i]);
	}
	if (files != NULL)
		free(files);

}

