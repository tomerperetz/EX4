// Includes ----------------------------------------------------------------->
#pragma once
#include "../Shared/hardCodedData.h"

// Hardcoded messages
static const char CONNECTION_SUCCEEDED[100] = "Connected to server on %s:%s\n";
static const char CONNECTION_FAILED[100] = "Failed connecting to server on %s:%s.\n"\
												"Choose what to do next:\n"\
												"1.	Try to reconnect\n"\
												"2. Exit\n" ;
static const char CONNECTION_LOST[150] = "Failed connecting to server on %s:%s has been lost.\n"\
										"Choose what to do next:\n"\
										"1.	Try to reconnect\n"\
										"2. Exit\n";
static const char SERVER_APPROVED[100] = "Connection to server on %s:%s APPROVED\n";
static const char SERVER_DENIED[150] = "Server on %s:%s denied the connection request\n"\
											"Choose what to do next:\n"\
											"1.	Try to reconnect\n"\
											"2. Exit\n";
static const char SERVER_MAIN_MANU[150] = "Choose what to do next:\n"\
											"1.	Play against another client\n"\
											"2. Play against the server\n"\
											"3.	View the leaderboard\n"\
											"4. Quit\n";
static const char SERVER_LEADERBOARD_MANU[150] = "Choose what to do next:\n"\
													"1.	Refresh leaderboard\n"\
													"2. Return to the main manu\n";
static const char SERVER_PLAYER_MOVE[150] = "Choose a move from the list: Rock, Paper, Scissors, Lizard or Spock\n";
static const char SERVER_GAME_RESULTS[150] = "You played: %s\n"\
												"%s played: %s\n"\
												"%s won!\n";
static const char SERVER_GAME_RESULTS_DRAW[150] = "You played: %s\n"\
													"%s played: %s\n";
static const char SERVER_GAMEOVER_MANU[150] = "Choose what to do next:\n"\
												"1.	Play again\n"\
												"2. Return to the main manu\n";	
static const char SERVER_OPPONENT_QUIT[150] = "%s has left the game!\n";



