#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../Shared/hardCodedData.h"
#include "./client_services.h"
#include "../Shared/SocketSendRecvTools.h"

// defines --------------------------------------------------------------------->
#define NUM_OF_THREADS 2
#define SEND_THREAD_IDX 0
#define RECV_THREAD_IDX 1
#define CTRL_THREAD_IDX 2

// globals --------------------------------------------------------------------->
Socket_info m_socket_data;
static HANDLE msg_q_semaphore;
static HANDLE terminate_semaphore;
static HANDLE hThread[NUM_OF_THREADS];
int send_flag = FALSE;
int recv_flag = FALSE;
int close_send_brutally = TRUE;
int close_recv_brutally = TRUE;
int client_want_to_exit = FALSE;
int connection_failure = FALSE;
int connection_failure_msg = TRUE;
int program_closed_nicely = TRUE;
BOOL waiting_for_user_answer = FALSE;
static HANDLE send_trigger_semaphore = NULL;
static HANDLE send_timeout_semaphore = NULL;
static HANDLE recv_trigger_semaphore = NULL;
static HANDLE recv_timeout_semaphore = NULL;
int done_flag = FALSE;


void printMenuAndGetAnswer(char *menu, int *answer, int max_menu_option)
{
	char *user_answer;
	DWORD wait_code;
	BOOL release_res;
	BOOL done = FALSE;

	do {
		if (recv_flag) return;

		printf("%s", menu);

		wait_code = WaitForSingleObject(terminate_semaphore, INFINITE);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) goto RELEASE_SEMAPHORE;
		waiting_for_user_answer = TRUE;
		user_answer = getString(stdin);
		waiting_for_user_answer = FALSE;
		release_res = ReleaseSemaphore(terminate_semaphore, 1, NULL);
		if (release_res != TRUE)
		{
			printf("Error in releasing semaphore!\n");
			raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
			return;
		}
		
		if (user_answer == NULL) {
			*answer = ERR;
		}
		if ((STRINGS_ARE_EQUAL(user_answer, "1") || STRINGS_ARE_EQUAL(user_answer, "2"))&& max_menu_option != PLAYER_MOVE) {
			done = TRUE;
			*answer = atoi(user_answer);
		}
		else if ((STRINGS_ARE_EQUAL(user_answer, "3") && max_menu_option >= 3) && max_menu_option != PLAYER_MOVE) {
			done = TRUE;
			*answer = atoi(user_answer);
		}
		else if ((STRINGS_ARE_EQUAL(user_answer, "4") && max_menu_option >= 4) && max_menu_option != PLAYER_MOVE) {
			done = TRUE;
			*answer = atoi(user_answer);
		}
		else if (max_menu_option == PLAYER_MOVE)
		{
			lowerCase(user_answer);
			if STRINGS_ARE_EQUAL(user_answer, "rock")
			{
				done = TRUE;
				*answer = ROCK;
			}
			else if (STRINGS_ARE_EQUAL(user_answer, "paper"))
			{
				done = TRUE;
				*answer = PAPER;
			}
			else if (STRINGS_ARE_EQUAL(user_answer, "scissors"))
			{
				done = TRUE;
				*answer = SCISSORS;
			}
			else if (STRINGS_ARE_EQUAL(user_answer, "lizard"))
			{
				done = TRUE;
				*answer = LIZARD;
			}
			else if (STRINGS_ARE_EQUAL(user_answer, "spock"))
			{
				done = TRUE;
				*answer = SPOCK;
			}

		}
		free(user_answer);
	} while (!done);

RELEASE_SEMAPHORE:
	if (!done)
		release_res = ReleaseSemaphore(terminate_semaphore, 1, NULL);
		if (release_res != TRUE)
		{
			printf("Error in releasing semaphore!\n");
			raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
			return;
		}
}

int tryToReconnect(Socket_info *socket_data, int *try_to_reconnect_answer)
{
	// First print the menu and then try to connect, otherwise try first to connect
	if (*try_to_reconnect_answer != 0) 
	{
		printMenuAndGetAnswer(RECONNECT_MENU, try_to_reconnect_answer, 2);
		//*try_to_reconnect_answer = RECONNECT;
		if (*try_to_reconnect_answer == EXIT_PROGRAM || *try_to_reconnect_answer == ERR) {
			return EXIT_PROGRAM;
		}
	}
	do {
		if (connect(socket_data->socket, (SOCKADDR*)&(socket_data->sock_adrr), sizeof(socket_data->sock_adrr)) == SOCKET_ERROR) {
			printf("Failed to connecting to server on %s:%s.\n", socket_data->ip_addres, socket_data->port_num_char);
			printMenuAndGetAnswer(RECONNECT_MENU, try_to_reconnect_answer, 2);
			//*try_to_reconnect_answer = RECONNECT;
			if (*try_to_reconnect_answer == EXIT_PROGRAM || *try_to_reconnect_answer == ERR) {
				return EXIT_PROGRAM;
			}
		}
		else
			*try_to_reconnect_answer = CONNECTED;
	} while (*try_to_reconnect_answer == RECONNECT);
	return CONNECTED;
}

int clientStateMachine(Messege *msg_in, Messege *msg_out)
{
	int ret_val = TRUE;
	int user_answer = ERR;

	if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_MAIN_MENU))
	{
		printMenuAndGetAnswer(SERVER_MAIN_MENU_MSG, &user_answer, 4);
		switch (user_answer)
		{
		case 1:
			initMessege(msg_out, CLIENT_VERSUS, NULL, NULL, NULL, NULL, NULL);
			break;
		case 2:
			initMessege(msg_out, CLIENT_CPU, NULL, NULL, NULL, NULL, NULL);
			break;
		case 3:
			initMessege(msg_out, CLIENT_LEADERBOARD, NULL, NULL, NULL, NULL, NULL);
			break;
		case 4:
			initMessege(msg_out, CLIENT_DISCONNECT, NULL, NULL, NULL, NULL, NULL);
			client_want_to_exit = TRUE;
			break;
		}

		return TRUE;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_INVITE))
	{
		printf(SERVER_INVITE_MSG, msg_in->params[0]);
		return NO_NEED_TO_REPLY;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_PLAYER_MOVE_REQUEST))
	{
		printMenuAndGetAnswer(SERVER_PLAYER_MOVE_REQUEST_MSG, &user_answer, PLAYER_MOVE);

		switch (user_answer)
		{
		case ROCK:
			initMessege(msg_out, CLIENT_PLAYER_MOVE, "ROCK", NULL, NULL, NULL, NULL);
			break;
		case PAPER:
			initMessege(msg_out, CLIENT_PLAYER_MOVE, "PAPER", NULL, NULL, NULL, NULL);
			break;
		case SCISSORS:
			initMessege(msg_out, CLIENT_PLAYER_MOVE, "SCISSORS", NULL, NULL, NULL, NULL);
			break;
		case LIZARD:
			initMessege(msg_out, CLIENT_PLAYER_MOVE, "LIZARD", NULL, NULL, NULL, NULL);
			break;
		case SPOCK:
			initMessege(msg_out, CLIENT_PLAYER_MOVE, "SPOCK", NULL, NULL, NULL, NULL);
			break;
		}

		return TRUE;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_GAME_RESULTS))
	{
		if (STRINGS_ARE_EQUAL(msg_in->params[3], "DRAW"))
			printf(SERVER_GAME_RESULTS_DRAW_MSG, msg_in->params[0], msg_in->params[1], msg_in->params[2]);
		else
			printf(SERVER_GAME_RESULTS_MSG, msg_in->params[0], msg_in->params[1], msg_in->params[2], msg_in->params[3]);

		return NO_NEED_TO_REPLY;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_GAME_OVER_MENU))
	{
		printMenuAndGetAnswer(SERVER_GAME_OVER_MENU_MSG, &user_answer, 2);

		switch (user_answer)
		{
		case 1:
			initMessege(msg_out, CLIENT_REPLAY, NULL, NULL, NULL, NULL, NULL);
			break;
		case 2:
			initMessege(msg_out, CLIENT_MAIN_MENU, NULL, NULL, NULL, NULL, NULL);
			break;
		}

		return TRUE;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_OPPONENT_QUIT))
	{
		printf(SERVER_OPPONENT_QUIT_MSG, msg_in->params[0]);
		return NO_NEED_TO_REPLY;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_NO_OPPONENTS))
	{
		printf(SERVER_NO_OPPONENTS_MSG);
		return NO_NEED_TO_REPLY;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_LEADERBOARD))
	{
		printf("Print leader board somehow\n");
		return NO_NEED_TO_REPLY;
	}

	else
	{
		printf("seems we don't handle this message type: %s\n", msg_in->type);
		return ERR;
	}

	return TRUE;
}

int checkServerAnswer(Messege *msg)
{
	int ret_val = CONNECTED;
	if (STRINGS_ARE_EQUAL(msg->type, SERVER_APPROVED))
	{
		printf(SERVER_APPROVED_MSG);
		ret_val = CONNECTED;
	}
	else if (STRINGS_ARE_EQUAL(msg->type, SERVER_DENIED)) {
		printf(SERVER_DENIED_MSG_ARGS, msg->params[0], msg->params[1], msg->params[2]);
		connection_failure = TRUE;
		connection_failure_msg = FALSE;
		ret_val = EXIT_PROGRAM;
	}
	else
	{
		printf("The Server sent illeagal messege type: %s\n", msg->type);
		connection_failure = TRUE;
		connection_failure_msg = FALSE;
		ret_val = ERR;
	}
	freeMessege(msg);
	return ret_val;

}
//==========================================================================
//					Threads
//==========================================================================
void TerminateThreadBrutally(int thread_idx)
{
	extern HANDLE hThread[NUM_OF_THREADS];
	TerminateThread(hThread[thread_idx], 0x555);
	fflush(stdin);
	if (CloseHandle(hThread[thread_idx]) == FALSE)
	{
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
		printf("Details: Error when closing thread\n");
	}
}

void TerminateThreadNicelly(int thread_idx)
{
	extern HANDLE hThread[NUM_OF_THREADS];
	LPDWORD exit_code = 0;

	//if (GetExitCodeThread(hThread[thread_idx], exit_code) == 0)
	//	raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);

	if (CloseHandle(hThread[thread_idx]) == FALSE)
	{
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
		printf("Details: Error when closing thread\n");
	}
}

//Reading data coming from the server
static DWORD RecvDataThread(void)
{
	extern msg_fifo *msg_q;
	extern int send_flag;
	extern int recv_flag;
	extern int close_send_brutally;

	int ret_val = TRUE;
	int recv_ret_val = TRUE;
	int wait_code = TRUE;
	int release_res = TRUE;
	int release_code = TRUE;

	LONG previous_count;
	
	while (TRUE)
	{
		if (send_flag)
		{
			close_recv_brutally = FALSE;
			return TRUE;
		}

		// init msg struct to load msg recieved from server
		Messege msg_struct;
		ret_val = initMessege(&msg_struct, NULL, NULL, NULL, NULL, NULL, NULL);
		if (ret_val == ERR) goto UPDATE_FLAG_AND_EXIT_THREAD;

		// decode messege recieved from server and load it to msg_struct
		recv_ret_val = decodeWrapper(&msg_struct, &m_socket_data.socket);
	
		if (recv_ret_val != TRUE) goto UPDATE_FLAG_AND_EXIT_THREAD;
		
		// reciving semaphore - only recieve if not sending
		wait_code = WaitForSingleObject(msg_q_semaphore, INFINITE);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) goto EXIT_AND_RLS_SMPHR;

		// insert to messege Q
		ret_val = msg_q_insert(&msg_struct);

		// done reciving, release semaphore
		release_res = ReleaseSemaphore(msg_q_semaphore, 1, &previous_count);
		if (release_res == FALSE)
		{
			printf("Realese semaphore error!\n");
			raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
			goto UPDATE_FLAG_AND_EXIT_THREAD;
		}

		if (ret_val != TRUE)
		{
			freeMessege(&msg_struct);
			goto UPDATE_FLAG_AND_EXIT_THREAD;
		}


		// free Messege struct. will be allocated again on next loop
		freeMessege(&msg_struct);

	}

	return TRUE;

UPDATE_FLAG_AND_EXIT_THREAD:
	close_recv_brutally = FALSE;
	if ((recv_ret_val != TRUE) && (client_want_to_exit != TRUE)) {
		connection_failure = TRUE;
	}
	else {
		if (!client_want_to_exit)
			printf("Fatal error occured, aborting...\n"); 
	}
	if (waiting_for_user_answer) printf("Please press any key to continue...\n");
	recv_flag = TRUE;
	return ERR;

EXIT_AND_RLS_SMPHR:
	release_res = ReleaseSemaphore(msg_q_semaphore, 1, &previous_count);
	if (release_res == FALSE)
	{
		printf("Realese semaphore error!\n");
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		goto UPDATE_FLAG_AND_EXIT_THREAD;
	}
	printf("Fatal error occured, aborting...\n");
	recv_flag = TRUE;
	return ERR;

}

//Sending data to the server
static DWORD SendDataThread(void)
{	
	extern int send_flag;
	extern int recv_flag;
	extern int close_send_brutally;
	BOOL done = FALSE;
	int ret_val = TRUE;
	int send_ret_val = TRUE;
	int wait_code = TRUE;
	int release_res = TRUE;
	LONG previous_count;

	while (!done)
	{
		// if recv send closed, stop with this thread as well.
		if (recv_flag)
		{
			close_send_brutally = FALSE;
			return TRUE;
		}

		Messege msg_out;
		Messege curr_msg;

		// init msg out and curr msg for handling
		ret_val = initMessege(&msg_out, NULL, NULL, NULL, NULL, NULL, NULL);
		if (ret_val == ERR) goto UPDATE_FLAG_AND_EXIT_THREAD;

		ret_val = initMessege(&curr_msg, NULL, NULL, NULL, NULL, NULL, NULL);
		if (ret_val == ERR) goto UPDATE_FLAG_AND_EXIT_THREAD;
		
		wait_code = WaitForSingleObject(msg_q_semaphore, INFINITE);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE)
			goto EXIT_AND_RLS_SMPHR;

		// pop first messege in Q
		ret_val = msg_q_pop(&curr_msg);

		release_res = ReleaseSemaphore(msg_q_semaphore, 1, &previous_count);
		if (release_res != TRUE)
		{
			printf("Error in releasing semaphore!\n");
			raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
			if (ret_val == TRUE) freeMessege(&curr_msg);
			goto UPDATE_FLAG_AND_EXIT_THREAD;
			return ERR;
		}

		// if we have msg in Q
		if ((ret_val == TRUE)&&(recv_flag!=TRUE))
		{	

			// send messege to state machine
			ret_val = clientStateMachine(&curr_msg, &msg_out);

			if (ret_val == ERR)
			{
				// free curr_msg
				freeMessege(&curr_msg);
				printf("Details: Client state machine error\n");
				raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
				goto EXIT_AND_RLS_SMPHR;
			}

			// in case we have a messege to send back to server
			else if (ret_val == TRUE)
			{
				if (recv_flag)
				{
					close_send_brutally = FALSE;
					freeMessege(&msg_out);
					freeMessege(&curr_msg);
					return TRUE;
				}

				// encode meseege and send
				send_ret_val = sendMessegeWrapper(m_socket_data.socket, msg_out.type, msg_out.params[0], \
					msg_out.params[1], msg_out.params[2], msg_out.params[3], msg_out.params[4]);

				if (send_ret_val != TRUE)
				{
					// free curr_msg
					freeMessege(&curr_msg);
					freeMessege(&msg_out);
					goto UPDATE_FLAG_AND_EXIT_THREAD;
				}

				// if clients wants to quit, close send msg loop and start exit process
				if (STRINGS_ARE_EQUAL(msg_out.type, CLIENT_DISCONNECT)) 
				{
					done = TRUE;
					client_want_to_exit = TRUE;
				}

				// free messege out
				freeMessege(&msg_out);
			}

			// free curr_msg
			freeMessege(&curr_msg);
		}
	}

	send_flag = TRUE;
	shutdown(m_socket_data.socket, SD_SEND);
	return TRUE;


UPDATE_FLAG_AND_EXIT_THREAD:
	if (send_ret_val != TRUE) connection_failure = TRUE;
	else {
		printf("Fatal error occured, aborting...\n");
	}
	shutdown(m_socket_data.socket, SD_SEND);
	send_flag = TRUE;
	return ERR;

EXIT_AND_RLS_SMPHR:
	release_res = ReleaseSemaphore(msg_q_semaphore, 1, &previous_count);
	if (release_res == FALSE)
	{
		printf("Realese semaphore error!\n");
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		goto UPDATE_FLAG_AND_EXIT_THREAD;
	}
	printf("Fatal error occured, aborting...\n");
	send_flag = TRUE;
	shutdown(m_socket_data.socket, SD_SEND);
	return ERR;

}

// Thread controller - verify both thread are up and runnning
static DWORD ThreadCtrl(void)
{
	extern int recv_flag;
	extern int send_flag;
	extern int close_send_brutally;
	extern int close_recv_brutally;
	extern HANDLE hThread[NUM_OF_THREADS];
	int ret_val = TRUE;
	int local_wait = WAITING_TIME_MILLI_SEC;
	DWORD wait_code;
	BOOL release_res;

	while (TRUE)
	{

		if (send_flag)
		{
			if (client_want_to_exit) local_wait = 100;
			Sleep(local_wait);
			if (close_recv_brutally)
			{
				TerminateThreadBrutally(RECV_THREAD_IDX);
			}
			else
			{
				TerminateThreadNicelly(RECV_THREAD_IDX);
			}
			TerminateThreadNicelly(SEND_THREAD_IDX);
			return TRUE;
		}	

		if (recv_flag && !send_flag)
		{	
			wait_code = WaitForSingleObject(terminate_semaphore, INFINITE);
			if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) goto RELEASE_SEMAPHORE_CTRL;
			shutdown(m_socket_data.socket, SD_SEND);
			Sleep(local_wait);
			if (close_send_brutally)
				TerminateThreadBrutally(SEND_THREAD_IDX);
			else
				TerminateThreadNicelly(SEND_THREAD_IDX);
			TerminateThreadNicelly(RECV_THREAD_IDX);

RELEASE_SEMAPHORE_CTRL:
			release_res = ReleaseSemaphore(terminate_semaphore, 1, NULL);
			if (release_res != TRUE)
			{
				printf("Error in releasing semaphore!\n");
				raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
				return ERR;
			}
			return TRUE;
		}
	}
	return TRUE;
}

//==========================================================================
//					Semaphores
//==========================================================================
int createProgramSemaphores()
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

	msg_q_semaphore = NULL;
	msg_q_semaphore = CreateSemaphore(NULL, 1, 1, NULL);
	if (msg_q_semaphore == NULL) {
		printf("Semaphore creation failed\n");
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		ret_val = ERR;
	}

	terminate_semaphore = NULL;
	terminate_semaphore = CreateSemaphore(NULL, 1, 1, NULL);
	if (terminate_semaphore == NULL) {
		printf("Semaphore creation failed\n");
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		ret_val = ERR;
	}

	return ret_val;
}

//==========================================================================
//					Main
//==========================================================================

int MainClient(char *ip_addres, char *port_num_char, char *user_name, int try_to_reconnect_answer)
{
	extern int close_send_brutally;
	extern int close_recv_brutally;
	extern int send_flag;
	extern int recv_flag;
	Messege approval_msg;
	DWORD wait_code;
	LPDWORD exit_code_recv = 0;
	LPDWORD exit_code_send = 0;

	int ret_val = TRUE;

	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	//The WSADATA structure contains information about the Windows Sockets implementation.

	// Create program semaphores
	ret_val = createProgramSemaphores();
	if (ret_val != TRUE) return ERR;
	
	// init messege queue for reciving msgs
	extern msg_fifo *msg_q;
	msg_q_init();
	initMessege(&approval_msg, NULL, NULL, NULL, NULL, NULL, NULL);
	//Call WSAStartup and check for errors.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");

	strcpy_s(m_socket_data.ip_addres, IP_MAX_LEN, ip_addres);
	strcpy_s(m_socket_data.port_num_char, PORT_MAX_LEN, port_num_char);
	m_socket_data.server_not_client = FALSE;

	//Call the socket function and return its value to the m_socket variable. 
	// For this application, use the Internet address family, streaming sockets, and the TCP/IP protocol.

	// Create a socket.
	m_socket_data.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket_data.socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return ERR;
	}

	//For a client to communicate on a network, it must connect to a server.
	// Connect to a server.

	//Create a sockaddr_in object clientService and set  values.
	m_socket_data.sock_adrr.sin_family = AF_INET;
	m_socket_data.sock_adrr.sin_addr.s_addr = inet_addr(ip_addres); //Setting the IP address to connect to
	m_socket_data.sock_adrr.sin_port = htons(atoi(port_num_char)); //Setting the port to connect to.

	/*
		AF_INET is the Internet address family.
	*/


	// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	
	tryToReconnect(&m_socket_data, &try_to_reconnect_answer);
	if (try_to_reconnect_answer != CONNECTED) {
		goto MAIN_CLEAN;
	}

	// Send client request messege to server
	ret_val = sendMessegeWrapper(m_socket_data.socket, "CLIENT_REQUEST", user_name, NULL, NULL, NULL, NULL);
	if (ret_val != TRUE)
	{
		printf("Client request messege didn't sent due to connection error\n");
		connection_failure = TRUE;
		goto MAIN_CLEAN;
	}
	ret_val = decodeWrapper(&approval_msg, &m_socket_data.socket);
	if (ret_val != TRUE)
	{
		connection_failure = TRUE;
		goto MAIN_CLEAN;
	}

	if (checkServerAnswer(&approval_msg) != CONNECTED) goto MAIN_CLEAN;

	printf("Connected to server on %s:%s\n", m_socket_data.ip_addres, m_socket_data.port_num_char);
	// create threads
	hThread[SEND_THREAD_IDX] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendDataThread, NULL, 0, NULL);
	hThread[RECV_THREAD_IDX] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvDataThread, NULL, 0, NULL);
	hThread[CTRL_THREAD_IDX] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadCtrl, NULL, 0, NULL);

	if ((hThread[SEND_THREAD_IDX] == NULL)||(hThread[RECV_THREAD_IDX] == NULL)||(hThread[CTRL_THREAD_IDX] == NULL))
	{
		printf("thread creation failed. exiting \n");
	}

	wait_code = WaitForSingleObject(hThread[CTRL_THREAD_IDX], INFINITE);
	ret_val = checkWaitCodeStatus(wait_code, FALSE);


	if (ret_val == ERR)
	{
		printf("error in creating threads\n");
		TerminateThreadBrutally(SEND_THREAD_IDX);
		TerminateThreadBrutally(RECV_THREAD_IDX);
		TerminateThreadBrutally(CTRL_THREAD_IDX);
	}
	else
	{
		TerminateThreadNicelly(CTRL_THREAD_IDX);
	}

MAIN_CLEAN:
	msg_q_freeQ();
	closesocket(m_socket_data.socket);
	WSACleanup();
	closeHandles(&msg_q_semaphore, 1);

	// restart globals
	send_flag = FALSE;
	recv_flag = FALSE;
	close_send_brutally = TRUE;
	close_recv_brutally = TRUE;
	client_want_to_exit = FALSE;

	if (connection_failure == TRUE)
	{
		printf("Connection to server on %s:%s has been lost.\n", m_socket_data.ip_addres, m_socket_data.port_num_char);
		connection_failure = FALSE;
		return RECONNECT;
	}

	return EXIT_PROGRAM;
}

//==========================================================================
//					Timeout thread
//==========================================================================

static DWORD sendTimeOutThread()
{
	int done = FALSE;
	while (!done)
	{
		DWORD wait_code;
		wait_code = WaitForSingleObject(send_trigger_semaphore, INFINITE);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) {
			send_flag = TRUE;
			recv_flag = TRUE;
			shutdown(m_socket_data.socket, SD_BOTH);
			break;
		}
		wait_code = WaitForSingleObject(send_timeout_semaphore, WAITING_TIME_MILLI_SEC*10);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) {
			send_flag = TRUE;
			recv_flag = TRUE;
			shutdown(m_socket_data.socket, SD_BOTH);
			break;
		}
		if (done_flag) break;
	}
	return TRUE;
}

static DWORD recvTimeOutThread()
{
	int done = FALSE;
	while (!done)
	{
		DWORD wait_code;
		wait_code = WaitForSingleObject(recv_trigger_semaphore, INFINITE);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) {
			send_flag = TRUE;
			recv_flag = TRUE;
			shutdown(m_socket_data.socket, SD_BOTH);
			break;
		}
		wait_code = WaitForSingleObject(recv_timeout_semaphore, WAITING_TIME_MILLI_SEC * 10);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) {
			send_flag = TRUE;
			recv_flag = TRUE;
			shutdown(m_socket_data.socket, SD_BOTH);
			break;
		}
		if (done_flag) break;
	}
	return TRUE;
}


// add in begging of main;
BOOL createTimeHandles(HANDLE *send_time_handle, HANDLE *recv_time_handle)
{
	*send_time_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)sendTimeOutThread, NULL, 0, NULL);
	if (*send_time_handle == NULL)
		return FALSE;
	*recv_time_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recvTimeOutThread, NULL, 0, NULL);
	if (*recv_time_handle == NULL)
		return FALSE;
	return TRUE;
}

// add in begining of main, dont forget to release
int createTimeSemaphores()
{
	/*
		Description: init global semaphores
		parameters:
				 - none
		Returns: TRUE if succeded, ERR o.w
	*/
	int ret_val = TRUE;

	send_timeout_semaphore = NULL;
	send_timeout_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (send_timeout_semaphore == NULL) {
		printf("Semaphore creation failed\n");
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		ret_val = ERR;
	}

	send_trigger_semaphore = NULL;
	send_trigger_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (send_trigger_semaphore == NULL) {
		printf("Semaphore creation failed\n");
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		ret_val = ERR;
	}
	recv_trigger_semaphore = NULL;
	recv_trigger_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (recv_trigger_semaphore == NULL) {
		printf("Semaphore creation failed\n");
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		ret_val = ERR;
	}

	recv_timeout_semaphore = NULL;
	recv_timeout_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (recv_timeout_semaphore == NULL) {
		printf("Semaphore creation failed\n");
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		ret_val = ERR;
	}

	return ret_val;
} 

// add in end of main
// consider splitting to seperate functions by type (recv, send)
void closeTimeThreadAndSemaphores(HANDLE *send_time_handle, HANDLE *recv_time_handle)
{
	BOOL release_res = TRUE;

	Sleep(WAITING_TIME_MILLI_SEC);
	done_flag = TRUE;
	release_res = ReleaseSemaphore(send_trigger_semaphore, 1, NULL);
	release_res = ReleaseSemaphore(send_timeout_semaphore, 1, NULL) & release_res;
	release_res = ReleaseSemaphore(recv_trigger_semaphore, 1, NULL) & release_res;
	release_res = ReleaseSemaphore(recv_timeout_semaphore, 1, NULL) & release_res;
	if (release_res != TRUE)
	{
		printf("Error in releasing semaphore!\n");
	}



	if (CloseHandle(*send_time_handle) == FALSE || CloseHandle(*recv_time_handle) == FALSE)
	{
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
		printf("Details: Error when closing thread\n");
	}
	if (CloseHandle(send_trigger_semaphore) == FALSE || CloseHandle(send_timeout_semaphore) == FALSE
		|| CloseHandle(recv_trigger_semaphore) == FALSE || CloseHandle(recv_timeout_semaphore) == FALSE)
	{
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
		printf("Details: Error when closing thread\n");
	}

	printf("Program released all resorces Nicely\n");
}

void test_func()
{

	HANDLE send_time_handle;
	HANDLE recv_time_handle;
	int release_res = TRUE;

	// init like this in main
	createTimeSemaphores();
	if (!createTimeHandles(&send_time_handle, &recv_time_handle)) return;



	// wrap sendWrapper with this 
	release_res = ReleaseSemaphore(send_trigger_semaphore, 1, NULL);
	if (release_res != TRUE)
	{
		printf("Error in releasing semaphore!\n");
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		return;
	}
	printf("Program gets in to a blocking function\n");
	//sendwraper
	Sleep(5000);
	release_res = ReleaseSemaphore(send_timeout_semaphore, 1, NULL);


	printf("Program gets out from a blocking function\n");
	if (release_res != TRUE)
	{
		printf("Error in releasing semaphore!\n");
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		return;
	}



	// wrap decoderMsg with this 
	release_res = ReleaseSemaphore(recv_trigger_semaphore, 1, NULL);
	if (release_res != TRUE)
	{
		printf("Error in releasing semaphore!\n");
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		return;
	}
	printf("Program gets in to a blocking function\n");
	// decode function
	Sleep(5000);
	release_res = ReleaseSemaphore(recv_timeout_semaphore, 1, NULL);


	printf("Program gets out from a blocking function\n");
	if (release_res != TRUE)
	{
		printf("Error in releasing semaphore!\n");
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		return;
	}


	closeTimeThreadAndSemaphores(&send_time_handle, &recv_time_handle);
	printf("YES\n");
}