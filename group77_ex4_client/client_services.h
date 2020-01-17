// Includes ----------------------------------------------------------------->
#pragma once
#include "../Shared/hardCodedData.h"

// Hardcoded messages

#define SERVER_INVITE "SERVER_INVITE"
#define SERVER_INVITE_MSG "%s"


#define SERVER_NO_OPPONENTS "SERVER_NO_OPPONENTS"
#define SERVER_NO_OPPONENTS_MSG "Couldn't find and opponent. Please try again later.\n"

#define SERVER_LEADERBOARD "SERVER_LEADERBOARD"

#define CONNECTION_SUCCEEDED_MSG  "Connected to server on %s:%s\n"

#define CONNECTION_FAILED_MSG "Failed connecting to server on %s:%s.\n"\
												"Choose what to do next:\n"\
												"1.	Try to reconnect\n"\
												"2. Exit\n" 

#define CONNECTION_LOST_MSG "Failed connecting to server on %s:%s has been lost.\n"\
										"Choose what to do next:\n"\
										"1.	Try to reconnect\n"\
										"2. Exit\n"

#define SERVER_APPROVED "SERVER_APPROVED"
#define SERVER_APPROVED_MSG  "Connected to server on %s:%s approved\n"

#define SERVER_DENIED "SERVER_DENIED"
#define SERVER_DENIED_MSG_ARGS "Server on %s:%s denied the connection request, %s.\n"
#define SERVER_DENIED_MSG_MANU "Choose what to do next:\n"\
											"1. Try to reconnect\n"\
											"2. Exit\n"

#define SERVER_MAIN_MANU "SERVER_MAIN_MANU"
#define SERVER_MAIN_MANU_MSG "Choose what to do next:\n"\
											"1.Play against another client\n"\
											"2.Play against the server\n"\
											"3.View the leaderboard\n"\
											"4.Quit\n"

#define SERVER_LEADERBOARD_MANU "SERVER_LEADERBOARD_MANU"
#define SERVER_LEADERBOARD_MANU_MSG "Choose what to do next:\n"\
													"1. Refresh leaderboard\n"\
													"2. Return to the main manu\n"

#define SERVER_PLAYER_MOVE_REQUEST "SERVER_PLAYER_MOVE_REQUEST"
#define SERVER_PLAYER_MOVE_REQUEST_MSG "Choose a move from the list: Rock, Paper, Scissors, Lizard or Spock\n"

#define SERVER_GAME_RESULTS "SERVER_GAME_RESULTS"
#define SERVER_GAME_RESULTS_MSG "You played: %s\n"\
												"%s played: %s\n"\
												"%s won!\n"

#define SERVER_GAME_RESULTS_DRAW_MSG "You played: %s\n"\
													"%s played: %s\n"

#define SERVER_GAME_OVER_MANU "SERVER_GAME_OVER_MANU"
#define SERVER_GAME_OVER_MANU_MSG "Choose what to do next:\n"\
												"1. Play again\n"\
												"2. Return to the main manu\n"

#define SERVER_OPPONENT_QUIT "SERVER_OPPONENT_QUIT"
#define SERVER_OPPONENT_QUIT_MSG "%s has left the game!\n"

#define NO_NEED_TO_REPLY -100

// Declerations -------------------------------------------------------------------->



//int clientStateMachine(Messege *msg_out, Messege *g_msg_in);
//
//void runClientTest();
//

//
//void printMsg(Messege *msg);

