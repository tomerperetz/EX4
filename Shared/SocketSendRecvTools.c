#include "SocketSendRecvTools.h"


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
void printMessege(Messege *msg)
{
	if (msg->type == NULL) return;
	printf("Type: %s\n", msg->type);
	printf("Number of Parameters: %d\n", msg->num_of_params);
	for (int i = 0; i < MAX_NUM_OF_PARAMS; i++) {
		if (msg->params[i] == NULL) return;
		printf("Param %d: %s, Len: %d\n", i, msg->params[i], msg->params_len_lst[i]);
	}
}

void freeMessege(Messege *msg) 
{
	if (msg->type != NULL) {
		free(msg->type);
	}

	for (int i = 0; i < MAX_NUM_OF_PARAMS; i++) {
		if (msg->params[i] != NULL) {
			free(msg->params[i]);
			msg->params_len_lst[i] = 0;
		}	
	}
	msg->num_of_params = 0;
}

int initMsgParam(char *param, Messege *msg, int param_idx) 
{
	if (param == NULL) {
		return TRUE;
	}
	msg->params[param_idx] = (char*)malloc((strlen(param) + 1) * sizeof(char));
	if (msg->params[param_idx] == NULL) {
		return ERR;
	}
	strcpy_s(msg->params[param_idx], (strlen(param) + 1) * sizeof(char),param);
	msg->params_len_lst[param_idx] = (int) strlen(param);
	msg->num_of_params += 1;
	return TRUE;
}

int initMessege(Messege *msg, char *type, char *param1, char *param2, char *param3, char *param4, char *param5)
{

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

	msg->type = (char*) malloc((strlen(type) + 1) * sizeof(char));
	if (msg->type == NULL) {
		ret_val = ERR;
		goto MAIN_CLEAN_UP;
	}
	
	strcpy_s(msg->type, (strlen(type)+1) * sizeof(char),type);
	
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
	*encoded_messege_len = (int) strlen(msg->type) + 1;
	for (int idx = 0; idx < MAX_NUM_OF_PARAMS; idx++) {
		if (msg->params_len_lst[idx] == 0)
			break;
		*encoded_messege_len += (int) strlen(msg->params[idx]) + 1;
	}
}

void printEncodedMessege(char *encoded_msg)
{
	char *curr_pos = encoded_msg;
	for (curr_pos; *curr_pos != '\n'; curr_pos++) {
		printf("%c", *curr_pos);
	}
	printf("%c", *curr_pos);

}

int encodeMessegeAndSend(Messege *msg, SOCKET socket)
{
	char *encoded_messege;
	BOOL type_flag = TRUE;
	int encoded_messege_len = 0, params_idx = 0, ret_val = TRUE;
	TransferResult_t send_ret_val = TRNS_SUCCEEDED;

	getEncodeMessegeLength(msg, &encoded_messege_len);
	encoded_messege = (char*) malloc (sizeof(char) * (encoded_messege_len + 1 ));
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
	encoded_messege[(int) strlen(encoded_messege)] = '\n';
	
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
			printf("send() failed, error %d\n", WSAGetLastError() );
			raiseError(15, __FILE__, __func__, __LINE__, "\n");
			return TRNS_FAILED;
		}
		
		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

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

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

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
			printf("recv() failed, error %d\n", WSAGetLastError() );
			return TRNS_FAILED;
		}		
		else if ( BytesJustTransferred == 0 )
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

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

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

int getLen(char *buffer, int idx, char last_char)
{
	int len = 0;
	for (idx; buffer[idx] != last_char && buffer[idx] != '\n'; idx++)
	{
		len++;
	}
	return len;
}

//void getSegement2(char *dst_buffer, char *src_buffer, int *src_idx, char last_char)
//{
//	int dst_idx = 0;
//	int src_idx_local = *src_idx;
//	for (src_idx_local; src_buffer[src_idx_local] != last_char && src_buffer[src_idx_local] != '\n'; src_idx_local++)
//	{
//		dst_buffer[dst_idx] = src_buffer[src_idx_local];
//		dst_idx++;
//	}
//	
//	if (src_buffer[src_idx_local] != '\n')
//		src_idx_local++;
//	
//	dst_buffer[dst_idx] = '\0';
//	*src_idx = src_idx_local;
//	return;
//}

void getSegement(char *dst_buffer, char *src_buffer, int start_idx, char last_idx)
{
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
	int idx = 0;
	int len = 0;
	int last_idx = 0;
	int flag = 1;
	int param_idx = 0;
	char last_char = ':';

	// Initializtion
	decoded_msg->type = NULL;
	decoded_msg->num_of_params = 0;
	for (int i = 0; i < MAX_NUM_OF_PARAMS; i++) {
		decoded_msg->params[i] = NULL;
		decoded_msg->params_len_lst[i] = 0;
	}

	decoded_msg->num_of_params = 0;

	for (idx=0; char_arr[idx]!='\n'; idx++)
	{
		// Calc paramter length
		len = getLen(char_arr, idx, last_char);
		last_idx = idx + len;
		// allocate memory
		// first iteration, this is the messege type
		if (flag)
		{
			decoded_msg->type = (char*)malloc((len + 1) * sizeof(char));
			getSegement(decoded_msg->type, char_arr, idx, last_idx);
			last_char = ';';
			flag = 0;
		}

		// not first iteration, this is a paramter
		else
		{
			decoded_msg->params[param_idx] = (char*)malloc((len + 1) * sizeof(char));

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
	char *AcceptedStr = NULL;
	TransferResult_t RecvRes;

	RecvRes = ReceiveString(&AcceptedStr, *socket);
	if (RecvRes == TRNS_FAILED)
	{
		printf("Service socket error while reading, closing thread.\n");
		closesocket(*socket);
		return ERR;
	}
	else if (RecvRes == TRNS_DISCONNECTED)
	{
		printf("Connection closed while reading, closing thread.\n");
		closesocket(*socket);
		return ERR;
	}
	else
	{
		decodeMsg(AcceptedStr, msg);
	}
	free(AcceptedStr);
	return TRUE;
}

//int decodeMsg2(char *char_arr, Messege *decoded_msg)
//{
//	int idx = 0;
//	int len = 0;
//	int end = 0;
//	int flag = 1;
//	int param_idx = 0;
//	char last_char = ':';
//
//	initMessege(decoded_msg, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
//	
//	decoded_msg->num_of_params = 0;
//	
//	while (char_arr[idx] != '\n')
//	{
//		// Calc paramter length
//		len = getLen(char_arr, idx, last_char);
//		end = idx + len;
//		// allocate memory
//
//		// first iteration, this is the messege type
//		if (flag)
//		{
//			decoded_msg->type = (char*)malloc((len + 1) * sizeof(char));
//			getSegement(decoded_msg->type, char_arr, &idx, last_char);
//			last_char = ';';
//			flag = 0;
//		}
//		
//		// not first iteration, this is a paramter
//		else
//		{
//			decoded_msg->params[param_idx] = (char*)malloc((len + 1) * sizeof(char));
//			
//			// update corresponding struct field
//			getSegement(decoded_msg->params[param_idx], char_arr, &idx, last_char);
//			decoded_msg->params_len_lst[param_idx] = (int)strlen(decoded_msg->params[param_idx]);
//
//			// update for next iteration
//			param_idx++;
//			decoded_msg->num_of_params++;
//		}	
//		
//		
//	}
//
//	
//
//	return TRUE;
//}