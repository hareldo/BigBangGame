#include "Server.h"


#define SERVER_ADDRESS_STR "127.0.0.1"
#define SERVER_PORT 8888

int main(int argc, char * argv[])
{
	SOCKET server_socket = INVALID_SOCKET;

	if (argc < 2) {
		printf("Not enough arguments.\n");
		return STATUS_CODE_FAILURE;
	}

	if (InitServerSocket(&server_socket) == STATUS_CODE_FAILURE)
		return STATUS_CODE_FAILURE;

}

TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd)
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;

	while (RemainingBytesToSend > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesTransferred = send(sd, CurPlacePtr, RemainingBytesToSend, 0);
		if (BytesTransferred == SOCKET_ERROR)
		{
			printf("send() failed, error %d\n", WSAGetLastError());
			return TRNS_FAILED;
		}

		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

TransferResult_t SendString(const char *Str, SOCKET sd)
{
	/* Send the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t SendRes;

	/* The request is sent in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	TotalStringSizeInBytes = (int)(strlen(Str) + 1); // terminating zero also sent	

	SendRes = SendBuffer(
		(const char *)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // sizeof(int) 
		sd);

	if (SendRes != TRNS_SUCCEEDED) return SendRes;

	SendRes = SendBuffer(
		(const char *)(Str),
		(int)(TotalStringSizeInBytes),
		sd);

	return SendRes;
}

TransferResult_t ReceiveBuffer(char* OutputBuffer, int BytesToReceive, SOCKET sd)
{
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;

	while (RemainingBytesToReceive > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
		if (BytesJustTransferred == SOCKET_ERROR)
		{
			printf("recv() failed, error %d\n", WSAGetLastError());
			return TRNS_FAILED;
		}
		else if (BytesJustTransferred == 0)
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd)
{
	/* Recv the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t RecvRes;
	char* StrBuffer = NULL;

	if ((OutputStrPtr == NULL) || (*OutputStrPtr != NULL))
	{
		printf("The first input to ReceiveString() must be "
			"a pointer to a char pointer that is initialized to NULL. For example:\n"
			"\tchar* Buffer = NULL;\n"
			"\tReceiveString( &Buffer, ___ )\n");
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	RecvRes = ReceiveBuffer(
		(char *)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // 4 bytes
		sd);

	if (RecvRes != TRNS_SUCCEEDED) return RecvRes;

	StrBuffer = (char*)malloc(TotalStringSizeInBytes * sizeof(char));

	if (StrBuffer == NULL)
		return TRNS_FAILED;

	RecvRes = ReceiveBuffer(
		(char *)(StrBuffer),
		(int)(TotalStringSizeInBytes),
		sd);

	if (RecvRes == TRNS_SUCCEEDED)
	{
		*OutputStrPtr = StrBuffer;
	}
	else
	{
		free(StrBuffer);
	}

	return RecvRes;
}



static DWORD RecieveDataFromClient(SOCKET *server_socket) {
	char SendStr[SEND_STR_MAX_SIZE];

	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	while (!Done)
	{
		char *AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, *server_socket);

		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			DeinitializeSocket(server_socket);
			return STATUS_CODE_FAILURE;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			DeinitializeSocket(server_socket);
			return STATUS_CODE_FAILURE;
		}
		else
		{
			printf("Got string : %s\n", AcceptedStr);
		}

		//After reading a single line, checking to see what to do with it
		//If got "hello" send back "what's up?"
		//If got "how are you?" send back "great"
		//If got "bye" send back "see ya!" and then end the thread
		//Otherwise, send "I don't understand"

		if (STRINGS_ARE_EQUAL(AcceptedStr, "hello"))
		{
			strcpy(SendStr, "what's up?");
		}
		else if (STRINGS_ARE_EQUAL(AcceptedStr, "how are you?"))
		{
			strcpy(SendStr, "great");
		}
		else if (STRINGS_ARE_EQUAL(AcceptedStr, "bye"))
		{
			strcpy(SendStr, "see ya!");
			Done = TRUE;
		}
		else
		{
			strcpy(SendStr, "I don't understand");
		}

		SendRes = SendString(SendStr, *t_socket);

		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			DeinitializeSocket(server_socket);
			return STATUS_CODE_FAILURE;
		}

		free(AcceptedStr);
	}

	DeinitializeSocket(server_socket);
	return STATUS_CODE_SUCCESS;
}

int InitServerSocket(SOCKET *server_socket) {
	// Initialize Winsock.
	WSADATA wsa_data;
	unsigned long address;
	SOCKADDR_IN service;
	int status;

	status = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (status != 0) {
		printf("WSAStartup failed: %d\n", status);
		return STATUS_CODE_FAILURE;
	}
	// Create a socket.    
	*server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*server_socket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		DeinitializeSocket(NULL);
		return STATUS_CODE_FAILURE;
	}


	address = inet_addr(SERVER_ADDRESS_STR);
	if (address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		DeinitializeSocket(*server_socket);
		return STATUS_CODE_FAILURE;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = address;
	service.sin_port = htons(SERVER_PORT);

	//Call bind
	status = bind(*server_socket, (SOCKADDR*)&service, sizeof(service));
	if (status == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		DeinitializeSocket(*server_socket);
		return STATUS_CODE_FAILURE;
	}

	// Listen on the Socket.
	status = listen(*server_socket, SOMAXCONN);
	if (status == SOCKET_ERROR) {
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		DeinitializeSocket(*server_socket);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}

int DeinitializeSocket(SOCKET *socket) {
	int result;

	if (*socket != NULL) {
		if (closesocket(*socket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		return STATUS_CODE_FAILURE;
	}

	result = WSACleanup();
	if (result != 0) {
		printf("WSACleanup failed: %d\n", result);
		return STATUS_CODE_FAILURE;
	}
}