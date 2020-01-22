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

#include "SocketExampleServer.h"
#include "../Shared/SocketExampleShared.h"
#include "../Shared/SocketSendRecvTools.h"
#include "server_services.h"


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define NUM_OF_WORKER_THREADS 2
#define MAX_LOOPS 3
#define SEND_STR_SIZE 35

// Globals ------------------------------------------------------------------>
HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
SOCKET MainSocket = INVALID_SOCKET;
static HANDLE end_program_semaphore;
int force_exit_flag = FALSE;
int main_socket_is_closed = FALSE;
int close_brutally[NUM_OF_WORKER_THREADS];
int close_brutally_main = TRUE;

// Local functions declerations ------------------------------------------>
static int FindFirstUnusedThreadSlot();
static void closeProgramNicely();
static DWORD ServiceThread(int *threadIdx);
static DWORD exitProgramThread();
void closeControlersThreadsAndResources(HANDLE *exit_program_handle);
int createServerSemaphores();

// Functions -------------------------------------------------------------->

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void MainServer(char port_num_char[5])
{
	extern SOCKET MainSocket;
	int Ind;
	//int Loop;
	HANDLE exit_program_handle;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	int port_num_int = atoi(port_num_char);
	int ret_val = TRUE;
	// Initialize Winsock.
    WSADATA wsaData;
	DWORD wait_code;

	close_brutally[0] = FALSE;
	close_brutally[1] = FALSE;
	createServerSemaphores();

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
	
	exit_program_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) exitProgramThread, NULL, 0, NULL);
	if (exit_program_handle == NULL)
	{
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
		printf("details: Error when creating thread\n");
		ret_val = ERR;
		goto server_cleanup_2;
	}
	
	//for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )
	while (TRUE)
	{

		SOCKET AcceptSocket = accept( MainSocket, NULL, NULL );
		
		if ( AcceptSocket == INVALID_SOCKET && (force_exit_flag == FALSE))
		{
			printf( "Accepting connection with client failed, error %ld\n", WSAGetLastError() ) ; 
			goto server_cleanup_3;
		}

		//Checks whether while waiting to the Socket connection someone shutdwon the server
		if (force_exit_flag == TRUE || force_exit_flag == ERR)
		{
			close_brutally_main = FALSE;
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
			ThreadHandles[Ind] = CreateThread(NULL,0,( LPTHREAD_START_ROUTINE ) ServiceThread,
														&( Ind ),0,NULL);
		}
		
    } // for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )

server_cleanup_3:
	wait_code = WaitForSingleObject(end_program_semaphore, WAIT_FOR_ALL_RESOURCE_TO_CLOSE);
	ret_val = checkWaitCodeStatus(wait_code, TRUE);
	if (ret_val != TRUE) {
		printf("Terminating all open resources...\n");
		TerminateThread(exit_program_handle, THREAD_ERR);
	}
	
server_cleanup_2:
	if (!main_socket_is_closed) {
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
	}
server_cleanup_1:
	closeControlersThreadsAndResources(&exit_program_handle);
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

static void closeProgramNicely()
{
	int Ind; 
	DWORD wait_code;
	int ret_val;
	LPDWORD lpExitCode = 0;
	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
	{
		if ( ThreadHandles[Ind] != NULL && close_brutally[Ind] == FALSE)
		{
			// poll to check if thread finished running:
			printf("Waiting.....\n");
			wait_code = WaitForSingleObject(ThreadHandles[Ind], INFINITE);
			ret_val = checkWaitCodeStatus(wait_code, TRUE);
			if (GetExitCodeThread(ThreadHandles[Ind], lpExitCode) == 0) {
				printf("Failed to get exitcode, error %ld. Ending program.\n", GetLastError());
				raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
			}
			if (ret_val == TRUE)
				CloseHandle(ThreadHandles[Ind]);
			else
				TerminateThread(ThreadHandles[Ind], THREAD_ERR);
			if (closesocket(ThreadInputs[Ind]) != 0) {
				raiseError(10, __FILE__, __func__, __LINE__, ERROR_ID_10_SOCKET);
			}
			
		}
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
static DWORD ServiceThread(int *threadIdx ) 
{
	extern force_exit_flag;
	int ret_val = TRUE;
	BOOL Done = FALSE;

	SOCKET *t_socket;
	Player player;
	Messege first_msg;


	t_socket = &ThreadInputs[*threadIdx];
	close_brutally[*threadIdx] = TRUE;
	initMessege(&first_msg, NULL, NULL, NULL, NULL, NULL, NULL);
	player.win = 0;
	player.loss = 0;


	ret_val = decodeWrapper(&first_msg, &ThreadInputs[*threadIdx]);
	if (!STRINGS_ARE_EQUAL(first_msg.type, CLIENT_REQUEST) || ret_val != TRUE) {
		freeMessege(&first_msg);
		goto MAIN_CLEAN;
	}
	else {
		strcpy_s(player.name, USERNAME_MAX_LEN, first_msg.params[0]);
		printf("Player name: %s\n", player.name);
		freeMessege(&first_msg);
	}
	
	ret_val = sendMessegeWrapper(*t_socket, SERVER_APPROVED, NULL, NULL, NULL, NULL, NULL);
	
	
	if (ret_val == ERR) {
		/*TO DO*/
	}
	while ( !Done ) 
	{		
		
		Messege msg_struct;

		initMessege(&msg_struct, NULL, NULL, NULL, NULL, NULL, NULL);
		ret_val = sendMessegeWrapper(*t_socket, SERVER_MAIN_MENU, NULL, NULL, NULL, NULL, NULL);


		ret_val = decodeWrapper(&msg_struct, t_socket);
		printMessege(&msg_struct);

		if (STRINGS_ARE_EQUAL(msg_struct.type, CLIENT_CPU)) {
			ret_val = client_vs_cpu(t_socket, &player);
			if (ret_val != TRUE) goto MAIN_CLEAN;
		}
		if (STRINGS_ARE_EQUAL(msg_struct.type, CLIENT_DISCONNECT)) {
			//send end messege
			Done = TRUE;
		}

		freeMessege(&msg_struct);

	}
	
MAIN_CLEAN:
	shutdown(*t_socket, SD_SEND);
	closesocket( *t_socket );
	return 0;
}

static DWORD exitProgramThread()
{
	extern SOCKET MainSocket;
	extern int main_socket_is_closed;
	extern int force_exit_flag;
	extern HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
	extern SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
	int exit_code = TRUE;

	while (TRUE) 
	{
		char *exit_str;
		exit_str = getString(stdin);
		if (exit_str == NULL) {
			force_exit_flag = ERR;
			break;
		}

		lowerCase(exit_str);
		if (STRINGS_ARE_EQUAL(exit_str, "exit")) {
			force_exit_flag = TRUE;
			free(exit_str);
			break;
		}

		free(exit_str);
	}
	printf("+++++++++++++++++++++++++++++++++++++++\n");
	printf("Closing All Resources And Exiting...\n");
	printf("Waiting For All Resources To Close...\n");
	if (ThreadInputs[0] != INVALID_SOCKET) shutdown(ThreadInputs[0], SD_SEND);
	if (ThreadInputs[1] != INVALID_SOCKET) shutdown(ThreadInputs[1], SD_SEND);
	Sleep(WAITING_TIME_MILLI_SEC);
	if (close_brutally_main) {
		// Usig Semaphore here is Optional.
		main_socket_is_closed = TRUE;
		printf("Done waiting to main socket, Closing bruttaly...\n");
		if (closesocket(MainSocket) != 0) {
			raiseError(10, __FILE__, __func__, __LINE__, ERROR_ID_10_SOCKET);
		}
		
	}


	for (int idx = 0; idx < NUM_OF_WORKER_THREADS; idx++) {
		if (close_brutally[idx]) {
			printf("Done waiting to socket #%d, Closing bruttaly...\n",idx);
			if (TerminateThread(ThreadHandles[idx], THREAD_ERR) == 0)
				raiseError(10, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
			if (closesocket(ThreadInputs[idx]) != 0) {
				raiseError(10, __FILE__, __func__, __LINE__, ERROR_ID_10_SOCKET);
			}
		}
	}

	closeProgramNicely();
	printf("All Resources has been closed\n");
	printf("+++++++++++++++++++++++++++++++++++++++\n");
	ReleaseSemaphore(end_program_semaphore, 1, NULL);
	return TRUE;
}

void closeControlersThreadsAndResources(HANDLE *exit_program_handle) {
	
	if (CloseHandle(*exit_program_handle) == 0) {
		printf("Failed to close 'Exit Handle. ERROR NUMBER %ld\nAborting...", GetLastError());
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
		return;
	}
	if (end_program_semaphore != NULL) {
		if (CloseHandle(end_program_semaphore) == 0) {
			printf("Failed to close 'Exit Handle. ERROR NUMBER %ld\nAborting...", GetLastError());
			raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
			return;
		}
	}
}

int createServerSemaphores()
{

	/*
		Description: init global semaphores
		parameters:
				 - none
		Returns: TRUE if succeded, ERR o.w
	*/
	int ret_val = TRUE;
	int wait_code = ERR;
	int release_res = ERR;

	end_program_semaphore = NULL;
	end_program_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (end_program_semaphore == NULL) {
		printf("Semaphore creation failed\n");
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		ret_val = ERR;
	}
	return ret_val;
}