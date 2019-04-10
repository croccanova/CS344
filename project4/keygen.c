/***********************************
* Christian Roccanova
* CS 344 - 400 
* Summer 2018
* Project 4
* Description: Randomly generates keys of a specified length
************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[])
{
	// seed random generator
	srand(time(NULL));

	// potential character A - Z and " "
	char chars[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	int randNum;
	int keyLen;
	int i;

	// atoi converts string argument into an integer
	keyLen = atoi(argv[1]);
	
	for (i = 0; i < keyLen; i++)
	{
		// generate random number to determine character
		randNum = rand() % 27;

		// output corresponding random character
		printf("%c", chars[randNum]);

	}

	// output trailing newline
	printf("\n");

	return 0;
}