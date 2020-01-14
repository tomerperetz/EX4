#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../Shared/hardCodedData.h"


SOCKET m_socket;

//Reading data coming from the server
static DWORD RecvDataThread(void)
{
	extern Messege g_msg_in;
	int ret_val = TRUE;
	while (1)
	{

		Messege msg_struct;
		ret_val = decodeWrapper(&msg_struct, &m_socket);
		if (ret_val == ERR) {
			/*TO DO*/
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

		freeMessege(&msg_struct);
	}
	return 0;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Sending data to the server
static DWORD SendDataThread(void)
{
	extern Messege g_msg_in;
	
	int ret_val;
	while (1) 
	{
		Messege msg_out;
/*		
		=====================
			STATE MACHINE
		=====================
*/

		ret_val = clientStateMachine(msg_out);
		if (ret_val != TRUE)
			return ERR;
/*
		=====================
		   ENCODE AND SEND
		=====================
*/
		sendMessegeWrapper(m_socket, msg_out.type, msg_out.params[0], msg_out.params[1], \
			msg_out.params[2], msg_out.params[3], msg_out.params[4]);

		freeMessege(&msg_out);
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void MainClient(char *ip_addres, char *port_num_char)
{
	SOCKADDR_IN clientService;
	HANDLE hThread[2];

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
	if ( connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
        printf( "Failed to connect.\n" );
        WSACleanup();
        return;
    }

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

