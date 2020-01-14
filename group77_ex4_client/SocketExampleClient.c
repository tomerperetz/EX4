#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../Shared/hardCodedData.h"

Socket_info m_socket_data;

//Reading data coming from the server
static DWORD RecvDataThread(void)
{
	extern Messege g_msg_in;
	int ret_val = TRUE;
	while (1)
	{

		Messege msg_struct;
		ret_val = decodeWrapper(&msg_struct, &m_socket_data.socket);
		if (ret_val == ERR) {
			/*TO DO*/
			printf("ERRRRORRRR\n");
		}
		

		/*=====================
			MUTEX START
		=====================*/

		ret_val = copyMsg(&msg_struct, &g_msg_in);
		
		if (ret_val != TRUE)
			return ERR;

		/*=====================
			MUTEX END
		=====================*/
		printMessege(&msg_struct);
		freeMessege(&msg_struct);
	}
	return 0;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Sending data to the server
static DWORD SendDataThread(void)
{
	extern Messege g_msg_in;
	
	int ret_val = TRUE;
	while (1) 
	{
		Messege msg_out;
/*		
		=====================
			STATE MACHINE
		=====================
*/
		
		ret_val = clientStateMachine(&msg_out, &g_msg_in);
		printMessege(&msg_out);

		if (ret_val != TRUE)
			return ERR;
/*
		=====================
		   ENCODE AND SEND
		=====================
*/
		
		sendMessegeWrapper(m_socket_data.socket, "ALL_GOOD", "111", NULL, NULL, NULL, NULL);
		//sendMessegeWrapper(m_socket_data.socket, msg_out.type, msg_out.params[0], msg_out.params[1], \
		//	msg_out.params[2], msg_out.params[3], msg_out.params[4]);

		//freeMessege(&msg_out);
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
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

