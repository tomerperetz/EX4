#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../Shared/hardCodedData.h"
#include "./client_services.h"
Socket_info m_socket_data;
static HANDLE msg_q_semaphore;
static HANDLE msg_q_mutex;

int clientStateMachine(Messege *msg_in, Messege *msg_out)
{
	int ret_val = TRUE;
	int user_answer = ERR;

	if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_MAIN_MANU))
	{
	MAIN_MANU:
		printMenuAndGetAnswer(SERVER_MAIN_MANU_MSG, &user_answer, 4);
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
		printf(SERVER_APPROVED_MSG, msg_in->params[0], msg_in->params[1]);
		return NO_NEED_TO_REPLY;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_DENIED))
	{
		printf(SERVER_DENIED_MSG_ARGS, msg_in->params[0], msg_in->params[1], msg_in->params[2]);
		printMenuAndGetAnswer(SERVER_DENIED_MSG_MANU, &user_answer, 2);
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
		if (msg_in->params[3] != NULL)
			printf(SERVER_GAME_RESULTS_MSG, msg_in->params[0], msg_in->params[1], msg_in->params[2], msg_in->params[3]);
		else
			printf(SERVER_GAME_RESULTS_DRAW_MSG, msg_in->params[0], msg_in->params[1], msg_in->params[2]);
		return NO_NEED_TO_REPLY;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_GAME_OVER_MANU))
	{
		printMenuAndGetAnswer(SERVER_GAME_OVER_MANU_MSG, &user_answer, 2);

		switch (user_answer)
		{
		case 1:
			initMessege(msg_out, msg_in->params[0], NULL, NULL, NULL, NULL, NULL);
			break;
		case 2:
			goto MAIN_MANU;
			break;
		}

		return TRUE;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_OPPONENT_QUIT))
	{
		printf(SERVER_OPPONENT_QUIT_MSG, msg_in->params[0]);
		goto MAIN_MANU;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_NO_OPPONENTS))
	{
		printf(SERVER_NO_OPPONENTS_MSG);
		goto MAIN_MANU;
	}

	else if (STRINGS_ARE_EQUAL(msg_in->type, SERVER_LEADERBOARD))
	{
		printf("Print leader board somehow\n");
		goto MAIN_MANU;
	}

	else
	{
		printf("seems we don't handle this message type: %s", msg_in->type);
		return ERR;
	}


	return TRUE;
}

//Reading data coming from the server
static DWORD RecvDataThread(void)
{
	extern msg_fifo *msg_q;
	int ret_val = TRUE;
	int wait_code = TRUE;
	int release_res = TRUE;
	int release_code = TRUE;
	LONG previous_count;
	while (1)
	{
		Messege msg_struct;

		ret_val = decodeWrapper(&msg_struct, &m_socket_data.socket);
		
		if (ret_val == ERR) {
			/*TO DO*/
			printf("ERRRRORRRR\n");
		}
		
		wait_code = WaitForSingleObject(msg_q_semaphore, INFINITE);

		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE)
			return ERR;

		printf("\n==================\nRECV THREAD START\n==================\n\n");

		ret_val = msg_q_insert(&msg_struct);
		msg_q_printQ();

		if (ret_val != TRUE)
			return ERR;
		printf("\n==================\nRECV THREAD END\n==================\n\n");
		
		release_res = ReleaseSemaphore(msg_q_semaphore, 1, &previous_count);
		if (release_res == FALSE) goto Error_And_Close;

	Error_And_Close:
		if (ret_val == ERR) 
			return ERR;
		
		freeMessege(&msg_struct);
	}
	return 0;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Sending data to the server
static DWORD SendDataThread(void)
{	
	int ret_val = TRUE;
	int wait_code = TRUE;
	int release_res = TRUE;
	LONG previous_count;


	while (TRUE) 
	{
		//Messege *msg_out = (Messege*)malloc(sizeof(Messege*));
		//Messege *curr_msg = (Messege*)malloc(sizeof(Messege*));
		Messege msg_out;
		Messege *curr_msg;
/*		
		==========================================
			Use messege queue semaphore
		==========================================
*/
		wait_code = WaitForSingleObject(msg_q_semaphore, INFINITE);

		if (checkWaitCodeStatus(wait_code, TRUE) != TRUE)
			return ERR;

		printf("\n==================\nSEND THREAD START\n==================\n\n");

		/*msg_q_printQ();*/

		curr_msg = msg_q_pop();
		
		printf("done popping\n");

		if (release_res == FALSE)
			return ERR;

		if (curr_msg != NULL)
		{			
			ret_val = clientStateMachine(curr_msg, &msg_out);
			if (ret_val == ERR)
				return ERR;

			else if (ret_val == TRUE)
			{
				if (curr_msg != NULL)
				{
					printf("\n==============================\n\n");
					printf("Your messege decoded:\n");
					printMessege(&msg_out);
					sendMessegeWrapper(m_socket_data.socket, msg_out.type, msg_out.params[0], msg_out.params[1], msg_out.params[2], msg_out.params[3], msg_out.params[4]);
					printf("before free msg\n");
					freeMessege(&msg_out);
					printf("after free msg\n");
				}
			}
		}

		release_res = ReleaseSemaphore(msg_q_semaphore, 1, &previous_count);

		printf("\n==================\nSEND THREAD END\n==================\n\n");
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
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

void MainClient(char *ip_addres, char *port_num_char)
{
	HANDLE hThread[2];
	int try_to_reconnect_answer = 0;
    // Initialize Winsock.
    WSADATA wsaData; //Create a WSADATA object called wsaData.
	//The WSADATA structure contains information about the Windows Sockets implementation.
	
	// init messege queue for reciving msgs
	extern msg_fifo *msg_q;
	msg_q_init();

	// Create program semaphores
	createProgramSemaphores();
	createProgramMutexes();

	//Call WSAStartup and check for errors.
    int iResult = WSAStartup( MAKEWORD(2, 2), &wsaData );
    if ( iResult != NO_ERROR )
        printf("Error at WSAStartup()\n");

	strcpy_s(m_socket_data.ip_addres, IP_MAX_LEN, ip_addres);
	strcpy_s(m_socket_data.port_num_char, PORT_MAX_LEN, port_num_char);
	m_socket_data.server_not_client = FALSE;

	//Call the socket function and return its value to the m_socket variable. 
	// For this application, use the Internet address family, streaming sockets, and the TCP/IP protocol.
	
	// Create a socket.
	m_socket_data.socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	// Check for errors to ensure that the socket is a valid socket.
    if (m_socket_data.socket == INVALID_SOCKET ) {
        printf( "Error at socket(): %ld\n", WSAGetLastError() );
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

	hThread[0]=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) SendDataThread,NULL,0,NULL);
	hThread[1]=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) RecvDataThread,NULL,0,NULL);

	WaitForMultipleObjects(2,hThread,FALSE,INFINITE);

	TerminateThread(hThread[0],0x555);
	TerminateThread(hThread[1],0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
MAIN_CLEAN:
	closesocket(m_socket_data.socket);
	
	WSACleanup();
    
	return;
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

int createProgramMutexes()
{
	/*
	Description: init global mutexes
	parameters:
			 - none
	Returns: TRUE if succeded, ERR o.w
	*/

	int retVal = TRUE;

	msg_q_mutex = NULL;
	msg_q_mutex = CreateMutex(NULL, FALSE, NULL);
	if (msg_q_mutex == NULL) {
		retVal = ERR;  goto Main_cleanup;
	}
Main_cleanup:
	if (retVal == ERR) {
		raiseError(6, __FILE__, __func__, __LINE__, "Mutex creation failed\n");
	}
	return retVal;
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

