/*
====================================================================================================================
Description:
File handeling functions
====================================================================================================================
*/

// Includes --------------------------------------------------------------------

#pragma once
#include "./hardCodedData.h"
#include "lib_errorHandler.h"
#include "lib_str_func.h"
#include "lib_osHandler.h"
#include <Windows.h>

// Function Definitions --------------------------------------------------------

int readGradeFile(char filename[], int *grade);

DWORD WINAPI readGradeFileThread(LPVOID lpParam);

void freeFilesList(char **files);

int getStudentID(char *path, char *ID);

void str_safe_free(char *ptr);

int printToFile(char *filename, int final_grade);