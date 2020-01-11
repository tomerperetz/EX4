/*
====================================================================================================================
description:
operation system functions: proccesses and threads.
====================================================================================================================
*/

// includes ---------------------------------------------------------------------------
#include "lib_oshandler.h"


//// Thread Function Definitions --------------------------------------------------------



int closeHandles(const HANDLE *p_thread_handles, int size) {
	/*
	Description: create thread with arg wrapper
	parameters:
			- HANDLE *p_thread_handles - array of handles
			- int size - size of handles araay
	Return: true if all the handles closed safly 
	*/
	int idx = 0, retVal = TRUE;
	for (idx = 0; idx < size; idx++) {
		if (p_thread_handles[idx] != NULL) {
			if (CloseHandle(p_thread_handles[idx]) == FALSE) {
				raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
				printf("Details: Error when closing thread\n");
				retVal = FALSE;
			}
		}
	}
	return retVal;
}

