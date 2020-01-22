#include "client_services.h"
#include "./SocketExampleClient.h"
#include <ctype.h>

// global variables ------------------------------------------------------>


// Functions ------------------------------------------------------------->

//==========================================================================
//					State Machine Functions
//==========================================================================


int encodeMessegeLocal(Messege *msg)
{
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

	for (int i = 0; encoded_messege[i] != '\n'; i++)
		printf("%c",encoded_messege[i]);
	printf("\n");

MAIN_CLEAN_UP1:
	if (ret_val == ERR) {
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
	}
//MAIN_CLEAN_UP2:
	free(encoded_messege);
	return ret_val;
}


