#pragma once
/*
=================================================================================
							Description
=================================================================================
		Socket server headers and structs
=================================================================================
*/

// Includes ------------------------------------------------------------------>
#ifndef SOCKET_EXAMPLE_SERVER_H
#define SOCKET_EXAMPLE_SERVER_H
#include "../Shared/hardCodedData.h"
#include "../Shared/SocketSendRecvTools.h"

// Defines ------------------------------------------------------------------>
#define USERNAME_MAX_LEN 20
#define DONT_CLOSE_BRUTALLY 3
#define CLOSE_BRUTALLY 1
#define DONT_KNOW 2

// Structs ------------------------------------------------------------------>
typedef struct _Player
{
	int win;
	int loss;
	char name[USERNAME_MAX_LEN];
} Player;

typedef struct _User
{
	Player *player_data;
	int status;
	int idx;
	BOOL online;
	int play_vs_again;
} User;

// Declerations -------------------------------------------------------------->
void MainServer(char port_num_char[5]);
static DWORD exitProgramThread();


#endif 