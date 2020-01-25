//==========================================================================
//					Description
//==========================================================================
/*
		mutual communication function such as send and recv, 
		messege encode and decode and msg line
*/
//==========================================================================


// includes ---------------------------------------------------------------->
#include "SocketSendRecvTools.h"
#include "./../group77_ex4_client/SocketClient.h"
#include <stdio.h>

// Globals ----------------------------------------------------------------->
msg_fifo *msg_q;

// Functions --------------------------------------------------------------->
//==========================================================================
//				Communication functions
//==========================================================================

TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd )
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;
	
	while ( RemainingBytesToSend > 0 )  
	{
		/* send does not guarantee that the entire message is sent */
		BytesTransferred = send (sd, CurPlacePtr, RemainingBytesToSend, 0);
		if ( BytesTransferred == SOCKET_ERROR ) 
		{
			printf("CONNECTION ERROR: send() function failed, error #%d\n", WSAGetLastError() );
			return TRNS_FAILED;
		}
		
		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

TransferResult_t SendString( const char *char_arr, SOCKET sd )
{
	/* Send the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t SendRes;

	/* The request is sent in two parts. First the Length of the string (stored in 
	   an int variable ), then the string itself. */
		
	TotalStringSizeInBytes = (int)( strlen(char_arr) + 1 ); // terminating zero also sent	

	SendRes = SendBuffer( 
		(const char *)( &TotalStringSizeInBytes ),
		(int)( sizeof(TotalStringSizeInBytes) ), // sizeof(int) 
		sd );

	if ( SendRes != TRNS_SUCCEEDED ) return SendRes ;

	SendRes = SendBuffer( 
		(const char *)(char_arr),
		(int)( TotalStringSizeInBytes ), 
		sd );

	return SendRes;
}

TransferResult_t ReceiveBuffer( char* OutputBuffer, int BytesToReceive, SOCKET sd )
{
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;
	
	
	while ( RemainingBytesToReceive > 0 )  
	{
		/* send does not guarantee that the entire message is sent */
		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
		if ( BytesJustTransferred == SOCKET_ERROR ) 
		{
			printf("CONNECTION ERROR: recv() function failed, error #%d\n", WSAGetLastError() );
			return TRNS_FAILED;
		}		
		else if (BytesJustTransferred == 0) {
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.
		}
			

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

TransferResult_t ReceiveString( char** OutputStrPtr, SOCKET sd )
{
	/* Recv the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t RecvRes;
	char* StrBuffer = NULL;


	if ( ( OutputStrPtr == NULL ) || ( *OutputStrPtr != NULL ) )
	{
		printf("The first input to ReceiveString() must be " 
			   "a pointer to a char pointer that is initialized to NULL. For example:\n"
			   "\tchar* Buffer = NULL;\n"
			   "\tReceiveString( &Buffer, ___ )\n" );
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the string (stored in 
	   an int variable ), then the string itself. */
		
	RecvRes = ReceiveBuffer( 
		(char *)( &TotalStringSizeInBytes ),
		(int)( sizeof(TotalStringSizeInBytes) ), // 4 bytes
		sd );

	if ( RecvRes != TRNS_SUCCEEDED ) return RecvRes;

	StrBuffer = (char*)malloc( TotalStringSizeInBytes * sizeof(char) );

	if ( StrBuffer == NULL )
		return TRNS_FAILED;

	RecvRes = ReceiveBuffer( 
		(char *)( StrBuffer ),
		(int)( TotalStringSizeInBytes), 
		sd );

	if ( RecvRes == TRNS_SUCCEEDED ) 
		{ *OutputStrPtr = StrBuffer; }
	else
	{
		free( StrBuffer );
	}

	return RecvRes;
}


//==========================================================================
//				Messege usage functions
//==========================================================================

void printMessege(Messege *msg)
{
	/*
	Description: Print msg struct values, for debugging
	parameters:
			 - Messege *msg msg struct
	Returns: void
	*/
	if (msg->type == NULL) return;
	printf("Type: %s\n", msg->type);
	printf("Number of Parameters: %d\n", msg->num_of_params);
	for (int i = 0; i < MAX_NUM_OF_PARAMS; i++) {
		if (msg->params[i] == NULL) return;
		printf("Param %d: %s, Len: %d\n", i, msg->params[i], msg->params_len_lst[i]);
	}
}

int copyMsg(Messege *msg_src, Messege *msg_dst)
{
	/*
	Description: copy src msg struct to dst msg struct
	parameters:
			 - Messege *msg_src 
			 - Messege *msg_dst
	Returns:TRUE if succeded, ERR o.w
	*/
	int ret_val = ERR;

	ret_val = initMessege(msg_dst, msg_src->type, msg_src->params[0], msg_src->params[1], msg_src->params[2], \
		msg_src->params[3], msg_src->params[4]);
	if (ret_val != TRUE)\
	{
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		return ERR;
	}

	return TRUE;
}

void freeMessege(Messege *msg)
{
	/*
	Description: free msg struct fields
	parameters:
			 - Messege *msg
	Returns: void
	*/

	if (msg->type != NULL) {
		free(msg->type);
	}

	for (int i = 0; i < MAX_NUM_OF_PARAMS; i++) {
		if (msg->params[i] != NULL)
		{
			free(msg->params[i]);
			msg->params_len_lst[i] = 0;
		}
	}
	msg->num_of_params = 0;
}

int initMsgParam(char *param, Messege *msg, int param_idx)
{
	/*
	Description: init msg struct param property with given char
	parameters:
			 - char *param - char to init in param property
			 - Messege *msg - msg to init the value into
			 - int param_idx - param index in msg param array
	Returns: TRUE if succeded, ERR o.w
	*/

	if (param == NULL) {
		return TRUE;
	}
	msg->params[param_idx] = (char*)malloc((strlen(param) + 1) * sizeof(char));
	if (msg->params[param_idx] == NULL) {
		return ERR;
	}
	strcpy_s(msg->params[param_idx], (strlen(param) + 1) * sizeof(char), param);
	msg->params_len_lst[param_idx] = (int)strlen(param);
	msg->num_of_params += 1;
	return TRUE;
}

int initMessege(Messege *msg, char *type, char *param1, char *param2, char *param3, char *param4, char *param5)
{
	/*
	Description: init msg struct given values
	parameters:
			 - Messege *msg - msg dst struct
			 - char *type - msg type
			 - char *param<index> - param to init in param array in corresponding index
	Returns: TRUE if succeded, ERR o.w
	*/

	int ret_val = TRUE;

	// Initializtion
	msg->type = NULL;
	msg->num_of_params = 0;
	for (int i = 0; i < MAX_NUM_OF_PARAMS; i++) {
		msg->params[i] = NULL;
		msg->params_len_lst[i] = 0;
	}

	if (type == NULL)
		return ret_val;

	msg->type = (char*)malloc((strlen(type) + 1) * sizeof(char));
	if (msg->type == NULL) {
		ret_val = ERR;
		goto MAIN_CLEAN_UP;
	}

	strcpy_s(msg->type, (strlen(type) + 1) * sizeof(char), type);

	if (initMsgParam(param1, msg, 0) != TRUE) {
		ret_val = ERR;
		goto MAIN_CLEAN_UP;
	}

	if (initMsgParam(param2, msg, 1) != TRUE) {
		ret_val = ERR;
		goto MAIN_CLEAN_UP;
	}
	if (initMsgParam(param3, msg, 2) != TRUE) {
		ret_val = ERR;
		goto MAIN_CLEAN_UP;
	}
	if (initMsgParam(param4, msg, 3) != TRUE) {
		ret_val = ERR;
		goto MAIN_CLEAN_UP;
	}
	if (initMsgParam(param5, msg, 4) != TRUE) {
		ret_val = ERR;
		goto MAIN_CLEAN_UP;
	}
MAIN_CLEAN_UP:
	if (ret_val == ERR) {
		freeMessege(msg);
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
	}
	return ret_val;
}

void getEncodeMessegeLength(Messege *msg, int *encoded_messege_len)
{
	/*
	Description: get wanted length for encoding messege (from struct to msg require: <type>:<param_1>...<param_n>
	parameters:
			 - Messege *msg
			 - int *encoded_messege_len
			 - int param_idx - param index in msg param array
	Returns: void
	*/

	*encoded_messege_len = (int)strlen(msg->type) + 1;
	for (int idx = 0; idx < MAX_NUM_OF_PARAMS; idx++) {
		if (msg->params_len_lst[idx] == 0)
			break;
		*encoded_messege_len += (int)strlen(msg->params[idx]) + 1;
	}
}

void printEncodedMessege(char *encoded_msg)
{
	/*
	Description: print encoded msg - for debugging
	parameters:
			 - char *encoded_msg
	Returns: void
	*/
	char *curr_pos = encoded_msg;
	for (curr_pos; *curr_pos != '\n'; curr_pos++) {
		printf("%c", *curr_pos);
	}
	printf("%c", *curr_pos);

}

int encodeMessegeAndSend(Messege *msg, SOCKET socket)
{
	/*
	Description: encode msg (from struct to desired structure) to and send it using given socket
	parameters:
			 - Messege *msg
			 - SOCKET socket
			 - int param_idx - param index in msg param array
	Returns: TRUE if succeded, ERRR o.w
	*/	
	char *encoded_messege;
	BOOL type_flag = TRUE;
	int encoded_messege_len = 0, params_idx = 0, ret_val = TRUE;
	TransferResult_t send_ret_val = TRNS_SUCCEEDED;

	getEncodeMessegeLength(msg, &encoded_messege_len);
	encoded_messege = (char*)malloc(sizeof(char) * (encoded_messege_len + 1));
	if (encoded_messege == NULL) {
		ret_val = ERR;
		goto MAIN_CLEAN_UP1;
	}
	strcpy_s(encoded_messege, (encoded_messege_len + 1), msg->type);
	while (msg->num_of_params > 0 && msg->params[params_idx] != NULL) {
		if (type_flag) {
			strcat_s(encoded_messege, (encoded_messege_len + 1), ":");
			type_flag = FALSE;
		}
		else {
			strcat_s(encoded_messege, (encoded_messege_len + 1), ";");
		}
		strcat_s(encoded_messege, (encoded_messege_len + 1), msg->params[params_idx]);
		params_idx++;
		msg->num_of_params--;
	}
	encoded_messege[(int)strlen(encoded_messege)] = '\n';

	send_ret_val = SendString(encoded_messege, socket);
	if (send_ret_val == TRNS_FAILED) {
		ret_val = ERR;
		goto MAIN_CLEAN_UP2;
	}


MAIN_CLEAN_UP1:
	if (ret_val == ERR) {
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
	}
MAIN_CLEAN_UP2:
	free(encoded_messege);
	return ret_val;
}

int calcCharLstLen(const char* Buffer)
{
	/*
	Description: caculate char length from given index to first \n char
	parameters:
			 - const char* Buffer - start index
	Returns: len + 1
	*/
	const char* CurPlacePtr = Buffer;
	int len = 0;
	if (CurPlacePtr == NULL) return ERR;
	while (*CurPlacePtr != '\n') {
		len += 1;
		CurPlacePtr += 1;
	}
	return len + 1;
}

int sendMessegeWrapper(SOCKET socket, char *type, char *param1, char *param2, char *param3,
	char *param4, char *param5)
{
	/*
	Description: init messege from params, encode and send it
	parameters:
			 - SOCKET socket 
			 - char *type - msg type
			 - char *param_i - msg params
	Returns: TRUE if succeded, ERRR o.w
	*/

	Messege msg;
	int ret_val = TRUE;
	if (type == NULL) {
		raiseError(16, __FILE__, __func__, __LINE__, "Messege Type Error: No Such Messege!\n");
		return ERR;
	}

	ret_val = initMessege(&msg, type, param1, param2, param3, param4, param5);
	if (ret_val != TRUE) {
		return ERR;
	}

	ret_val = encodeMessegeAndSend(&msg, socket);
	freeMessege(&msg);

	return ret_val;

}

int getLen(char *buffer, int idx, char last_char)
{
	/*
	Description: get length between givin index and last char
	parameters:
			 - char *buffer - buffer to search on
			 - int idx - start idx
			 - char last_char - last char to find
	Returns: len
	*/
	int len = 0;
	for (idx; buffer[idx] != last_char && buffer[idx] != '\n'; idx++)
	{
		len++;
	}
	return len;
}

void getSegement(char *dst_buffer, char *src_buffer, int start_idx, int last_idx)
{
	/*
	Description: get segment from a char array and copy it to dst char array
	parameters:
			 - char *dst_buffer
			 - char *src_buffer
			 - int start_idx
			 - int last_idx
	Returns: void
	*/
	int dst_idx = 0;
	for (int i = start_idx; i < last_idx; i++)
	{
		dst_buffer[dst_idx] = src_buffer[i];
		dst_idx++;
	}
	dst_buffer[dst_idx] = '\0';
	return;
}

int decodeMsg(char *char_arr, Messege *decoded_msg)
{

	/*
	Description: convert msg recivied from <type>:<param_1>..<param_n> to msg struct 
	parameters:
			 - Messege *decoded_msg - dst msg to init
			 - char *char_arr - char array recived
	Returns: void
	*/
	int idx = 0;
	int len = 0;
	int last_idx = 0;
	int flag = 1;
	int param_idx = 0;
	char last_char = ':';
	int ret_val = TRUE;

	// Initializtion
	ret_val = initMessege(decoded_msg, NULL, NULL, NULL, NULL, NULL, NULL);
	
	if (ret_val != TRUE)
	{
		printf("Error initializing messege in decoder!\n");
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_7_OTHER);
		return ERR;
	}

	for (idx=0; char_arr[idx]!='\n'; idx++)
	{
		// Calc paramter length
		len = getLen(char_arr, idx, last_char);
		last_idx = idx + len;
		// first iteration, this is the messege type
		if (flag)
		{
			// allocate memory
			decoded_msg->type = (char*)malloc((len + 1) * sizeof(char));
			if (decoded_msg->type == NULL)
			{
				raiseError(4, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
				return ERR;
			}
			
			getSegement(decoded_msg->type, char_arr, idx, last_idx);
			last_char = ';';
			flag = 0;
		}

		// not first iteration, this is a paramter
		else
		{
			decoded_msg->params[param_idx] = (char*)malloc((len + 1) * sizeof(char));
			if (decoded_msg->params[param_idx] == NULL)
			{
				raiseError(4, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
				return ERR;
			}

			// update corresponding struct field
			getSegement(decoded_msg->params[param_idx], char_arr, idx, last_idx);
			decoded_msg->params_len_lst[param_idx] = (int)strlen(decoded_msg->params[param_idx]);

			// update for next iteration
			param_idx++;
			decoded_msg->num_of_params++;
		}

		idx = last_idx;
		if (char_arr[idx] == '\n')
			break;
	}

	return TRUE;
}

int decodeWrapper(Messege *msg, SOCKET *socket) {
	/*
	Description: decode wrapper - recv msg and decode it into struct
	parameters:
			 - Messege *msg - dsp msg struct
			 - SOCKET *socket 
	Returns: TRUE if succeded, ERR for error or EXIT_PROGRAM for shutdown msg
	*/

	char *AcceptedStr = NULL;
	int ret_val = TRUE;
	TransferResult_t RecvRes;

	RecvRes = ReceiveString(&AcceptedStr, *socket);
	if (RecvRes == TRNS_FAILED)
	{
		return ERR;
	}
	else if (RecvRes == TRNS_DISCONNECTED)
	{
		return EXIT_PROGRAM;
	}
	else
	{
		ret_val = decodeMsg(AcceptedStr, msg);
		if (ret_val == ERR) return ERR;
	}

	free(AcceptedStr);
	
	return TRUE;
}

char *getString(FILE* fp)
{
	/*
	Description: get input from user in any length,  until he hits Enter key
	parameters:
			 - FILE* fp - buffer to read from (we send stdin)
	Returns: char arr recived
	*/

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
		if (len == size)
		{
			str = realloc(str, sizeof(char)*(size += 16));
			if (str == NULL)
			{
				raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
				return str;
			}
		}
	}
	str[len++] = '\0';
	return realloc(str, sizeof(char)*len);
}

void lowerCase(char *str)
{
	/*
	Description: recieve char array in any form (lower/higher/mixed case) and convert all to lower case
	parameters:
			 - char *str
	Returns: void, convert arr itself
	*/
	for (int i = 0; str[i] != '\0'; i++)
		str[i] = tolower(str[i]);
}

//==========================================================================
//					Messege queue functions
//==========================================================================

void msg_q_init()
{
	/*
	Description: init msg q struct
	parameters: none
	Returns: void
	*/
	extern msg_fifo *msg_q;
	msg_q = (msg_fifo*)malloc(sizeof(msg_fifo));
	msg_q->head = NULL;
	msg_q->tail = NULL;
	
}

int msg_q_pop(Messege *msg_dst)
{
	/*
	Description: pop msg from fifo queue
	parameters: 
			- Messege *msg_dst - load msg popped into this variable
	Returns: TRUE if we had msg, FALSE if q is empty
	*/
	extern msg_fifo *msg_q;
	msg_q_item *temp;
	Messege *pop_data = NULL;
	if (msg_q->head == NULL)
	{ 
		// No msg in head of line
		return FALSE;
	}
	
	// init dst msg with current head data
	pop_data = msg_q->head->data;
	initMessege(msg_dst, pop_data->type, pop_data->params[0], pop_data->params[1], pop_data->params[2], pop_data->params[3], pop_data->params[4]);

	// replace head, free old head
	temp = msg_q->head;
	msg_q->head = msg_q->head->prev;
	free(temp);

	return TRUE;

}

int msg_q_insert(Messege *src_msg)
{
	/*
	Description: insert msg to fifo queue
	parameters:
			- Messege *msg_src - msg we want to insert to q
	Returns: TRUE if succeded, ERR ow
	*/
	extern msg_fifo *msg_q;
	msg_q_item *new_node;
	Messege *dst_msg;
	int ret_val = ERR;

	// allocate memory for new msg in line
	dst_msg = (Messege*)malloc(sizeof(Messege));
	if (dst_msg == NULL)
	{
		raiseError(4, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
		return ERR;
	}

	// copy src msg fields into dst msg
	ret_val = copyMsg(src_msg, dst_msg);
	if (ret_val == ERR)
	{
		printf("Error in copy msg\n");
		raiseError(4, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
		return ERR;
	}
		
	// allocate new node on list
	new_node = (msg_q_item*)malloc(sizeof(msg_q_item));
	if (new_node == NULL)
	{
		raiseError(4, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
		return ERR;
	}

	// add to msg Q
	// case A: no items in line
	if (msg_q->head == NULL)
	{
		// head is the new msg node
		msg_q->head = new_node;
		msg_q->tail = new_node;

		// new node data is dst msg
		new_node->data = dst_msg;
		new_node->next = NULL;
		new_node->prev = NULL;
	}

	// case B: items in line, add elemnt last in line
	else
	{
		// tail prev pointing to new node
		msg_q->tail->prev = new_node;

		// new node next pointing to old tail
		new_node->next = msg_q->tail;

		// new tail is new node
		msg_q->tail = new_node;

		// new node data is dst msg, prev is null - this is the tail.
		new_node->data = dst_msg;
		new_node->prev = NULL;

	}

	return TRUE;

}

void msg_q_printQ()
{
	/*
	Description: print msg q. for debugging.
	parameters: none
	Returns: void
	*/

	extern msg_fifo *msg_q;
	msg_q_item *curr;
	curr = msg_q->head;
	printf("======================\n");
	printf("Messege Q:\n");
	printf("======================\n");
	if (curr == NULL)
	{
		printf("no items in list\n");
		return;
	}
	do
	{
		printMessege(curr->data);
		curr = curr->prev;
	} while (curr != NULL);

	printf("Messege Q done printing\n");
	printf("======================\n");
}

void msg_q_freeQ()
{
	/*
	Description: for all messeges in q
	parameters: none
	Returns: void
	*/
	extern msg_fifo *msg_q;
	msg_q_item *curr = msg_q->head;

	// if line is empty
	if (curr == NULL)
		return;

	// else free line
	do
	{
		freeMessege(curr->data);
		curr = curr->prev;
		free(curr->next);
	} while (curr != NULL);


}

//==========================================================================
//					mutex\semaphore functions
//==========================================================================

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
