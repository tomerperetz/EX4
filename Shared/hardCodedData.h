/*
====================================================================================================================
Description:
Hard coded data: includes and defines
====================================================================================================================
*/

// Pragma ------------------------------------------------------>
#pragma once
#ifdef _MSC_VER
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN


// Includes --------------------------------------------------->
#include <stdio.h>	
#include <string.h>
#include <windows.h>
#include <sys/stat.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#include "argparser.h"
#include "lib_errorHandler.h"
#include "lib_osHandler.h"
#include "SocketSendRecvTools.h"
#include "../group77_ex4_server/SocketExampleServer.h"
#include "../group77_ex4_client/SocketExampleClient.h"
#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"
#include "../group77_ex4_client/client_services.h"

// Consts ----------------------------------------------------->
static const int ERR = -1;
static const int EQUAL = 0;
static const char END_OF_STR = '\0';
static const char END_OF_MSG = '\n';


//defines ----------------------------------------------------->
#define NAME_MAX_LEN 20
#define SERVER_EXPECTED_ARGC 1
#define CLIENT_EXPECTED_ARGC 3
