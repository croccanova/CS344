/***********************************
* Christian Roccanova
* CS 344 - 400
* Summer 2018
* Project 4
* Description: sends requests to server to decrypt text
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


// opens file and checks that the text is valid
int checkFile(char fileName[70000], char buffer[70000], char message[70000])
{
	int i;
	FILE* myFile;
	
	// clear memory
	memset(message, '\0', 70000);
	memset(buffer, '\0', 70000);

	// opens the file
	myFile = fopen(fileName, "r");

	// check if file can be opened
	if (!(myFile))
	{
		// put error message into message 
		sprintf(message, "ERROR could not open '%s'\n", fileName);

		// close the file
		fclose(myFile);
		return 1;
	}

	// read text
	fgets(buffer, 70000, myFile);

	// remove newline character
	strtok(buffer, "\n");

	// checks if file contains invalid characters
	for (i = 0; i < strlen(buffer); i++)
	{
		// checks characters other than "ABCDEFGHIJKLMNOPQRSTUVWXYZ " are present
		if ((buffer[i] < 65 || buffer[i] > 90) && buffer[i] != 32)
		{
			// put error message into message 
			sprintf(message, "ERROR invalid characters found in '%s'\n", fileName);

			// close the file
			fclose(myFile);
			return 1;
		}
	}

	// close the file
	fclose(myFile);
	return 0;
}


int main(int argc, char *argv[])
{
	int socketFD, portNum;
	struct sockaddr_in server;
	struct hostent *serverHostInfo;
	char buffer[70000];
	char message[70000];
	char textFile[70000];
	char keyFile[70000];
	char fileText[70000];
	char keyText[70000];

	// special character for end of text
	char endChar = '*';

	// check number of arguments
	if (argc < 3)
	{

		// send error to stderr
		fprintf(stderr, "ERROR too few arguments\n");
		exit(1);
	}

	// gets values from arguments
	strcpy(textFile, argv[1]);
	strcpy(keyFile, argv[2]);
	portNum = atoi(argv[3]);

	// check if textFile is valid
	if (checkFile(textFile, fileText, message) != 0)
	{
		// send error to stderr
		fprintf(stderr, "%s", message);

		exit(1);
	}

	// check if keyFile is valid 
	if (checkFile(keyFile, keyText, message) != 0)
	{
		// send error to stderr
		fprintf(stderr, "%s", message);

		exit(1);
	}

	// compare key length and text length
	if (strlen(fileText) > strlen(keyText))
	{
		//sprintf(message, "ERROR key is too short\n", keyFile);

		// send error to stderr
		fprintf(stderr, "ERROR key is too short\n");

		exit(1);
	}

	// gets the host
	serverHostInfo = gethostbyname("localhost");

	// clear memory
	memset(&server, '\0', sizeof(server));
	
	server.sin_family = AF_INET;  // Create a network-capable socket
	server.sin_port = htons(portNum);  // Store the port number
	memcpy(&server.sin_addr, serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	
	// creates the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);

	// check if socket was created
	if (socketFD < 0)
	{
		// send error to stderr
		fprintf(stderr, "ERROR opening socket\n");

		exit(2);
	}

	// connects to the server
	if (connect(socketFD, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		// send error to stderr
		fprintf(stderr, "ERROR connecting\n");

		exit(2);
	}

	// tell server that otp_dec is sending the request
	strcpy(buffer, "otp_dec");
	writeStuff(socketFD, buffer, &endChar);

	// checks if the client and server dont match
	if (strncmp(buffer, "ERROR otp_enc cannot use otp_dec_d", 35) == 0)
	{
		// send error to stderr
		fprintf(stderr, "%s", buffer);

		exit(2);
	}

	// sends the file text to server
	writeStuff(socketFD, fileText, &endChar);

	// send key text to server
	writeStuff(socketFD, keyText, &endChar);

	// read server response
	readStuff(socketFD, buffer, &endChar);
	
	// print the buffer
	printf("%s\n", buffer);

	// close the socket
	close(socketFD);
	exit(0);
}