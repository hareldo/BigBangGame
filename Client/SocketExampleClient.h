/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering, Winter 2011, 
 by Amnon Drory.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifndef SOCKET_EXAMPLE_CLIENT_H
#define SOCKET_EXAMPLE_CLIENT_H

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"

#define STATUS_CODE_FAILURE FALSE
#define STATUS_CODE_SUCCESS TRUE
#define MAX_LENGTH 20
#define MAX_LENGTH_IP_ADRESS 15
#define WAIT_15S 15000
#define WAIT_30S 30000
#define LONGEST_MESSAGE MAX_LENGTH+14 
#define CPU_LONG 3
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
enum state { again, endofwork, notValid };
enum server {
	SERVER_MAIN_MENU, SERVER_APPROVED, SERVER_DENIED, SERVER_INVITE, SERVER_PLAYER_MOVE_REQUEST,
	SERVER_GAME_RESULTS, SERVER_GAME_OVER_MENU, SERVER_OPPONENT_QUIT, SERVER_NO_OPPONENTS, SERVER_LEADERBOARD, SERVER_LEADERBORAD_MENU
};

typedef struct Client{
	char ip_adress[MAX_LENGTH_IP_ADRESS];
	int portName;
	char usrename[MAX_LENGTH];
}client;
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static DWORD RecvDataThread(LPVOID lpParam);
static DWORD SendDataThread(LPVOID lpParam);
int main(int argc, char ** argv);
int What_to_do();
int Server_ReceiveString(char *ptr);
int check_if_SendReceiveString(TransferResult_t RecvRes);
int CreateAllSemphores();
void closeAllHandle();
int str_prefix(char *str, const char *prefix);

int TimeTogoOut = 0, ServerNoOpponemts = 0, ServerOpponemtsQuit = 0;
SOCKET m_socket;
HANDLE TimeOut_conctionProblem_GoOut_Semphore = NULL, SERVER_APROVED_Semphore = NULL, SERVER_MAIN_MENU_Semphore = NULL,
SERVER_INVITE_NO_OPPONENTS_Semphore = NULL, SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT = NULL,
SERVER_GAME_RESULTS_Semphore = NULL, SERVER_GAME_OVER_MENU_Semphore = NULL,
SERVER_LEADERBOARD_Semphore = NULL, SERVER_LEADERBORAD_MENU_Semphore = NULL;


#endif // SOCKET_EXAMPLE_CLIENT_H