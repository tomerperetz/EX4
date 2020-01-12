/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "SocketExampleClient.h"
#include "../Shared/SocketExampleShared.h"
#include "../Shared/SocketSendRecvTools.h"


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

SOCKET m_socket;

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Reading data coming from the server
static DWORD RecvDataThread(void)
{
	int ret_val = TRUE;
	while (1)
	{

		Messege msg_struct;
		ret_val = decodeWrapper(&msg_struct, &m_socket);
		if (ret_val == ERR) {
			/*TO DO*/
		}
		printMessege(&msg_struct);
		// This is where we will use the server state machine
		// for demo only print do something and send insulting text to client
		printf("#########################################\n\n");
		printf("This is the state machine \n\n");
		printf("#########################################\n\n");
		freeMessege(&msg_struct);
	}
	return 0;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Sending data to the server
static DWORD SendDataThread(void)
{
	char SendStr[256];
	//TransferResult_t SendRes;

	while (1) 
	{
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard

		if ( STRINGS_ARE_EQUAL(SendStr,"quit") ) 
			return 0x555; //"quit" signals an exit from the client side
		
		// Old send implementation from class

		//SendRes = SendString( SendStr, m_socket);
		//	
		//if ( SendRes == TRNS_FAILED ) 
		//{
		//	printf("Socket error while trying to write data to socket\n");
		//	return 0x555;
		//}

		sendMessegeWrapper(m_socket, "SendDataThread_type", "param1", "param2", "param3", "param4", "param5");
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
char *getUserAnswer(FILE* fp)
{
	//The size is extended by the input with the value of the provisional
	char *str;
	int ch;
	size_t len = 0;
	size_t size = 10;
	str = realloc(NULL, sizeof(char)*size);//size is start size
	if (str == NULL) return str;
	while (EOF != (ch = fgetc(fp)) && ch != '\n') {
		str[len++] = ch;
		if (len == size) {
			str = realloc(str, sizeof(char)*(size += 16));
			if (str == NULL) return str;
		}
	}
	str[len++] = '\0';

	return realloc(str, sizeof(char)*len);
}

void tryToReconnect(int *answer)
{
	char *user_answer;
	BOOL done = FALSE;
	do {
		printf("Choose what to do next:\n");
		printf("1. Try to reconnect\n");
		printf("2. Exit\n");
		user_answer = getUserAnswer(stdin);
		if (user_answer == NULL) {
			raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
			*answer = ERR;
		}
		if (strcmp(user_answer, "1") == EQUAL || strcmp(user_answer, "2") == EQUAL) {
			done = TRUE;
			*answer = atoi(user_answer);
		}
		free(user_answer);
	} while (!done);
}
void MainClient(char *ip_addres, char *port_num_char)
{
	SOCKADDR_IN clientService;
	HANDLE hThread[2];
	int try_to_reconnect_answer;
    // Initialize Winsock.
    WSADATA wsaData; //Create a WSADATA object called wsaData.
	//The WSADATA structure contains information about the Windows Sockets implementation.
	
	//Call WSAStartup and check for errors.
    int iResult = WSAStartup( MAKEWORD(2, 2), &wsaData );
    if ( iResult != NO_ERROR )
        printf("Error at WSAStartup()\n");

	//Call the socket function and return its value to the m_socket variable. 
	// For this application, use the Internet address family, streaming sockets, and the TCP/IP protocol.
	
	// Create a socket.
    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	// Check for errors to ensure that the socket is a valid socket.
    if ( m_socket == INVALID_SOCKET ) {
        printf( "Error at socket(): %ld\n", WSAGetLastError() );
        WSACleanup();
        return;
    }
	/*
	 The parameters passed to the socket function can be changed for different implementations. 
	 Error detection is a key part of successful networking code. 
	 If the socket call fails, it returns INVALID_SOCKET. 
	 The if statement in the previous code is used to catch any errors that may have occurred while creating 
	 the socket. WSAGetLastError returns an error number associated with the last error that occurred.
	 */


	//For a client to communicate on a network, it must connect to a server.
    // Connect to a server.

    //Create a sockaddr_in object clientService and set  values.
    clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(ip_addres); //Setting the IP address to connect to
    clientService.sin_port = htons( atoi(port_num_char) ); //Setting the port to connect to.
	
	/*
		AF_INET is the Internet address family. 
	*/


    // Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	do {
		if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
			printf("Failed to connecting to server on %s:%s.\n", ip_addres, port_num_char);
			tryToReconnect(&try_to_reconnect_answer);
			if (try_to_reconnect_answer == EXIT_PROGRAM || try_to_reconnect_answer == ERR) {
				WSACleanup();
				return;
			}
		}
		else
			try_to_reconnect_answer = CONNECTED;
	} while (try_to_reconnect_answer == RECONNECT);
	
			
        
    

    // Send and receive data.
	/*
		In this code, two integers are used to keep track of the number of bytes that are sent and received. 
		The send and recv functions both return an integer value of the number of bytes sent or received, 
		respectively, or an error. Each function also takes the same parameters: 
		the active socket, a char buffer, the number of bytes to send or receive, and any flags to use.

	*/	

	hThread[0]=CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE) SendDataThread,
		NULL,
		0,
		NULL
	);
	hThread[1]=CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE) RecvDataThread,
		NULL,
		0,
		NULL
	);

	WaitForMultipleObjects(2,hThread,FALSE,INFINITE);

	TerminateThread(hThread[0],0x555);
	TerminateThread(hThread[1],0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	
	closesocket(m_socket);
	
	WSACleanup();
    
	return;
}

