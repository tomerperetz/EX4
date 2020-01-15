#include "client_services.h"
#include "./SocketExampleClient.h"

Messege g_msg_in;

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
		if (max_menu_option == ERR)
			// in case it we only print, don't wait for answer
			return;
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

//void printMsg(Messege *msg)
//{
//
//	if (strcmp(msg->type, SERVER_MAIN_MANU) == 0)
//	{
//		//printf(SERVER_MAIN_MANU_MSG);
//	}
//	else if (strcmp(msg->type, SERVER_APPROVED) == 0)
//	{
//		//printf(SERVER_APPROVED_MSG);
//	}
//	else if (strcmp(msg->type, SERVER_DENIED) == 0)
//	{
//		//printf(SERVER_DENIED_MSG);
//	}
//	else if (strcmp(msg->type, SERVER_INVITE) == 0)
//	{
//		// DO SOMETHING
//	}
//	else if (strcmp(msg->type, SERVER_PLAYER_MOVE_REQUEST) == 0)
//	{
//		//printf(SERVER_PLAYER_MOVE_REQUEST_MSG);
//	}
//	else if (strcmp(msg->type, SERVER_GAME_RESULTS) == 0)
//	{
//		//printf(SERVER_GAME_RESULTS_MSG);
//	}
//	else if (strcmp(msg->type, SERVER_GAME_OVER_MANU) == 0)
//	{
//		//printf(SERVER_GAME_OVER_MANU_MSG);
//	}
//	else if (strcmp(msg->type, SERVER_OPPONENT_QUIT) == 0)
//	{
//		//printf(SERVER_OPPONENT_QUIT_MSG);
//	}
//	else if (strcmp(msg->type, SERVER_NO_OPPONENTS) == 0)
//	{
//		//printf("SERVER_NO_OPPONENTS_MSG");
//	}
//	else if (strcmp(msg->type, SERVER_LEADERBOARD) == 0)
//	{
//		//printf("Print leader board\n");
//	}
//	else if (strcmp(msg->type, SERVER_LEADERBOARD_MANU) == 0)
//	{
//		//printf(SERVER_LEADERBOARD_MANU_MSG);
//	}
//	else {
//		//printf("seems we don't handle this message type: %s", msg->type);
//	}
//		
//
//}


int clientStateMachine(Messege *msg_out, Messege *g_msg_in)
{
	char *SendStr;
	int ret_val = TRUE;
	int user_answer = ERR;
	//printf("++++++++++++++++++++\n");
	//printMessege(g_msg_in);
	//printf("%s   %s\n", g_msg_in->type, SERVER_MAIN_MANU);
	while (TRUE)
		if (strcmp(g_msg_in->type, SERVER_MAIN_MANU) == 0)
		{
			printMenuAndGetAnswer(SERVER_MAIN_MANU_MSG, &user_answer, 4);
			break;
		}
		else if (strcmp(g_msg_in->type, SERVER_APPROVED) == 0)
		{
			printMenuAndGetAnswer(SERVER_APPROVED_MSG, &user_answer, 0);
			break;
		}
		else if (strcmp(g_msg_in->type, SERVER_DENIED) == 0)
		{
			printMenuAndGetAnswer(SERVER_DENIED_MSG, &user_answer, 0);
			break;
		}
		else if (strcmp(g_msg_in->type, SERVER_INVITE) == 0)
		{
			printf(SERVER_DENIED_MSG, g_msg_in->params[0]);
			break;
		}
		else if (strcmp(g_msg_in->type, SERVER_PLAYER_MOVE_REQUEST) == 0)
		{
			printMenuAndGetAnswer(SERVER_PLAYER_MOVE_REQUEST_MSG, &user_answer, 0);
			break;
		}
		else if (strcmp(g_msg_in->type, SERVER_GAME_RESULTS) == 0)
		{
			printf(SERVER_GAME_RESULTS_MSG, g_msg_in[0], g_msg_in[1], g_msg_in[2]);
			break;
		}
		else if (strcmp(g_msg_in->type, SERVER_GAME_OVER_MANU) == 0)
		{
			printMenuAndGetAnswer(SERVER_GAME_OVER_MANU_MSG, &user_answer, 2);
			break;
		}
		else if (strcmp(g_msg_in->type, SERVER_OPPONENT_QUIT) == 0)
		{
			printMenuAndGetAnswer(SERVER_OPPONENT_QUIT_MSG, &user_answer, 0);
			break;
		}
		else if (strcmp(g_msg_in->type, SERVER_NO_OPPONENTS) == 0)
		{
			printMenuAndGetAnswer(SERVER_NO_OPPONENTS_MSG, &user_answer, 0);
			printMenuAndGetAnswer(SERVER_MAIN_MANU_MSG, &user_answer, 4);
			break;
		}
		else if (strcmp(g_msg_in->type, SERVER_LEADERBOARD) == 0)
		{
			printf("Print leader board somehow\n");
			printMenuAndGetAnswer(SERVER_LEADERBOARD_MANU_MSG, &user_answer, 2);
			break;
		}
		else
		{
			printf("seems we don't handle this message type: %s", g_msg_in->type);
			break;
		}

		return TRUE;
}

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



MAIN_CLEAN_UP1:
	if (ret_val == ERR) {
		raiseError(7, __FILE__, __func__, __LINE__, ERROR_ID_4_MEM_ALLOCATE);
	}
MAIN_CLEAN_UP2:
	free(encoded_messege);
	return ret_val;
}

void runClientTest()
{
	Messege msg_in;
	Messege msg_out;
	char user_answer;
	char text[50] = "Please type in a messege sent from server";
	initMessege(&msg_in, SERVER_MAIN_MANU, NULL, NULL, NULL, NULL, NULL);
	printf("Welcome to client test program. type in decoded messege and recieve client reaction");

	while (TRUE)
	{
		clientStateMachine(&msg_in, &msg_out);
		user_answer = getString(stdin);
		if (user_answer == NULL) {
			printf("error reciving user answer");
			break;
		}
		if (strcmp(user_answer, "quit"))
			break;
		encodeMessegeLocal(&msg_in);
	}

}