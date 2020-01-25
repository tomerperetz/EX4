#pragma once
/*
=================================================================================
							Description
=================================================================================
		 Public send recv tools and functions headers

=================================================================================
*/


// Defines ----------------------------------------------------------------->
#ifndef SOCKET_SEND_RECV_TOOLS_H
#define SOCKET_SEND_RECV_TOOLS_H

#define MAX_NUM_OF_PARAMS 5

// Includes ----------------------------------------------------------------->

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#include "hardCodedData.h"

// Structs ----------------------------------------------------------------->

typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;

typedef struct _Messege
{
	char *type;
	int num_of_params;
	char *params[5];
	int params_len_lst[MAX_NUM_OF_PARAMS];
} Messege;

typedef struct _msg_q_item
{
	Messege *data;
	struct _msg_q_item *prev;
	struct _msg_q_item *next;

} msg_q_item;

typedef struct _msg_fifo
{
	msg_q_item *head;
	msg_q_item *tail;

} msg_fifo;

// Declerations ----------------------------------------------------------------->
void printMessege(Messege *msg);

int copyMsg(Messege *msg_src, Messege *msg_dst);

void freeMessege(Messege *msg);

int initMsgParam(char *param, Messege *msg, int param_idx);

int initMessege(Messege *msg, char *type, char *param1, char *param2, char *param3, char *param4, char *param5);

void getEncodeMessegeLength(Messege *msg, int *encoded_messege_len);

int getLen(char *buffer, int idx, char last_char);

void getSegement(char *dst_buffer, char *src_buffer, int start_idx, int last_idx);

int decodeMsg(char *char_arr, Messege *decoded_msg);

void printEncodedMessege(char *encoded_msg);

int encodeMessegeAndSend(Messege *msg, SOCKET sd);

int calcCharLstLen(const char* Buffer);

int sendMessegeWrapper(SOCKET sd, char *type, char *param1, char *param2, char *param3,
	char *param4, char *param5);

int decodeWrapper(Messege *msg, SOCKET *socket);

int msg_q_pop(Messege *msg_dst);

int msg_q_insert(Messege *new_msg);

void msg_q_printQ();

void msg_q_freeQ();

void msg_q_init();


char *getString(FILE* fp);

void lowerCase(char *str);

int checkWaitCodeStatus(DWORD wait_code, BOOL singleNotMultiple);

/**
 * SendBuffer() uses a socket to send a buffer.
 *
 * Accepts:
 * -------
 * Buffer - the buffer containing the data to be sent.
 * BytesToSend - the number of bytes from the Buffer to send.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if sending succeeded
 * TRNS_FAILED - otherwise
 */
TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd );

/**
 * SendString() uses a socket to send a string.
 * Str - the string to send. 
 * sd - the socket used for communication.
 */ 
TransferResult_t SendString(const char *char_arr, SOCKET sd);

/**
 * Accepts:
 * -------
 * ReceiveBuffer() uses a socket to receive a buffer.
 * OutputBuffer - pointer to a buffer into which data will be written
 * OutputBufferSize - size in bytes of Output Buffer
 * BytesReceivedPtr - output parameter. if function returns TRNS_SUCCEEDED, then this 
 *					  will point at an int containing the number of bytes received.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving succeeded
 * TRNS_DISCONNECTED - if the socket was disconnected
 * TRNS_FAILED - otherwise
 */ 
TransferResult_t ReceiveBuffer( char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd );

/**
 * ReceiveString() uses a socket to receive a string, and stores it in dynamic memory.
 * 
 * Accepts:
 * -------
 * OutputStrPtr - a pointer to a char-pointer that is initialized to NULL, as in:
 *
 *		char *Buffer = NULL;
 *		ReceiveString( &Buffer, ___ );
 *
 * a dynamically allocated string will be created, and (*OutputStrPtr) will point to it.
 * 
 * sd - the socket used for communication.
 * 
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving and memory allocation succeeded
 * TRNS_DISCONNECTED - if the socket was disconnected
 * TRNS_FAILED - otherwise
 */ 
TransferResult_t ReceiveString( char** OutputStrPtr, SOCKET sd );



/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/


#endif // SOCKET_SEND_RECV_TOOLS_H