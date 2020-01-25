#pragma once
/*
=================================================================================
							Description
=================================================================================
		Socket shared functions and defines
=================================================================================
*/

#ifndef SOCKET_EXAMPLE_SHARED_H
#define SOCKET_EXAMPLE_SHARED_H

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define SERVER_ADDRESS_STR "127.0.0.1"
#define IP_MAX_LEN 20
#define PORT_MAX_LEN 33
#define WAITING_TIME_MILLI_SEC 1500
#define WAIT_FOR_ALL_RESOURCE_TO_CLOSE 60000
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

#define CLIENT_REQUEST "CLIENT_REQUEST"
#define CLIENT_MAIN_MENU "CLIENT_MAIN_MENU"
#define CLIENT_CPU "CLIENT_CPU"
#define CLIENT_VERSUS "CLIENT_VERSUS"
#define CLIENT_PLAYER_MOVE "CLIENT_PLAYER_MOVE"
#define CLIENT_REPLAY "CLIENT_REPLAY"
#define CLIENT_REFRESH "CLIENT_REFRESH"
#define CLIENT_DISCONNECT "CLIENT_DISCONNECT"

#define SERVER_MAIN_MENU "SERVER_MAIN_MENU"
#define SERVER_APPROVED "SERVER_APPROVED"
#define SERVER_DENIED "SERVER_DENIED"
#define SERVER_INVITE "SERVER_INVITE"
#define SERVER_PLAYER_MOVE_REQUEST "SERVER_PLAYER_MOVE_REQUEST"
#define SERVER_GAME_RESULTS "SERVER_GAME_RESULTS"
#define SERVER_GAME_OVER_MENU "SERVER_GAME_OVER_MENU"
#define SERVER_OPPONENT_QUIT "SERVER_OPPONENT_QUIT"
#define SERVER_NO_OPPONENT "SERVER_NO_OPPONENT"


typedef struct _Socket_info
{
	SOCKET socket;
	char ip_addres[IP_MAX_LEN];
	char port_num_char[PORT_MAX_LEN];
	SOCKADDR_IN sock_adrr;
	BOOL server_not_client;
} Socket_info;


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif 