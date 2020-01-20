



#include "SocketExampleClient.h"

//Reading data coming from the server
static DWORD RecvDataThread(LPVOID lpParam)
{
	TransferResult_t RecvRes;
	client *user;
	enum server val;
	char *ptr=NULL,*next_ptr=NULL,*username=NULL,*my_move=NULL,*opponent_move=NULL,*winner=NULL;
	user = (client *)lpParam;
	while (1)
	{
		char *AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, m_socket);
		if (check_if_SendReceiveString( RecvRes))
		{
			ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
			return 0x555;
		}
		ptr = strtok_s(AcceptedStr, ":;", &next_ptr);
		val = Server_ReceiveString(ptr);
		switch (val) {
		case SERVER_MAIN_MENU:
			printf("Choose what to do next:\n");
			printf("1. Play against another client\n");
			printf("2. Play against the server\n");
			printf("3. View the leaderboard\n");
			printf("4. Quit\n");
			ReleaseSemaphore(SERVER_MAIN_MENU_Semphore, 1, NULL);
			free(AcceptedStr);
			break;
		case SERVER_APPROVED:
			ReleaseSemaphore(SERVER_APROVED_Semphore, 1, NULL);
			free(AcceptedStr);
			break;
		case SERVER_DENIED:
			printf("Server on %s:%d denied the connection request.\n", user->ip_adress, user->portName);
			ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
			free(AcceptedStr);
			return 0x555;
		case SERVER_INVITE:
			ReleaseSemaphore(SERVER_INVITE_NO_OPPONENTS_Semphore, 1, NULL);
			free(AcceptedStr);
			break;
		case SERVER_PLAYER_MOVE_REQUEST:
			printf("Choose a move from the list: Rock, Paper, Scissors, Lizard or Spock:\n");
			ReleaseSemaphore(SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT, 1, NULL);
			free(AcceptedStr);
			break;
		case SERVER_GAME_RESULTS:
			username= strtok_s(NULL, ":;", &next_ptr);
			opponent_move= strtok_s(NULL, ":;", &next_ptr);
			my_move= strtok_s(NULL, ":;", &next_ptr);
			printf("You played : %s\n", my_move);
			printf("%s played : %s\n", username, opponent_move);
			if (strcmp(my_move, opponent_move) != 0) {
				winner = strtok_s(NULL, ":;", &next_ptr);
				printf("%s won!\n", winner);
			}
			ReleaseSemaphore(SERVER_GAME_RESULTS_Semphore, 1, NULL);
			free(AcceptedStr);
			break;
		case SERVER_GAME_OVER_MENU:
			printf("Choose what to do next :\n");
			printf("1. Play again\n");
			printf("2. Return to the main menu\n");
			ReleaseSemaphore(SERVER_GAME_OVER_MENU_Semphore, 1, NULL);
			free(AcceptedStr);
			break;
		case SERVER_OPPONENT_QUIT:
			ServerOpponemtsQuit = 1;
			ptr= strtok_s(NULL, ":;\n", &next_ptr);
			printf("%s has left the game!\n", ptr);
			ReleaseSemaphore(SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT, 1, NULL);
			free(AcceptedStr);
			break;
		case SERVER_NO_OPPONENTS:
			ServerNoOpponemts = 1;
			ReleaseSemaphore(SERVER_INVITE_NO_OPPONENTS_Semphore, 1, NULL);
			free(AcceptedStr);
			break;
		case SERVER_LEADERBOARD:
			ptr = strtok_s(NULL, ":;", &next_ptr);
			for (int i = 0; i < strlen(ptr); i++) {
				if (ptr[i] == ',')
					printf("\t\t");
				else
					printf("%c", ptr[i]);
			}
			ReleaseSemaphore(SERVER_LEADERBOARD_Semphore, 1, NULL);
			free(AcceptedStr);
			break;
		case  SERVER_LEADERBORAD_MENU:
			printf("Choose what to do next :\n");
			printf("1. Refresh leaderboard\n");
			printf("2. Return to the main menu\n");
			ReleaseSemaphore(SERVER_LEADERBORAD_MENU_Semphore, 1, NULL);
			free(AcceptedStr);
			break;
		default:
			free(AcceptedStr);
			return 0x555;
		}
	}
	return 0;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Sending data to the server
static DWORD SendDataThread(LPVOID lpParam)
{
	char massage[LONGEST_MESSAGE], SendStr[MAX_LENGTH];
	TransferResult_t SendRes;
	DWORD wait_res;
	client *user;
	int main_menu = 0;
	user = (client *)lpParam;
	strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_REQUEST:");
	strcat_s(massage, LONGEST_MESSAGE, user->usrename);
	strcat_s(massage, LONGEST_MESSAGE, "\n");
	SendRes = SendString(massage, m_socket);
	if (check_if_SendReceiveString(SendRes))
	{
		ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
		return 0x555;
	}
	wait_res = WaitForSingleObject(SERVER_APROVED_Semphore, WAIT_15S);
	if ((WAIT_OBJECT_0 != wait_res))
	{
		printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
		ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
		return 0x555;
	}
	while (1)
	{
		wait_res = WaitForSingleObject(SERVER_MAIN_MENU_Semphore, WAIT_15S);
		if ((WAIT_OBJECT_0 != wait_res))
		{
			printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
			ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
			return 0x555;
		}
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from SERVER_MAIN_MENU
		if (atoi(SendStr) == 1) {//Play against another client
			strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_VERSUS\n");
			SendRes = SendString(massage, m_socket);
			if (check_if_SendReceiveString(SendRes))
			{
				ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
				return 0x555;
			}
			while (1) {
				wait_res = WaitForSingleObject(SERVER_INVITE_NO_OPPONENTS_Semphore, WAIT_30S);
				if ((WAIT_OBJECT_0 != wait_res))
				{
					printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
					ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
					return 0x555;
				}
				if (ServerNoOpponemts) {
					ServerNoOpponemts = 0;
					break;
				}
				while (1) {
					wait_res = WaitForSingleObject(SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT, WAIT_30S);
					if ((WAIT_OBJECT_0 != wait_res))
					{
						printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
						ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
						return 0x555;
					}
					if(ServerOpponemtsQuit)
						break;
					gets_s(SendStr, sizeof(SendStr)); //Reading a string from SERVER_PLAYER_MOVE_REQUEST
					strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_PLAYER_MOVE:");
					_strupr_s(SendStr, LONGEST_MESSAGE);
					strcat_s(massage, LONGEST_MESSAGE, SendStr);
					strcat_s(massage, LONGEST_MESSAGE, "\n");
					SendRes = SendString(massage, m_socket);
					if (check_if_SendReceiveString(SendRes))
					{
						ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
						return 0x555;
					}
					wait_res = WaitForSingleObject(SERVER_GAME_RESULTS_Semphore, WAIT_30S);
					if ((WAIT_OBJECT_0 != wait_res))
					{
						printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
						ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
						return 0x555;
					}
					wait_res = WaitForSingleObject(SERVER_GAME_OVER_MENU_Semphore, WAIT_30S);
					if ((WAIT_OBJECT_0 != wait_res))
					{
						printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
						ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
						return 0x555;
					}
					gets_s(SendStr, sizeof(SendStr)); //Reading a string from SERVER_GAME_OVER_MENU
					if (atoi(SendStr) == 1) {//Play again
						strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_REPLAY\n");
						SendRes = SendString(massage, m_socket);
						if (check_if_SendReceiveString(SendRes))
						{
							ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
							return 0x555;
						}
					}
					else {//Return to the main menu
						strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_MAIN_MENU\n");
						SendRes = SendString(massage, m_socket);
						if (check_if_SendReceiveString(SendRes))
						{
							ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
							return 0x555;
						}
						main_menu = 1;
						break;
					}
				}
				if (ServerOpponemtsQuit|| main_menu) {
					main_menu = 0;
					ServerOpponemtsQuit = 0;
					break;
				}
			}
		}
		else if (atoi(SendStr) == 2) {//Play against the server
			strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_CPU\n");
			SendRes = SendString(massage, m_socket);
			if (check_if_SendReceiveString(SendRes))
			{
				ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
				return 0x555;
			}
			while (1) {
					wait_res = WaitForSingleObject(SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT, WAIT_15S);
					if ((WAIT_OBJECT_0 != wait_res))
					{
						printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
						ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
						return 0x555;
					}
					gets_s(SendStr, sizeof(SendStr)); //Reading a string from SERVER_PLAYER_MOVE_REQUEST
					strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_PLAYER_MOVE:");
					_strupr_s(SendStr, LONGEST_MESSAGE);
					strcat_s(massage, LONGEST_MESSAGE, SendStr);
					strcat_s(massage, LONGEST_MESSAGE,"\n");
					SendRes = SendString(massage, m_socket);
					if (check_if_SendReceiveString(SendRes))
					{
						ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
						return 0x555;
					}
					wait_res = WaitForSingleObject(SERVER_GAME_RESULTS_Semphore, WAIT_15S);
					if ((WAIT_OBJECT_0 != wait_res))
					{
						printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
						ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
						return 0x555;
					}
					wait_res = WaitForSingleObject(SERVER_GAME_OVER_MENU_Semphore, WAIT_15S);
					if ((WAIT_OBJECT_0 != wait_res))
					{
						printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
						ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
						return 0x555;
					}
					gets_s(SendStr, sizeof(SendStr)); //Reading a string from SERVER_GAME_OVER_MENU
					if (atoi(SendStr) == 1) {//Play again
						strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_REPLAY\n");
						SendRes = SendString(massage, m_socket);
						if (check_if_SendReceiveString(SendRes))
						{
							ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
							return 0x555;
						}
					}
					else {//Return to the main menu
						strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_MAIN_MENU\n");
						SendRes = SendString(massage, m_socket);
						if (check_if_SendReceiveString(SendRes))
						{
							ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
							return 0x555;
						}
						break;
					}
				
			}
		}
		else if (atoi(SendStr) == 3) {//View the leaderboard
			strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_LEADERBOARD\n");
			SendRes = SendString(massage, m_socket);
			if (check_if_SendReceiveString(SendRes))
			{
				ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
				return 0x555;
			}
			while (1) {
				wait_res = WaitForSingleObject(SERVER_LEADERBOARD_Semphore, WAIT_15S);
				if ((WAIT_OBJECT_0 != wait_res))
				{
					printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
					ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
					return 0x555;
				}
				wait_res = WaitForSingleObject(SERVER_LEADERBORAD_MENU_Semphore, WAIT_15S);
				if ((WAIT_OBJECT_0 != wait_res))
				{
					printf("Connection to server on %s:%d has been lost.\n", user->ip_adress, user->portName);
					ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
					return 0x555;
				}
				gets_s(SendStr, sizeof(SendStr)); //Reading a string from SERVER_LEADERBORAD_MENU
				if (atoi(SendStr) == 1) {//Refresh leaderboard
					strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_REFRESH\n");
					SendRes = SendString(massage, m_socket);
					if (check_if_SendReceiveString(SendRes))
					{
						ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
						return 0x555;
					}
				}
				else {//Return to the main menu
					strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_MAIN_MENU\n");
					SendRes = SendString(massage, m_socket);
					if (check_if_SendReceiveString(SendRes))
					{
						ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
						return 0x555;
					}
					break;
				}
			}
		}
		else {//Quit
			TimeTogoOut = 1;
			strcpy_s(massage, LONGEST_MESSAGE, "CLIENT_DISCONNECT\n");
			SendRes = SendString(massage, m_socket);
			ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
			return 0x555;
		}
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

int main(int argc, char ** argv)
{
	SOCKADDR_IN clientService;
	HANDLE hThread[2];
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	DWORD wait_res;
	int check;
	client user;

	//create user for the send and recive
	strcpy_s(user.ip_adress, MAX_LENGTH_IP_ADRESS, argv[1]);
	sscanf_s(argv[2], "%d", &user.portName);
	strcpy_s(user.usrename, MAX_LENGTH, argv[3]);

	// create all Semphores that we need
	if (CreateAllSemphores() == STATUS_CODE_FAILURE)
		return STATUS_CODE_FAILURE;
	
	//The WSADATA structure contains information about the Windows Sockets implementation.
	while (1) {
		//Call WSAStartup and check for errors.
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != NO_ERROR)
			printf("Error at WSAStartup()\n");

		//Call the socket function and return its value to the m_socket variable. 
		// For this application, use the Internet address family, streaming sockets, and the TCP/IP protocol.

		// Create a socket.
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		// Check for errors to ensure that the socket is a valid socket.
		if (m_socket == INVALID_SOCKET) {
			printf("Error at socket(): %ld\n", WSAGetLastError());
			WSACleanup();
			return;
		}

		//Create a sockaddr_in object clientService and set  values.
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = inet_addr(user.ip_adress);//Setting the IP address to connect to
		clientService.sin_port = htons(user.portName); //Setting the port to connect to.

		// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
		// Check for general errors.
		if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
			printf("Failed connecting to server on %s:%d\n", user.ip_adress, user.portName);
			check = What_to_do();
			while (check == notValid) {/// maybe not need  
				check = What_to_do();
			}
			if (check == endofwork)
				goto Exit_Out;
			continue;/// that meen that he want to try to connect agin 
		}
		printf("Connected to server on %s:%d\n", user.ip_adress, user.portName);

		// Send and receive data.
		hThread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendDataThread, &user, 0, NULL);
		if (NULL == hThread[0])
		{
			printf("Couldn't create threadSendData\n");
			printf("Connection to server on %s:%d has been lost.\n", user.ip_adress, user.portName);
			ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
		}
		hThread[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvDataThread, &user, 0, NULL);
		if (NULL == hThread[1])
		{
			printf("Couldn't create threadRecvData\n");
			printf("Connection to server on %s:%d has been lost.\n", user.ip_adress, user.portName);
			ReleaseSemaphore(TimeOut_conctionProblem_GoOut_Semphore, 1, NULL);
		}

		wait_res = WaitForSingleObject(TimeOut_conctionProblem_GoOut_Semphore, INFINITE);
		if ((WAIT_OBJECT_0 != wait_res))
		{
			printf("Couldn't wait for end of TimeOut_conctionProblem_GoOut_Semphore\n");
		}
		TerminateThread(hThread[0], 0x555);
		TerminateThread(hThread[1], 0x555);
		CloseHandle(hThread[0]);
		CloseHandle(hThread[1]);
		closesocket(m_socket);
		WSACleanup();
		if (TimeTogoOut)// if he want to disconect
			goto Go_Out;
		/// if the time_out or concection problem 
		check = What_to_do();
		while (check == notValid) {/// maybe not need  
			check = What_to_do();
		}
		if (check == endofwork)
			goto Go_Out;
	}

Exit_Out:
	WSACleanup();
	closesocket(m_socket);
	closeAllHandle();
	return STATUS_CODE_SUCCESS;
Go_Out:
	closeAllHandle();
	return STATUS_CODE_SUCCESS;
}

int What_to_do(){
	char masage[MAX_LENGTH];
	printf("Choose what to do next:\n");
	printf("1. Try to reconnect\n");
	printf("2. Exit\n");
	gets_s(masage, sizeof(masage)); //Reading a string from the keyboard
	switch (atoi(masage)) {
	case (1):
		return again;
	case (2):
		return endofwork;
	default: // CASE OF ILLEGAL COMMAND
		printf("not prmited value");
		return notValid;
	} // End of switch 
}
int check_if_SendReceiveString(TransferResult_t RecvRes) {
	if (RecvRes == TRNS_FAILED|| RecvRes == TRNS_DISCONNECTED)
	{
		return TRUE;
	}
	return FALSE;
}

int str_prefix(char *str, const char *prefix)
{
	return (strncmp(prefix, str, strlen(prefix)) == 0);
}

	

int Server_ReceiveString(char *ptr) {
	if (str_prefix(ptr, "SERVER_MAIN_MENU") == 1)
		return SERVER_MAIN_MENU;
	if (str_prefix(ptr, "SERVER_APPROVED") == 1)
		return SERVER_APPROVED;
	if (str_prefix(ptr, "SERVER_DENIED") == 1)
		return SERVER_DENIED;
	if (str_prefix(ptr, "SERVER_INVITE"))
		return SERVER_INVITE;
	if (str_prefix(ptr, "SERVER_PLAYER_MOVE_REQUEST"))
		return SERVER_PLAYER_MOVE_REQUEST;
	if (str_prefix(ptr, "SERVER_GAME_RESULTS"))
		return SERVER_GAME_RESULTS;
	if (str_prefix(ptr, "SERVER_GAME_OVER_MENU"))
		return SERVER_GAME_OVER_MENU;
	if (str_prefix(ptr, "SERVER_OPPONENT_QUIT"))
		return SERVER_OPPONENT_QUIT;
	if (str_prefix(ptr, "SERVER_NO_OPPONENTS"))
		return SERVER_NO_OPPONENTS;
	if (str_prefix(ptr, "SERVER_LEADERBOARD"))
		return SERVER_LEADERBOARD;
	return SERVER_LEADERBORAD_MENU;
}
int CreateAllSemphores() {
	SERVER_APROVED_Semphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == SERVER_APROVED_Semphore)
	{
		printf("Error when creating semaphore: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	TimeOut_conctionProblem_GoOut_Semphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == TimeOut_conctionProblem_GoOut_Semphore)
	{
		CloseHandle(SERVER_APROVED_Semphore);
		printf("Error when creating semaphore: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	SERVER_MAIN_MENU_Semphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == SERVER_MAIN_MENU_Semphore)
	{
		CloseHandle(TimeOut_conctionProblem_GoOut_Semphore);
		CloseHandle(SERVER_APROVED_Semphore);
		printf("Error when creating semaphore: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	SERVER_INVITE_NO_OPPONENTS_Semphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == SERVER_INVITE_NO_OPPONENTS_Semphore)
	{
		CloseHandle(TimeOut_conctionProblem_GoOut_Semphore);
		CloseHandle(SERVER_APROVED_Semphore);
		CloseHandle(SERVER_MAIN_MENU_Semphore);
		printf("Error when creating semaphore: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT)
	{
		CloseHandle(TimeOut_conctionProblem_GoOut_Semphore);
		CloseHandle(SERVER_APROVED_Semphore);
		CloseHandle(SERVER_MAIN_MENU_Semphore);
		CloseHandle(SERVER_INVITE_NO_OPPONENTS_Semphore);
		printf("Error when creating semaphore: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	SERVER_GAME_RESULTS_Semphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == SERVER_GAME_RESULTS_Semphore)
	{
		CloseHandle(TimeOut_conctionProblem_GoOut_Semphore);
		CloseHandle(SERVER_APROVED_Semphore);
		CloseHandle(SERVER_MAIN_MENU_Semphore);
		CloseHandle(SERVER_INVITE_NO_OPPONENTS_Semphore);
		CloseHandle(SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT);
		printf("Error when creating semaphore: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	SERVER_GAME_OVER_MENU_Semphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == SERVER_GAME_OVER_MENU_Semphore)
	{
		CloseHandle(TimeOut_conctionProblem_GoOut_Semphore);
		CloseHandle(SERVER_APROVED_Semphore);
		CloseHandle(SERVER_MAIN_MENU_Semphore);
		CloseHandle(SERVER_INVITE_NO_OPPONENTS_Semphore);
		CloseHandle(SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT);
		CloseHandle(SERVER_GAME_RESULTS_Semphore);
		printf("Error when creating semaphore: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	SERVER_LEADERBOARD_Semphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == SERVER_LEADERBOARD_Semphore)
	{
		CloseHandle(TimeOut_conctionProblem_GoOut_Semphore);
		CloseHandle(SERVER_APROVED_Semphore);
		CloseHandle(SERVER_MAIN_MENU_Semphore);
		CloseHandle(SERVER_INVITE_NO_OPPONENTS_Semphore);
		CloseHandle(SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT);
		CloseHandle(SERVER_GAME_RESULTS_Semphore);
		CloseHandle(SERVER_GAME_OVER_MENU_Semphore);
		printf("Error when creating semaphore: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	SERVER_LEADERBORAD_MENU_Semphore = CreateSemaphore(NULL, 0, 1, NULL);
	if (NULL == SERVER_LEADERBORAD_MENU_Semphore)
	{
		CloseHandle(TimeOut_conctionProblem_GoOut_Semphore);
		CloseHandle(SERVER_APROVED_Semphore);
		CloseHandle(SERVER_MAIN_MENU_Semphore);
		CloseHandle(SERVER_INVITE_NO_OPPONENTS_Semphore);
		CloseHandle(SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT);
		CloseHandle(SERVER_GAME_RESULTS_Semphore);
		CloseHandle(SERVER_GAME_OVER_MENU_Semphore);
		CloseHandle(SERVER_LEADERBOARD_Semphore);
		printf("Error when creating semaphore: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
void closeAllHandle() {
	CloseHandle(TimeOut_conctionProblem_GoOut_Semphore);
	CloseHandle(SERVER_APROVED_Semphore);
	CloseHandle(SERVER_MAIN_MENU_Semphore);
	CloseHandle(SERVER_INVITE_NO_OPPONENTS_Semphore);
	CloseHandle(SERVER_PLAYER_MOVE_REQUEST_Semphore_OPPONENT_QUIT);
	CloseHandle(SERVER_GAME_RESULTS_Semphore);
	CloseHandle(SERVER_GAME_OVER_MENU_Semphore);
	CloseHandle(SERVER_LEADERBOARD_Semphore);
	CloseHandle(SERVER_LEADERBORAD_MENU_Semphore);
}

