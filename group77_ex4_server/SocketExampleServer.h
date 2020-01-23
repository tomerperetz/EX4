#pragma once
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering, Winter 2011, 
 by Amnon Drory.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#ifndef SOCKET_EXAMPLE_SERVER_H
#define SOCKET_EXAMPLE_SERVER_H
#include "../Shared/hardCodedData.h"
#include "../Shared/SocketSendRecvTools.h"

#define USERNAME_MAX_LEN 20
#define DONT_CLOSE_BRUTALLY 3
#define CLOSE_BRUTALLY = 1
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
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
} User;


void MainServer(char port_num_char[5]);
static DWORD exitProgramThread();


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif // SOCKET_EXAMPLE_SERVER_H