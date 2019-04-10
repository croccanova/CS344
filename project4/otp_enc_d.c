/***********************************
* Christian Roccanova
* CS 344 - 400
* Summer 2018
* Project 4
* Description: receives requests from client to encrypt text
************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>


// writes to socket using '*' as an end character to denote end of text
void writeStuff(int socketFD, char buffer[70000], char* endChar)
{
	int n;
	int count = 0;

	// write the text
	while (count < strlen(buffer))
	{
		n = write(socketFD, &buffer[count], strlen(buffer) - count);

		// count of characters written
		count += n;

	}

	// writes the end character
	n = write(socketFD, endChar, 1);

}


// reads text in one character at a time until the end character is reached
void readStuff(int socketFD, char buffer[70000], char* endChar)
{
	int n;
	int j = 0;
	char myChar;

	// clears the memory of the buffer
	memset(buffer, '\0', 70000);

	// reads the text
	do {
		// read a character
		n = read(socketFD, &myChar, 1);

		// loop breaks when the special end character is read
		if (myChar == *endChar)
			break;

		// add myChar to buffer
		buffer[j] = myChar;

		// increment position in buffer
		j++;
	} while (1);

}


// converts characters into their index numbers 0-26 per the array in keygen.c
// array for reference: "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
int charToNum(char myChar)
{
	// myChar is A - Z
	if (myChar <= 90 && myChar >= 65)
	{
		return myChar - 65;
	}
	// myChar is ' '
	else
	{
		return 26;
	}
}



// converts index numbers 0-26 into the corresponding characters per the array in keygen.c
// array for reference: "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
char numToChar(int myNum)
{

	// myNum is A - Z
	if (myNum >= 0 && myNum < 26)
	{
		// returns the characters ASCII code
		return myNum + 65;
	}
	// myNum is ' '
	else
	{
		// directly returns space as it is the only option
		return 32;
	}
}


/*********************************************************************
** Function: Encrypt()
** Description: Encrypts text with provided key.
*********************************************************************/
void encrypt(char textIn[70000], char textOut[70000], char key[70000])
{
	int i;
	int encrypted;
	int textNum;
	int keyNum;
	
	// clear memory of the outgoing text
	memset(textOut, '\0', 70000);

	// encrypt the text
	for (i = 0; i < strlen(textIn); i++)
	{
		// convert characters to numbers for encryption
		textNum = charToNum(textIn[i]);
		keyNum = charToNum(key[i]);		
		
		// encrypt
		encrypted = (textNum + keyNum) % 27;
		
		// add encrypted character to the outgoing text
		textOut[i] = numToChar(encrypted);
	}
}


int main(int argc, char *argv[])
{
	int socketFD, clientSocket, portNum;
	struct sockaddr_in server, client;
	int childPid;
	int childStatus;
	char message[70000];
	char buffer[70000];
	char key[70000];
	char textIn[70000];
	char textOut[70000];

	// special character for end of text
	char endChar = '*';

	// check number of arguments
	if (argc < 2)
	{
		// send error to stderr
		fprintf(stderr, "ERROR: too few arguments\n");

		exit(1);
	}

	// clear memory
	memset(&server, '\0', sizeof(server));

	// set port number
	portNum = atoi(argv[1]);

	server.sin_family = AF_INET;  // Create a network-capable socket
	server.sin_port = htons(portNum);  // Store the port number
	server.sin_addr.s_addr = INADDR_ANY;  // Any address is allowed for connection to this process
	
	// creates the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);

	// check if socket was created
	if (socketFD < 0)
	{
		// send error to stderr
		fprintf(stderr, "ERROR opening socket\n");

		exit(1);
	}
		
	// binds port to socket
	if (bind(socketFD, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		// send error to stderr
		fprintf(stderr, "ERROR on binding\n");

		exit(1);
	}

	// listen for requests to a maximum of 5
	listen(socketFD, 5);

	// loop to accept requests
	while (1)
	{
		// clean up finished child processes
		do {
			childPid = waitpid(-1, &childStatus, WNOHANG);
		} while (childPid > 0);

		// clear memory
		memset(message, '\0', 70000);

		// accept connection
		clientSocket = accept(socketFD, NULL, NULL);

		// check if client was accepted
		if (clientSocket < 0)
		{
			// send error to stderr
			fprintf(stderr, "ERROR on accept\n");

			exit(1);
		}

		// fork a new process
		pid_t spawnpid = fork();

		// child process
		if (spawnpid == 0)
		{
			// make sure that request is not from otp_dec
			readStuff(clientSocket, buffer, &endChar);			
			if (strcmp(buffer, "otp_dec") == 0)
			{
				// send back error message
				sprintf(message, "ERROR otp_dec cannot use otp_enc_d\n");
				writeStuff(clientSocket, message, &endChar);

				exit(1);
			}

			// reads the text
			readStuff(clientSocket, textIn, &endChar);

			// reads the key
			readStuff(clientSocket, key, &endChar);

			// encrypts the text
			encrypt(textIn, textOut, key);

			// sends back encrypted text
			writeStuff(clientSocket, textOut, &endChar);
			break;
		}
	}

	// close sockets
	close(clientSocket);
	close(socketFD);
	exit(0);
}