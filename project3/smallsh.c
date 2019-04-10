/*
*	Christian Roccanova
*	CS344 - 400 - Summer 2018
*	Program 3
*	Creates a shell to take user input to run various commands including cd, status and exit.  Printf output designed to match that in the given example in the assignment instructions.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

// flags for various inputs 0 = false, 1 = true
int cdFlag = 0; // change directory
int specDirFlag = 0; // change to a specified directory
int statusFlag = 0; // output status
int exitFlag = 0; // exit
int commentFlag = 0; // comment
int echoFlag = 0; // echo command
int rd_stdoutFlag = 0; // redirect standard out
int rd_stdinFlag = 0; // redirect standard in
int bgFlag = 0; // background 
int commandFlag = 0; // there is a command					 
int fgOnlyFlag = 0; // shell is in foreground-only mode

// holds a line of user input
char userInput[2048];

// holds arguments from user input
char* args[512];

// user specified directory
char directory[500];
					
// child process exit status
int childExitMethod = -5;

// file for stdin redirection
char stdinFile[100];

// file for stdout redirection
char stdoutFile[100];

// holds a user input command 
char command[25];

// number of arguments given with a command
int argCount = 0;

// command argument1

char arg1[1024];
char arg2[1024];

// holds PIDs of unfinished processes
pid_t unfinishedProcesses[200];

// counter for unfinished processes array
int unfinishedCount = 0;


// resets certain flags to false state
void resetFlags()
{	
	
	cdFlag = 0;
	specDirFlag = 0;
	statusFlag = 0;
	exitFlag = 0;
	echoFlag = 0;
	rd_stdinFlag = 0;
	rd_stdoutFlag = 0;
	bgFlag = 0;
	commentFlag = 0;
	commandFlag = 0;
		
	// also resets argument counter
	argCount = 0;
}

// redirects stdin, code as seen in "More UNIX I/O" lecture
void rd_stdin()
{
	int sourceFD;

	// open target file
	sourceFD = open(stdinFile, O_RDONLY);

	// file failed to open
	if (sourceFD == -1)
	{
		printf("cannot open %s for input\n", stdinFile);
		fflush(stdout);
		exit(1);
	}

	// redirect stdin to the target file
	int result = dup2(sourceFD, 0);

	// redirect failed
	if (result == -1)
	{
		perror("dup2");
		exit(2);
	}
}

// redirects stdout, code as seen in "More UNIX I/O" lecture
void rd_stdout()
{
	int targetFD;

	// open target file
	targetFD = open(stdoutFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	// file failed to open
	if (targetFD == -1)
	{
		perror("open()");
		exit(1);
	}

	// redirect stdout to the target file
	int result = dup2(targetFD, 1);

	// redirect failed
	if (result == -1)
	{
		perror("dup2");
		exit(1);
	}
}

// executes using execlp() based on number of arguments that were input
void execute()
{
	// 0 arguments
	if(argCount == 0)
	{
		execlp(command, command, NULL);
		printf("%s: no such file or directory\n", command);
		fflush(stdout);
		exit(1);
	}
	// 1 argument
	else if (argCount == 1)
	{
		execlp(command, command, arg1, NULL);
		exit(1);
	}
	// 2 arguments
	else if (argCount == 2)
	{
		execlp(command, command, arg1, arg2, NULL);
		exit(1);
	}


}

void forkIt()
{
		// set the pid to a junk value
		pid_t spawnPID = -5;

		// get the child PID
		spawnPID = fork();		
		
		// output error if fork failed
		if(spawnPID == -1)
		{
			perror("Hull Breach!\n");
			exit(1);			
		}
		// child process
		else if (spawnPID == 0)
		{


			//foreground children are to terminate themselves when they receive SIGINT
			if (!bgFlag)
			{				
				struct sigaction SIGINT_action = { 0 };
				SIGINT_action.sa_handler = SIG_DFL;
				sigaction(SIGINT, &SIGINT_action, NULL);
			}

			// redirect stdin
			if (rd_stdinFlag)
			{
				rd_stdin();
			}

			// redirect stdout
			if (rd_stdoutFlag)
			{
				rd_stdout();
			}

			// executes commands now that any redirection is handled
			execute();


		}		
		else
		{
			// if we are not in foreground-only mode and the background flag is set, add the process to the array of unfinished processes
			if (bgFlag && !fgOnlyFlag)
			{
				// add the background process to the array
				unfinishedProcesses[unfinishedCount] = spawnPID;
				unfinishedCount++;
				printf("background pid is %d\n", spawnPID);
				fflush(stdout);
				
			}
			else
			{
				// waits for child process to finish
				pid_t waiting = waitpid(spawnPID, &childExitMethod, 0);

				// print message if process was terminated
				if (WIFSIGNALED(childExitMethod))
				{
					printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
					fflush(stdout);
				}

				// reset the spawn pid
				spawnPID = -5;
				
			}
		}
		

}

// gets input from user using the getline() function
void getUserInput()
{
	
	// prints command line
	printf(": ");
	fflush(stdout);
		
	// setting buffer to 0 and line pointer to null causes getline to automatically allocate a buffer. source: http://man7.org/linux/man-pages/man3/getline.3.html
	size_t buffer = 0;

	// pointer for string entered in getline()
	char* lineptr = NULL;

	// gets a line of input from user
	getline(&lineptr, &buffer, stdin);

	// copy getline into userInput
	strcpy(userInput, lineptr);

	// reremoves newline character and replaces it with a null character
	userInput[strcspn(userInput, "\n")] = '\0';

}


// splits user input into commands and arguments and sets appropriate flags
void parseInput()
{
		// end of the user input string
		int stringEnd = 0;

		// finds the end of the user input string
		while (userInput[stringEnd] != '\0')
		{
			stringEnd++;
		}

		// if the user input $$, replace it with PID
		if (userInput[stringEnd - 1] == '$' && userInput[stringEnd - 2] == '$')
		{
			// get the process id
			pid_t myPid = getpid();
			char pidNum[25];
			
			// make process id into a string
			sprintf(pidNum, "%d", myPid);

			// replace the $$ with the process id
			userInput[stringEnd - 1] = '\0';
			userInput[stringEnd - 2] = '\0';
			strcat(userInput, pidNum);
		}


		// temp string to be used with strtok
		char userStr[2048];

		// copies userInput into temp
		strcpy(userStr, userInput);

		// get a token
		char *token = strtok(userStr, " ");

		// get all arguments (separated by a space) in temp
		int i = 0;
		while (token != NULL)
		{
			strcpy(args[i], token);
			token = strtok(NULL, " ");
			i++;
		}

		// if first char is '#', its a comment
		if (userInput[0] == '#')
		{
			commentFlag = 1;
		}

		// loops through array of arguments, setting flags if appropriate
		i = 0;
		while (strcmp(args[i], "\0") != 0)
		{
			
			// user input cd
			if (strcmp(args[i], "cd") == 0)
			{
				cdFlag = 1;

				// if there are arguments, then the user input a specific directory
				if (strcmp(args[i + 1], "") != 0)
				{
					specDirFlag = 1;
					strcpy(directory, args[i + 1]);
				}
			}
			// user input status
			else if (strcmp(args[i], "status") == 0)
			{
				statusFlag = 1;
			}
			// user input exit
			else if (strcmp(args[i], "exit") == 0)
			{				
				exitFlag = 1;
			}
			// user wishes to redirect stdin
			else if (strcmp(args[i], "<") == 0)
			{
				rd_stdinFlag = 1;

				// increment i to get the next argument which will be the target file
				i++;
				strcpy(stdinFile, args[i]);
			}
			// user input echo
			else if (strcmp(args[i], "echo") == 0)
			{
				echoFlag = 1;
			}
			// user wishes to redirect stdout
			else if (strcmp(args[i], ">") == 0)
			{
				rd_stdoutFlag = 1;

				// increment i to get the next argument which will be the target file
				i++;
				strcpy(stdoutFile, args[i]);
			}
			// user input & as their last character, command is to be executed in the background
			else if (strcmp(args[i], "&") == 0 && strcmp(args[i + 1], "\0") == 0)
			{
				bgFlag = 1;
			}
			i++;
		}

		
}

// uses parsed user input to build commands and their arguments
void buildCommands()
{
	// if user did not input any built in commands, redirections or echo or &, sets up command to be run
	if (!rd_stdoutFlag && !rd_stdinFlag && !exitFlag && !statusFlag && !cdFlag && !bgFlag && !echoFlag)
	{
		// sets command flag
		commandFlag = 1;

		// get the command
		strcpy(command, args[0]);

		// if there are 2 arguments
		if (strcmp(args[2], "") != 0)
		{
			// sets argument counter
			argCount = 2;

			// get the 2 arguments
			strcpy(arg1, args[1]);
			strcpy(arg2, args[2]);
		}
		// there is 1 argument
		else if (strcmp(args[1], "") != 0)
		{
			// sets argument counter
			argCount = 1;

			// get the single argument
			strcpy(arg1, args[1]);
		}
	}
	
	// checks if the user input "echo"
	if (strcmp(args[0], "echo") == 0)
	{
		// set command flag
		commandFlag = 1;

		// copy echo into command
		strcpy(command, args[0]);

		// there is only one argument in echo
		argCount = 1;

		// gets the string to be echoed
		int i = 0;
		while (userInput[i] != '\0')
		{
			// everything after the space is part of the echo argument
			arg1[i] = userInput[i + 5];
			i++;
		}
	}

	// user input has a redirection argument
	if (rd_stdoutFlag || rd_stdinFlag)
	{
		commandFlag = 1;
		strcpy(command, args[0]);
	}

	// user input sleep as a background command
	if (strcmp(args[0], "sleep") == 0 && bgFlag)
	{
		// set command flag
		commandFlag = 1;

		// copy sleep into command
		strcpy(command, args[0]);

		// set argument count
		argCount = 1;

		// get how long to sleep
		strcpy(arg1, args[1]);
	}

	// user input kill as a background command
	if (strcmp(args[0], "kill") == 0 && bgFlag)
	{
		// set command flag
		commandFlag = 1;

		// copy kill into command
		strcpy(command, args[0]);

		// set argument count
		argCount = 2;

		//set arguments
		strcpy(arg1, args[1]);
		strcpy(arg2, args[2]);
	}
}

// Checks to see if proccesses have finished
void findFinishedProcesses()
{
		
	pid_t done;
	int exitMethod;
	int i;
	
	// looks through array of unfinished processes
	for (i = 0; i < unfinishedCount; i++)
	{
		// checks if the process has finished
		done = waitpid(unfinishedProcesses[i], &exitMethod, WNOHANG);
		if (done != 0)
		{
			// print processed finished
			printf("background pid %d is done: ", unfinishedProcesses[i]);
			fflush(stdout);

			// remove it from the array and decrement the counter
			unfinishedProcesses[i] = -5;
			unfinishedCount--;

			// if process exited, print exit status
			if (WIFEXITED(exitMethod))
			{
				printf("exit value %d\n", WEXITSTATUS(exitMethod));
				fflush(stdout);
			}
			// if process terminated by a signal, print signal
			else if (WIFSIGNALED(exitMethod))
			{
				printf("terminated by signal %d\n", WTERMSIG(exitMethod));
				fflush(stdout);
			}
		}
	}
}


// changes current directory
void cdFunc()
{
	// user gives a specific directory
	if (specDirFlag)
	{
		chdir(directory);
	}
	// go to HOME directory if none is specified
	else
	{
		chdir(getenv("HOME"));
	}
}

// prints exit status or termination signal
void statusFunc()
{
	// if process exited, print exit status
	if (WIFEXITED(childExitMethod))
	{
		printf("exit value %d\n", WEXITSTATUS(childExitMethod));
		fflush(stdout);
	}
	// if process terminated by a signal, print signal
	else if (WIFSIGNALED(childExitMethod))
	{
		printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
		fflush(stdout);
	}
}

// kills unfinished processes prior to exiting
void exitFunc()
{
	int i;

	// loop through unfinished processes
	for (i = 0; i < unfinishedCount; i++)
	{
		// kills any unfinished processes
		if (unfinishedProcesses[i] != -5)
		{
			kill(unfinishedProcesses[i], SIGKILL);
		}
	}
}

// Catches Ctrl-Z, based on example from Signals lecture slide "Catching & Ignoring Signals"
void catchSIGTSTP(int signo)
{
	// enters foreground-only mode if not already in it
	if (!fgOnlyFlag)
	{
		char* message = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, strlen(message));

		// sets flag for foreground-only mode to true
		fgOnlyFlag = 1;
	}
	// if already in foreground-only mode, exits it
	else
	{
		char* message = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, message, strlen(message));

		// sets flag for foreground-only mode to false
		fgOnlyFlag = 0;
	}
}

int main()
{
	//sigaction structs setup according to examples from Signals lecture
	struct sigaction SIGTSTP_action = { 0 };
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	SIGTSTP_action.sa_flags = 0;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	// default behavior is to ignore SIGINT, only foreground children should terminate
	struct sigaction ignore_action = { 0 };
	ignore_action.sa_handler = SIG_IGN;
	sigaction(SIGINT, &ignore_action, NULL);

	// loops until the user enters "exit"
	while (strcmp(userInput, "exit") != 0)
	{
		int i;
		
		//allocate memory for argument array
		for (i = 0; i < 512; i++)
		{
			args[i] = malloc(50 * sizeof(*args[i]));
			memset(args[i], '\0', 50 * sizeof(*args[i]));
		}

		// resets flags to 0 (false) for the new command line
		resetFlags();

		// get input from user
		getUserInput();

		// if the user input anything, seperate it into commands and arguments, else line was empty
		if (userInput[0])
		{
			parseInput();
			buildCommands();
		}
		
		// if commands other than comment were entered, for and execute them
		if (commandFlag && !commentFlag)
		{
			forkIt();
		}
		
		// call functions for built in commands if their flags are set
		if (cdFlag)
		{
			cdFunc();
		}
		else if (statusFlag)
		{
			statusFunc();
		}
		else if (exitFlag)
		{
			exitFunc();
		}

		// looks for any finished processes
		findFinishedProcesses();

		// free memory for argument array
		for (i = 0; i < 512; i++)
		{
			free(args[i]);
		}
	}

	return 0;
}