/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "SocketExampleServer.h"
#include "../Shared/SocketExampleShared.h"
#include "../Shared/SocketSendRecvTools.h"

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define NUM_OF_WORKER_THREADS 2

#define MAX_LOOPS 3

#define SEND_STR_SIZE 35

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
static DWORD ServiceThread( SOCKET *t_socket );

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void MainServer(char port_num_char[5])
{
	int Ind;
	int Loop;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	int port_num_int = atoi(port_num_char);

	// Initialize Winsock.
    WSADATA wsaData;
    int StartupRes = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );	           

    if ( StartupRes != NO_ERROR )
	{
        printf( "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError() );
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}
 
    /* The WinSock DLL is acceptable. Proceed. */

    // Create a socket.    
    MainSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( MainSocket == INVALID_SOCKET ) 
	{
        printf( "Error at socket( ): %ld\n", WSAGetLastError( ) );
		goto server_cleanup_1;
    }

    // Bind the socket.
	/*
		For a server to accept client connections, it must be bound to a network address within the system. 
		The following code demonstrates how to bind a socket that has already been created to an IP address 
		and port.
		Client applications use the IP address and port to connect to the host network.
		The sockaddr structure holds information regarding the address family, IP address, and port number. 
		sockaddr_in is a subset of sockaddr and is used for IP version 4 applications.
   */
	// Create a sockaddr_in object and set its values.
	// Declare variables

	Address = inet_addr( SERVER_ADDRESS_STR );
	if ( Address == INADDR_NONE )
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
				SERVER_ADDRESS_STR );
		goto server_cleanup_2;
	}

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = Address;
    service.sin_port = htons(port_num_int); //The htons function converts a u_short from host to TCP/IP network byte order 
	                                   //( which is big-endian ).
	/*
		The three lines following the declaration of sockaddr_in service are used to set up 
		the sockaddr structure: 
		AF_INET is the Internet address family. 
		"127.0.0.1" is the local IP address to which the socket will be bound. 
	    2345 is the port number to which the socket will be bound.
	*/

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
    bindRes = bind( MainSocket, ( SOCKADDR* ) &service, sizeof( service ) );
	if ( bindRes == SOCKET_ERROR ) 
	{
        printf( "bind( ) failed with error %ld. Ending program\n", WSAGetLastError( ) );
		goto server_cleanup_2;
	}
    
    // Listen on the Socket.
	ListenRes = listen( MainSocket, SOMAXCONN );
    if ( ListenRes == SOCKET_ERROR ) 
	{
        printf( "Failed listening on socket, error %ld.\n", WSAGetLastError() );
		goto server_cleanup_2;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
		ThreadHandles[Ind] = NULL;

    printf( "Waiting for a client to connect...\n" );
    
	for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )
	{
		SOCKET AcceptSocket = accept( MainSocket, NULL, NULL );
		if ( AcceptSocket == INVALID_SOCKET )
		{
			printf( "Accepting connection with client failed, error %ld\n", WSAGetLastError() ) ; 
			goto server_cleanup_3;
		}

        printf( "Client Connected.\n" );

		Ind = FindFirstUnusedThreadSlot();

		if ( Ind == NUM_OF_WORKER_THREADS ) //no slot is available
		{ 
			printf( "No slots available for client, dropping the connection.\n" );
			closesocket( AcceptSocket ); //Closing the socket, dropping the connection.
		} 
		else 	
		{
			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close 
											  // AcceptSocket, instead close 
											  // ThreadInputs[Ind] when the
											  // time comes.
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				( LPTHREAD_START_ROUTINE ) ServiceThread,
				&( ThreadInputs[Ind] ),
				0,
				NULL
			);
		}
    } // for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )

server_cleanup_3:

	CleanupWorkerThreads();

server_cleanup_2:
	if ( closesocket( MainSocket ) == SOCKET_ERROR )
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError() ); 

server_cleanup_1:
	if ( WSACleanup() == SOCKET_ERROR )		
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError() );
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot()
{ 
	int Ind;

	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
	{
		if ( ThreadHandles[Ind] == NULL )
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject( ThreadHandles[Ind], 0 ); 
				
			if ( Res == WAIT_OBJECT_0 ) // this thread finished running
			{				
				CloseHandle( ThreadHandles[Ind] );
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static void CleanupWorkerThreads()
{
	int Ind; 

	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
	{
		if ( ThreadHandles[Ind] != NULL )
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject( ThreadHandles[Ind], INFINITE ); 
				
			if ( Res == WAIT_OBJECT_0 ) 
			{
				closesocket( ThreadInputs[Ind] );
				CloseHandle( ThreadHandles[Ind] );
				ThreadHandles[Ind] = NULL;
				break;
			}
			else
			{
				printf( "Waiting for thread failed. Ending program\n" );
				return;
			}
		}
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
static DWORD ServiceThread( SOCKET *t_socket ) 
{
	char SendStr[SEND_STR_SIZE];
	int ret_val = TRUE;
	BOOL Done = FALSE;
	strcpy( SendStr, "Welcome to this server!" );
	
	ret_val = sendMessegeWrapper(*t_socket, "welcome_msg",NULL, NULL, NULL, NULL, NULL);
	if (ret_val == ERR) {
		/*TO DO*/
	}
	while ( !Done ) 
	{		
		
		Messege msg_struct;
		ret_val = decodeWrapper(&msg_struct, t_socket);
		if (ret_val == ERR) {
			/*TO DO*/
		}
		printMessege(&msg_struct);	


		// This is where we will use the server state machine
		// for demo only print do something and send insulting text to client

		printf("#########################################\n\n");
		printf("This is the state machine \n\n");
		printf("#########################################\n\n");

		sendMessegeWrapper(*t_socket, "ServiceThread_type", "param1", "param2", "param3",
			"param4", "param5");
		freeMessege(&msg_struct);
	}
	
	printf("Conversation ended.\n");
	closesocket( *t_socket );
	return 0;
}
