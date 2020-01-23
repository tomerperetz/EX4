#pragma once
#include "../Shared/hardCodedData.h"

// defines ------------------------------------------------------->
#define STATUS_INIT -1
#define STATUS_CLIENT_CPU 0
#define STATUS_CLIENT_VS 1
#define STATUS_CLIENT_QUIT 2
#define MAX_USERS 2
#define NO_PARTNER 0
// Structs ------------------------------------------------------->

//Declerations ---------------------------------------------------->
int client_vs_cpu(SOCKET *socket, Player *player);

int searchPartner();

int client_vs_client(SOCKET *socket, User *usr);

void initUser(User *new_user, Player *p_player_data, int status, int idx, BOOL online);

int seekAndDestroy();