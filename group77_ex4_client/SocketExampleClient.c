#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../Shared/hardCodedData.h"
#include "./client_services.h"

// defines --------------------------------------------------------------------->
#define NUM_OF_THREADS 2
#define SEND_THREAD_IDX 0
#define RECV_THREAD_IDX 1
#define CTRL_THREAD_IDX 2

// globals --------------------------------------------------------------------->
Socket_info m_socket_data;
static HANDLE msg_q_semaphore;
static HANDLE exit_semaphore;
static HANDLE hThread[NUM_OF_THREADS];
int send_flag = FALSE;
int recv_flag = FALSE;
int close_send_brutally = TRUE;
int close_recv_brutally = TRUE;


void lowerCase(char *str)
{
	for (int i = 0; str[i] != '\0'; i++)
		str[i] = tolower(str[i]);
}

char *getString(FILE* fp)
{
	//The size is extended by the input with the value of the provisional
	char *str;
	int ch;
	size_t len = 0;
	size_t size = 10;
	str = realloc(NULL, sizeof(char)*size);//size is start size
	if (str == NULL) {
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
		return str;
	}
	while (EOF != (ch = fgetc(fp)) && ch != '\n') {
		str[len++] = ch;
		if (len == size) {
			str = realloc(str, sizeof(char)*(size += 16));
			if (str == NULL) {
				raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
				return str;
			}
		}
	}
	str[len++] = '\0';

	return realloc(str, sizeof(char)*len);
}

void printMenuAndGetAnswer(char *menu, int *answer, int max_menu_option)
{
	char *user_answer;
	BOOL done = FALSE;
	do {
		printf("%s", menu);
		user_answer = getString(stdin);
		if (user_answer == NULL) {
			*answer = ERR;
		}
		if (STRINGS_ARE_EQUAL(user_answer, "1") || STRINGS_ARE_EQUAL(user_answer, "2")) {
			done = TRUE;
			*answer = atoi(user_answer);
		}
		else if (STRINGS_ARE_EQUAL(user_answer, "3") && max_menu_option >= 3) {
			done = TRUE;
			*answer = atoi(user_answer);
		}
		else if (STRINGS_ARE_EQUAL(user_answer, "4") && max_menu_option >= 4) {
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
}

int tryToReconnect(Socket_info *socket_data, int *try_to_reconnect_answer)
{
	// First print the menu and then try to connect, otherwise try first to connect
	if (*try_to_reconnect_answer != 0) {
		printMenuAndGetAnswer(RECONNECT_MENU, try_to_reconnect_answer, 2);
		if (*try_to_reconnect_answer == EXIT_PROGRAM || *try_to_reconnect_answer == ERR) {
			WSACleanup();
			return EXIT_PROGRAM;
		}
	}
	do {
		if (connect(socket_data->socket, (SOCKADDR*)&(socket_data->sock_adrr), sizeof(socket_data->sock_adrr)) == SOCKET_ERROR) {
			printf("Failed to connecting to server on %s:%s.\n", socket_data->ip_addres, socket_data->port_num_char);
			printMenuAndGetAnswer(RECONNECT_MENU, try_to_reconnect_answer, 2);
			if (*try_to_reconnect_answer == EXIT_PROGRAM || *try_to_reconnect_answer == ERR) {
				WSACleanup();
				return EXIT_PROGRAM;
			}
		}
		else
			*try_to_reconnect_answer = CONNECTED;
	} while (*try_to_reconnect_answer == RECONNECT);
	printf("Connected to server on %s:%s\n", socket_data->ip_addres, socket_data->port_num_char);
	return CONNECTED;
}

int clientStateMachine(Messege *msg_in, Messege *msg_out)
{
	int ret_val = TRUE;
	int user_answer = ERR;

	if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_MAIN_MENU))
	{
	MAIN_MENU:
		printMenuAndGetAnswer(SERVER_MAIN_MENU_MSG, &user_answer, 4);
		switch (user_answer)
		{
		case 1:
			initMessege(msg_out, "CLIENT_VERSUS", NULL, NULL, NULL, NULL, NULL);
			break;
		case 2:
			initMessege(msg_out, "CLIENT_CPU", NULL, NULL, NULL, NULL, NULL);
			break;
		case 3:
			initMessege(msg_out, "CLIENT_LEADERBOARD", NULL, NULL, NULL, NULL, NULL);
			break;
		case 4:
			initMessege(msg_out, "CLIENT_DISCONNECT", NULL, NULL, NULL, NULL, NULL);
			break;
		}

		return TRUE;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_APPROVED))
	{
		printf(SERVER_APPROVED_MSG);
		return NO_NEED_TO_REPLY;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_DENIED))
	{
		printf(SERVER_DENIED_MSG_ARGS, msg_in->params[0], msg_in->params[1], msg_in->params[2]);
		printMenuAndGetAnswer(SERVER_DENIED_MSG_MENU, &user_answer, 2);
		switch (user_answer)
		{
		case 1:
			printf("try to reconnect\n");
			return NO_NEED_TO_REPLY;
		case 2:
			printf("Exiting program..\n");
			return NO_NEED_TO_REPLY;
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
			initMessege(msg_out, "CLIENT_PLAYER_MOVE", "ROCK", NULL, NULL, NULL, NULL);
			break;
		case PAPER:
			initMessege(msg_out, "CLIENT_PLAYER_MOVE", "PAPER", NULL, NULL, NULL, NULL);
			break;
		case SCISSORS:
			initMessege(msg_out, "CLIENT_PLAYER_MOVE", "SCISSORS", NULL, NULL, NULL, NULL);
			break;
		case LIZARD:
			initMessege(msg_out, "CLIENT_PLAYER_MOVE", "LIZARD", NULL, NULL, NULL, NULL);
			break;
		case SPOCK:
			initMessege(msg_out, "CLIENT_PLAYER_MOVE", "SPOCK", NULL, NULL, NULL, NULL);
			break;
		}

		return TRUE;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_GAME_RESULTS))
	{
		if (STRINGS_ARE_EQUAL(msg_in->params[3], "DRAW"))
			printf(SERVER_GAME_RESULTS_MSG, msg_in->params[0], msg_in->params[1], msg_in->params[2], msg_in->params[3]);
		else
			printf(SERVER_GAME_RESULTS_DRAW_MSG, msg_in->params[0], msg_in->params[1], msg_in->params[2]);
		return NO_NEED_TO_REPLY;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_GAME_OVER_MENU))
	{
		printMenuAndGetAnswer(SERVER_GAME_OVER_MENU_MSG, &user_answer, 2);

		switch (user_answer)
		{
		case 1:
			initMessege(msg_out, msg_in->params[0], NULL, NULL, NULL, NULL, NULL);
			break;
		case 2:
			goto MAIN_MENU;
			break;
		}

		return TRUE;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_OPPONENT_QUIT))
	{
		printf(SERVER_OPPONENT_QUIT_MSG, msg_in->params[0]);
		goto MAIN_MENU;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_NO_OPPONENTS))
	{
		printf(SERVER_NO_OPPONENTS_MSG);
		goto MAIN_MENU;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_LEADERBOARD))
	{
		printf("Print leader board somehow\n");
		goto MAIN_MENU;
	}

	else
	{
		printf("seems we don't handle this message type: %s", msg_in->type);
		return ERR;
	}

	return TRUE;
}


//==========================================================================
//					Threads
//==========================================================================
void TerminateThreadBrutally(int thread_idx)
{
	extern HANDLE hThread[NUM_OF_THREADS];
	TerminateThread(hThread[thread_idx], 0x555);
}

void TerminateThreadNicelly(int thread_idx)
{
	extern HANDLE hThread[NUM_OF_THREADS];
	LPDWORD exit_code = 0;

	if (GetExitCodeThread(hThread[thread_idx], exit_code) == 0)
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);

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

		// reciving semaphore - only recieve if not sending
		wait_code = WaitForSingleObject(msg_q_semaphore, INFINITE);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE) goto UPDATE_FLAG_AND_EXIT_THREAD;
			

		printf("\n==================\nRECV THREAD START\n==================\n\n");

		// decode messege recieved from server and load it to msg_struct
		ret_val = decodeWrapper(&msg_struct, &m_socket_data.socket);
		if (ret_val == ERR) goto UPDATE_FLAG_AND_EXIT_THREAD;

		// print msg for debugging
		//printMessege(&msg_struct);
		
		// insert to messege Q
		ret_val = msg_q_insert(&msg_struct);
		if (ret_val != TRUE)
		{
			freeMessege(&msg_struct);
			return ERR;
		}

		// free Messege struct. will be allocated again on next loop
		freeMessege(&msg_struct);

		// print msg Q for debugging
		//msg_q_printQ();

		printf("\n==================\nRECV THREAD END\n==================\n\n");

		// done reciving, release semaphore
		release_res = ReleaseSemaphore(msg_q_semaphore, 1, &previous_count);
		if (release_res == FALSE)
		{
			printf("Realese semaphore error!\n");
			raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
			goto UPDATE_FLAG_AND_EXIT_THREAD;
		}	
	}

	return TRUE;

UPDATE_FLAG_AND_EXIT_THREAD:
	recv_flag = TRUE;
	return ERR;
}

//Sending data to the server
static DWORD SendDataThread(void)
{	
	extern int send_flag;
	extern int recv_flag;
	extern int close_send_brutally;
	extern char **g_argv;

	int ret_val = TRUE;
	int wait_code = TRUE;
	int release_res = TRUE;
	LONG previous_count;

	//// Send client request messege to server
	//ret_val = sendMessegeWrapper(m_socket_data.socket, "CLIENT_REQUEST", g_argv[3], NULL, NULL, NULL, NULL);
	//if (ret_val == ERR)
	//{
	//	// free curr_msg
	//	printf("Client state machine error\n");
	//	raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
	//	goto UPDATE_FLAG_AND_EXIT_THREAD;
	//}

	while (TRUE) 
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

		//use semaphore to verify done reciving and start sending process


		printf("\n==================\nSEND THREAD START\n==================\n\n");

		// print messege Q for debugging
		//msg_q_printQ();
		
		wait_code = WaitForSingleObject(msg_q_semaphore, INFINITE);
		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE)
			goto UPDATE_FLAG_AND_EXIT_THREAD;

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
		if (ret_val == TRUE)
		{	
			// for debugging - if we had messege in Q - print it
			//printMessege(&curr_msg);

			// send messege to state machine
			ret_val = clientStateMachine(&curr_msg, &msg_out);
			
			if (ret_val == ERR)
			{
				// free curr_msg
				freeMessege(&curr_msg);
				printf("Details: Client state machine error\n");
				raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
				goto UPDATE_FLAG_AND_EXIT_THREAD;
			}

			// in case we have a messege to send back to server
			else if (ret_val == TRUE)
			{
				// encode meseege and send
				ret_val = sendMessegeWrapper(m_socket_data.socket, msg_out.type, msg_out.params[0], msg_out.params[1], msg_out.params[2], msg_out.params[3], msg_out.params[4]);
				if (ret_val == ERR)
				{
					// free curr_msg
					freeMessege(&curr_msg);
					freeMessege(&msg_out);
					goto UPDATE_FLAG_AND_EXIT_THREAD;
				}

				// free messege out
				freeMessege(&msg_out);
			}

			// free curr_msg
			freeMessege(&curr_msg);
		}

		printf("\n==================\nSEND THREAD END\n==================\n\n");

	}
	
	return TRUE;

UPDATE_FLAG_AND_EXIT_THREAD:
	send_flag = TRUE;
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

	while (TRUE)
	{
		if (send_flag)
		{
			Sleep(15000);
			if (close_recv_brutally)
				TerminateThreadBrutally(RECV_THREAD_IDX);
			else
				TerminateThreadNicelly(RECV_THREAD_IDX);
			TerminateThreadNicelly(SEND_THREAD_IDX);
			return TRUE;
		}	

		if (recv_flag)
		{
			Sleep(15000);
			if (close_send_brutally)
				TerminateThreadBrutally(SEND_THREAD_IDX);
			else
				TerminateThreadNicelly(SEND_THREAD_IDX);
			TerminateThreadNicelly(RECV_THREAD_IDX);
			return TRUE;
		}
		
	}
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
	return ret_val;
}

int checkWaitCodeStatus(DWORD wait_code, BOOL singleNotMultiple) {
	/*
	Description: check wait code status from waitForMultipleObject o rwaitForSingleObject function
	parameters:
			 - DWORD wait_code - wait code recieved
			 - BOOL singleNotMultiple - TRUE for multiple, FALSE for single

	Returns: TRUE if succeded, ERR o.w
	*/

	int retVal1 = ERR;
	DWORD errorMessageID;
	switch (wait_code)
	{
	case WAIT_TIMEOUT:
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
		printf("details: Timeout error when waiting\n");
		break;
	case WAIT_FAILED:
		errorMessageID = GetLastError();
		printf("%d\n", errorMessageID);
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
		printf("details: Timeout error when waiting\n");
		break;
	case WAIT_OBJECT_0:
		retVal1 = TRUE;
		break;
	case WAIT_ABANDONED_0:
		raiseError(6, __FILE__, __func__, __LINE__, ERROR_ID_6_THREADS);
		printf("details: WAIT ANDONED\n");
		break;
	}
	
	return retVal1;
}

//==========================================================================
//					Main
//==========================================================================


void MainClient(char *ip_addres, char *port_num_char)
{
	extern HANDLE hThread[];
	DWORD wait_code;
	LPDWORD exit_code_recv = 0;
	LPDWORD exit_code_send = 0;

	int ret_val = TRUE;
	int try_to_reconnect_answer = 0;
	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	//The WSADATA structure contains information about the Windows Sockets implementation.

	// init messege queue for reciving msgs
	extern msg_fifo *msg_q;
	msg_q_init();

	// Create program semaphores
	createProgramSemaphores();

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
		return;
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

	// create threads
	hThread[SEND_THREAD_IDX] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendDataThread, NULL, 0, NULL);
	hThread[RECV_THREAD_IDX] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvDataThread, NULL, 0, NULL);
	hThread[CTRL_THREAD_IDX] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadCtrl, NULL, 0, NULL);


	wait_code = WaitForSingleObject(hThread[CTRL_THREAD_IDX], INFINITE);
	ret_val = checkWaitCodeStatus(wait_code, FALSE);
	if (ret_val == ERR)
	{
		TerminateThreadBrutally(SEND_THREAD_IDX);
		TerminateThreadBrutally(RECV_THREAD_IDX);
		TerminateThreadBrutally(CTRL_THREAD_IDX);
	}
	else
		TerminateThreadNicelly(CTRL_THREAD_IDX);

MAIN_CLEAN:
	msg_q_freeQ();
	closesocket(m_socket_data.socket);
	WSACleanup();

	return;
}

