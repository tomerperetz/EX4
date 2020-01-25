#pragma once
/*
//==========================================================================
//					Description
//==========================================================================
		public functions for socket client
============================================================================
*/

#include "../Shared/hardCodedData.h"
#include "../Shared/SocketShared.h"


#ifndef SOCKET_EXAMPLE_CLIENT_H
#define SOCKET_EXAMPLE_CLIENT_H
#define RECONNECT_MENU "Choose what to do next:\n1. Try to reconnect\n2. Exit\n"


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
static DWORD RecvDataThread(void);
static DWORD SendDataThread(void);
int MainClient(char *ip_addres, char *port_num_char, char *user_name, int );
void printMenuAndGetAnswer(char *menu, int *answer, int max_menu_option);

int tryToReconnect(Socket_info *socket_data, int * try_to_reconnect_answer);

int createProgramSemaphores();

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif