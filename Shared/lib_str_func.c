/*
====================================================================================================================
Description:
This lib contains all functions related to string handeling
====================================================================================================================
*/

// Includes --------------------------------------------------------------------------------------------------------
#include "lib_str_func.h"

// Functions -------------------------------------------------------------------------------------------------------
int strcatDynamic(const char first_source[], const char second_source[], char **buffer)
{
	/*
	Description: append two strings into a buffer 
	parameters:
			 - const char first_source[] - first string
			 - const char second_source[] - second string
			 - char **buffer - NULL buffer this will contain the attached string
	Return: TRUE if succeded, ERR o.w
	*/

	int retVal1 = ERR, retVal2 = ERR, size_of_file_path = 0;

	size_of_file_path = (strlen(first_source) + strlen(second_source) + 1);
	*buffer = (char*)malloc(sizeof(char)*size_of_file_path);
	if (*buffer == NULL) {
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
		return FALSE;
	}
	retVal1 = strcpy_s(*buffer, sizeof(char)*size_of_file_path, first_source);
	retVal2 = strcat_s(*buffer, sizeof(char)*size_of_file_path, second_source);
	if (retVal1 != 0 || retVal2 != 0) {
		raiseError(9, __FILE__, __func__, __LINE__, ERROR_ID_5_STRING_H);
		free(*buffer);
		return ERR;
	}
	return TRUE;
}

void str_safe_free(char *ptr)
{
	/*
	Description: free char poiner
	parameters:
			 - char *ptr - pointer
	Return: VOID
	*/

	if (ptr != NULL)
		free(ptr);
}