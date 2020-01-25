
/*
=================================================================================
							Description
=================================================================================
		Server commiunication threads functions.
		initialize server and open threads for users.
=================================================================================
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Includes ------------------------------------------------------------------>
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

// server ------------->
#include "SocketServer.h"
#include "server_services.h"

// Shared ------------->
#include "../Shared/SocketShared.h"
#include "../Shared/SocketSendRecvTools.h"

// Defines ------------------------------------------------------------------>
#define NUM_OF_WORKER_THREADS 2
#define MAX_USERS 2
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
HANDLE file_mutex;
HANDLE partner_played_semaphore;
User usr_arr[MAX_USERS];

// Local functions declerations ------------------------------------------>
static int FindFirstUnusedThreadSlot();
static void closeProgramNicely();
static DWORD ServiceThread(int *threadIdx);
static DWORD exitProgramThread();
void closeControlersThreadsAndResources(HANDLE *exit_program_handle);
int createServerSemaphores();
int createServerMutexes();
void denyClient(SOCKET socket, char *port_num_char);


// Functions -------------------------------------------------------------->

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void MainServer(char port_num_char[5])
{
	/*
	Description: Recivies port number given as an arg from user, and start a server on hard coded IP and this port num.
					for every user 
	parameters:
			 - char port_num_char[5] - port num given from user
	Returns: void
	*/
	extern SOCKET MainSocket;
	HANDLE exit_program_handle;
	unsigned long Address;
	SOCKADDR_IN service;
	int Ind;
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
	createServerMutexes();

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
	Address = inet_addr( SERVER_ADDRESS_STR );
	if ( Address == INADDR_NONE )
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
				SERVER_ADDRESS_STR );
		goto server_cleanup_2;
	}
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = Address;
    service.sin_port = htons(port_num_int); //The htons function converts a u_short from host to TCP/IP network byte order //( which is big-endian ).
	
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

    
	// init users
	initUser(&usr_arr[0], NULL, STATUS_INIT, 0, FALSE, DONT_KNOW);
	initUser(&usr_arr[1], NULL, STATUS_INIT, 1, FALSE, DONT_KNOW);
	
	printf( "Waiting for a client to connect...\n" );
	
	exit_program_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) exitProgramThread, NULL, 0, NULL);
	if (exit_program_handle == NULL)
	{
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
		printf("details: Error when creating thread\n");
		ret_val = ERR;
		goto server_cleanup_2;
	}

	while (TRUE)
	{
		SOCKET AcceptSocket = accept( MainSocket, NULL, NULL );
		if ( AcceptSocket == INVALID_SOCKET && (force_exit_flag == FALSE)){
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
			denyClient(AcceptSocket, port_num_char);
			shutdown(AcceptSocket, SD_SEND);
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
		} 
		else 	
		{
			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close cceptSocket, instead close ThreadInputs[Ind] when the time comes.
			ThreadHandles[Ind] = CreateThread(NULL,0,( LPTHREAD_START_ROUTINE ) ServiceThread,&( Ind ),0,NULL);
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
	/*
	Description: Find first slot available oin thread handles array 
	Returns: available thread index
	*/

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
	/*
	Description: Tries to close all threads with closeHandle, if cant - closes with Terminate
	Returns: void
	*/


	int Ind; 
	DWORD wait_code;
	int ret_val;
	LPDWORD lpExitCode = 0;
	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
	{
		if ( ThreadHandles[Ind] != NULL && close_brutally[Ind] == FALSE)
		{
			// poll to check if thread finished running:
			wait_code = WaitForSingleObject(ThreadHandles[Ind], INFINITE);
			ret_val = checkWaitCodeStatus(wait_code, TRUE);
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


static DWORD ServiceThread(int *threadIdx ) 
{
	/*
	Description: Service thread is the thread that opens for each successful client connection and "talks" to the client.
	parameters:
			 - char *player_move
			 - int player_idx
			 - int opponent_idx
			 - SOCKET *socket
	Returns: TRUE if succeded, ERR o.w
	*/

	int user_idx_local = *threadIdx;
	extern force_exit_flag;
	extern User usr_arr[MAX_USERS];
	int ret_val = TRUE;
	BOOL Done = FALSE;
	SOCKET *t_socket;
	Player player;
	Messege first_msg;
	
	t_socket = &ThreadInputs[user_idx_local];
	close_brutally[user_idx_local] = TRUE;
	initMessege(&first_msg, NULL, NULL, NULL, NULL, NULL, NULL);
	player.win = 0;
	player.loss = 0;

	// Recv client request from user
	ret_val = decodeWrapper(&first_msg, &ThreadInputs[user_idx_local]);
	if (!STRINGS_ARE_EQUAL(first_msg.type, CLIENT_REQUEST) || ret_val != TRUE) {
		freeMessege(&first_msg);
		goto MAIN_CLEAN;
	}
	else{
		strcpy_s(player.name, USERNAME_MAX_LEN, first_msg.params[0]);
		printf("Player name: %s\n", player.name);
		freeMessege(&first_msg);
	}
	
	// Send messege approval to user
	ret_val = sendMessegeWrapper(*t_socket, SERVER_APPROVED, NULL, NULL, NULL, NULL, NULL);
	if (ret_val!=TRUE) goto MAIN_CLEAN;

	// init user struct
	initUser(&usr_arr[user_idx_local], &player, STATUS_INIT, user_idx_local, TRUE, DONT_KNOW);
	if (ret_val == ERR) goto MAIN_CLEAN;

	while ( !Done ) 
	{		
		Messege msg_struct;
		// send user main menu
		initMessege(&msg_struct, NULL, NULL, NULL, NULL, NULL, NULL);
		ret_val = sendMessegeWrapper(*t_socket, SERVER_MAIN_MENU, NULL, NULL, NULL, NULL, NULL);
		if (ret_val != TRUE || force_exit_flag) {
			freeMessege(&msg_struct);
			goto MAIN_CLEAN;
		}
		// get user choise
		ret_val = decodeWrapper(&msg_struct, t_socket);
		if (ret_val != TRUE) {
			printf("The connection with %s has been lost\n", player.name);
			freeMessege(&msg_struct);
			goto MAIN_CLEAN;
		}

		// state machine - dependes on user's choise
		if (STRINGS_ARE_EQUAL(msg_struct.type, CLIENT_CPU)) {
			usr_arr[user_idx_local].status = STATUS_CLIENT_CPU;
			ret_val = client_vs_cpu(t_socket, &player);
			if (ret_val != TRUE) Done = TRUE;
		}
		if (STRINGS_ARE_EQUAL(msg_struct.type, CLIENT_DISCONNECT)) {
			//send end messege
			usr_arr[user_idx_local].status = STATUS_CLIENT_QUIT;
			Done = TRUE;
		}
		if (STRINGS_ARE_EQUAL(msg_struct.type, CLIENT_VERSUS)) {
			usr_arr[user_idx_local].status = STATUS_CLIENT_VS;
			usr_arr[user_idx_local].play_vs_again = DONT_KNOW;
			ret_val = client_vs_client(t_socket, &usr_arr[user_idx_local]);
			// if ret_val is no partner, update user and let him choose what's next
			if (ret_val == NO_PARTNER)
			{
				ret_val = sendMessegeWrapper(*t_socket, SERVER_NO_OPPONENTS, NULL, NULL, NULL, NULL, NULL);
				if (ret_val != TRUE) goto MAIN_CLEAN;
				usr_arr[user_idx_local].status = STATUS_INIT;
			}

			// if ret_val is err
			if (ret_val == ERR) Done = TRUE;
		}
		freeMessege(&msg_struct);
	}
	
MAIN_CLEAN:
	// user is done, restart his user struct back to offline mode
	initUser(&usr_arr[user_idx_local], NULL, STATUS_INIT, ERR ,FALSE, DONT_KNOW);
	close_brutally[*threadIdx] = FALSE;
	shutdown(*t_socket, SD_SEND);
	return 0;
}

static DWORD exitProgramThread()
{
	/*
	Description: exitProgramThread is listening to stdin - if "exit" recognized it starts closing server process.
	Returns: TRUE if succeded
	*/

	extern SOCKET MainSocket;
	extern int main_socket_is_closed;
	extern int force_exit_flag;
	extern HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
	extern SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
	int exit_code = TRUE;

	// listen to stdin
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
			closesocket(MainSocket);
			main_socket_is_closed = TRUE;
			free(exit_str);
			break;
		}
		free(exit_str);
	}
	// "exit" recognized, start closing program
	printf("+++++++++++++++++++++++++++++++++++++++\n");
	printf("Closing All Resources And Exiting...\n");
	printf("Waiting For All Resources To Close...\n");
	// send shutdown to users
	if (ThreadInputs[0] != INVALID_SOCKET) shutdown(ThreadInputs[0], SD_SEND);
	if (ThreadInputs[1] != INVALID_SOCKET) shutdown(ThreadInputs[1], SD_SEND);
	Sleep(WAITING_TIME_MILLI_SEC);

	//close if main socket didn't finish, close brutally
	if (close_brutally_main) {
		main_socket_is_closed = TRUE;
		printf("Done waiting to main socket, Closing bruttaly...\n");
		if (closesocket(MainSocket) != 0) {
			raiseError(10, __FILE__, __func__, __LINE__, ERROR_ID_10_SOCKET);
		}
	}

	// close threads
	for (int idx = 0; idx < NUM_OF_WORKER_THREADS; idx++) {
		if (close_brutally[idx]) {
			printf("Done waiting to socket #%d, Closing bruttaly...\n",idx);
			if (ThreadHandles[idx] != NULL) {
				if (TerminateThread(ThreadHandles[idx], THREAD_ERR) == 0)
					raiseError(10, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
			}
			if (ThreadInputs[idx] != INVALID_SOCKET) {
				if (closesocket(ThreadInputs[idx]) != 0) {
					raiseError(10, __FILE__, __func__, __LINE__, ERROR_ID_10_SOCKET);
				}
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
	/*
	Description: close controll threads and resources allocated
			 - HANDLE *exit_program_handle
	Returns: void
	*/
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

	partner_played_semaphore = NULL;
	partner_played_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (partner_played_semaphore == NULL) {
		printf("Semaphore creation failed\n");
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		ret_val = ERR;
	}

	return ret_val;
}

int createServerMutexes()
{
	/*
	Description: init global mutexes
	parameters:
			 - none
	Returns: TRUE if succeded, ERR o.w
	*/
	extern HANDLE file_mutex;
	int retVal = TRUE;

	// initialization
	file_mutex = NULL;

	// Create mutexes
	file_mutex = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL); /* un-named */
	if (file_mutex == NULL) {
		retVal = ERR;  goto Main_cleanup;
	}

Main_cleanup:
	if (retVal == ERR) 
	{
		raiseError(6, __FILE__, __func__, __LINE__, "Mutex creation failed\n");
	}
	return retVal;
}

void denyClient(SOCKET socket, char *port_num_char)
{
	/*
	Description: If server doesn't have avaialble slots, send sever denied msg to user
	parameters:
			 - SOCKET socket
			 - char *port_num_char
	Returns: void
	*/

	Messege req_msg;
	int ret_val = TRUE;
	initMessege(&req_msg, NULL, NULL, NULL, NULL, NULL, NULL);

	ret_val = decodeWrapper(&req_msg, &socket);
	if (ret_val != TRUE) return;
	ret_val = sendMessegeWrapper(socket, SERVER_DENIED, "No slots available, Server is dropping the connection...",
		SERVER_ADDRESS_STR, port_num_char, NULL, NULL);
	if (ret_val != TRUE) return;
	printf("No slots available for client, dropping the connection.\n");
	freeMessege(&req_msg);
}
