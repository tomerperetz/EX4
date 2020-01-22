#pragma once
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering, Winter 2011, 
 by Amnon Drory.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/


#include "../Shared/hardCodedData.h"
#include "../Shared/SocketExampleShared.h"


#ifndef SOCKET_EXAMPLE_CLIENT_H
#define SOCKET_EXAMPLE_CLIENT_H


#define RECONNECT_MENU "Choose what to do next:\n1. Try to reconnect\n2. Exit\n"
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
static DWORD RecvDataThread(void);
static DWORD SendDataThread(void);
int MainClient(char *ip_addres, char *port_num_char, char *user_name);
void printMenuAndGetAnswer(char *menu, int *answer, int max_menu_option);

int tryToReconnect(Socket_info *socket_data, int * try_to_reconnect_answer);

int createProgramSemaphores();


//int clientStateMachine(Messege *msg_in, Messege *msg_out);
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif // SOCKET_EXAMPLE_CLIENT_H