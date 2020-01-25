/*
====================================================================================================================
Description:
Operation System functions: Proccesses and Threads.
====================================================================================================================
*/

#pragma once
#ifdef _MSC_VER
#endif

// Includes --------------------------------------------------------------------
#include "hardCodedData.h"
#define THREAD_ERR 0x55

// Declerations ------------------------------------------------------------------------
// Thread Functions

int closeHandles(const HANDLE *p_thread_handles, int size);