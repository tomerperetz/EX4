
/*
====================================================================================================================
Description:
this lib is used for error handeling.
====================================================================================================================
*/


#pragma once
#ifdef _MSC_VER
#endif

// Includes --------------------------------------------------------------------------------------------------------
#include "hardCodedData.h"


// Defines ---------------------------------------------------------------------------------------------------------

# define STRCPY_ERROR "String copy error. please reset the program.\n"
static const char ERROR_ID_1_ARGS[170] = \
"ERROR ID 1: The given arguments are not in the correct format / number of arguments is incorrect.\n"\
"For more help please run the following command: ~<Project-Name> --help\n";
static const char ERROR_ID_2_IO[100] = \
"ERROR ID 2: IO ERROR has occured.\nProgram can't open the file\n";
static const char ERROR_ID_3_TIMEOUT[100] = \
"ERROR ID 3: Run Time Error: Process was timeout!\n" \
"Terminating brutally!\n";
static const char ERROR_ID_4_MEM_ALLOCATE[100] = \
"Allocation Error: Memory allocation failed!\n" \
"Terminating brutally!\n";
static const char ERROR_ID_5_STRING_H[100] = \
"STRLIB ERROR: Use of one of the functions from the <string.h> directory failed\n";
static const char ERROR_ID_6_THREADS[100] = \
"THREADS ERROR: Error occurred in threads creating function. please restart the program.\n";
static const char ERROR_ID_7_OTHER[100] = \
" Error occurred.\n";
static const char ERROR_ID_8_INTER_THREAD[100] = \
"Error occurred in one or more threads. ";
static const char ERROR_ID_9_CONNECTION[100] = \
"Error occurred in connection. \n ";
// Declerations -------------------------------------------------------------------------------------------------------

void raiseError(int error_id, const char *file_path, const char *function_name, int line, const char *description);

