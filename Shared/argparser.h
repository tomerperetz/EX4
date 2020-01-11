/*
====================================================================================================================
Description:
this lib is a parser for arguments recieved from user.
====================================================================================================================
*/

#pragma once
#ifdef _MSC_VER
#endif

#include "hardCodedData.h"
#include <sys/stat.h>

static const int EXPECTED_ARGC = 2;
static const char HELPER[1000] = \
"===========================================================================================\n"\
"This Program initalize a server allowing players play Rock, Papers, Scissors,Spock, Lizard \n"\
"You can play against another player, against CPU or watch leaderboard. \n"\
"Server inputs: port num\n"\
"Client inputs: server ip, server port num, user name \n"\
"Output: Leaderboard will be printed to an output file \n"\
"===========================================================================================\n";

int isDirectory(const char *path);
int ensureArgs(int argc, int expected_argc, char *argv[]);
int isArgsValid(int argc, char *argv[]);
void callHelper();

