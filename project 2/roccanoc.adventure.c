/*	Christian Roccanova
*	CS344-400
*	Program 2 - adventure
*	Runs a text based adventure game and can display the current date and time
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

char* roomTypes[3] = { "START_ROOM", "END_ROOM", "MID_ROOM" };	//room types
char* roomNames[10] = { "dungeon", "cellar", "cavern", "corridor", "treasury", "courtyard", "barracks", "arena", "kitchen", "stables" };   //potential room names
pthread_mutex_t myMutex; //initialize mutex

struct Room {
	char* name;
	char* type;
	int connections[6];
	int connectCount;
};

//gets directory storing room files
//Code mostly as shown reading 2.4: Manipulating Directories
char* getDir() {
	int newestDirTime = -1; // Modified timestamp of newest subdir examined
	char targetDirPrefix[32] = "roccanoc.rooms."; // Prefix we're looking for
	char* newestDirName = malloc(sizeof(char) * 64); // Holds the name of the newest dir that contains prefix
	memset(newestDirName, '\0', sizeof(newestDirName));

	DIR* dirToCheck; // Holds the directory we're starting in
	struct dirent *fileInDir; // Holds the current subdir of the starting dir
	struct stat dirAttributes; // Holds information we've gained about subdir

	dirToCheck = opendir("."); // Open up the directory this program was run in

	if (dirToCheck > 0) // Make sure the current directory could be opened
	{
		while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
		{
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
			{				
				stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

				if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
				{
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);					
				}
			}
		}
	}

	closedir(dirToCheck); // Close the directory we opened
	return newestDirName;
}

//fills the array of Room structs with name, type and connection information
void fillRms(char* directory, struct Room flrP[]) {
	DIR* folder = opendir(directory);
	struct dirent* filnm;
	int count = 0;
	int lineCount = 0; //holds total number of lines in the current file
	int i, j, k;
	FILE* myFile;
	char fileName[100];
	char line[100];	//holds the current line of text
	char foo[20], bar[20]; //holds junk
	char con[20]; //holds connections
	char tp[20]; //holds type
	char n[20]; //holds name

	//initialize connectCount
	for (i = 0; i < 7; i++) {
		flrP[i].connectCount = 0;
	}

	while ((filnm = readdir(folder)) != NULL) {

		//skip these, they do not have room information
		if (!strcmp(filnm->d_name, ".") || !strcmp(filnm->d_name, "..")) {
			continue;
		}

		sprintf(fileName, "./%s/%s", directory, filnm->d_name);		//filepath
		
			myFile = fopen(fileName, "r"); //open for read
			//get line count - iterate until fgets runs out of lines to read
			while (fgets(line, 100, myFile)) {
				lineCount++;
			}
			fclose(myFile);

			//get name
			myFile = fopen(fileName, "r"); //open for read
			fgets(line, 100, myFile);
			
			sscanf(line, "%s %s %s", foo, bar, n); //break up name line
						
			//compare name segment (n) to list of room names to fill struct array
			for (j = 0; j < 10; j++) {
				if (strcmp(n, roomNames[j]) == 0) {
					flrP[count].name = roomNames[j];

				}
			}

			//get connections
			for (j = 0; j < lineCount - 2; j++) {
				fgets(line, 100, myFile);
				sscanf(line, "%s %s %s", foo, bar, con); //break up connection line
				
				//compare connection segment (con) to list of room names to fill struct array
				for (k = 0; k < 10; k++) {
					if (strcmp(con, roomNames[k]) == 0) {
						flrP[count].connections[j] = k; //set connection equal to index of connecting room

					}
				}

				//increment count of connections
				flrP[count].connectCount++;
			}

			//get type
			fgets(line, 100, myFile);						
			sscanf(line, "%s %s %s", foo, bar, tp); //break up type line

			//compare type segment (tp) to list of types to fill struct array
			for (j = 0; j < 3; j++) {
				if (strcmp(tp, roomTypes[j]) == 0) {
					flrP[count].type = roomTypes[j];
					
				}
			}
			
			//reset line count
			lineCount = 0;
			fclose(myFile);
		

		count++;
	}

	closedir(folder);
}

//finds the name of the first room
const char* getStart(struct Room flrP[]) {
	int i;
	char* start;

	//cycles through types until START_ROOM is found
	for (i = 0; i < 7; i++) {
		if (strcmp(flrP[i].type, roomTypes[0]) == 0) {
			start = flrP[i].name;
		}
	}

	return start;
}

//checks if two rooms connect
int connectTrue(struct Room currentRm, char roomName[50]) {
	int i;
	for (i = 0; i < currentRm.connectCount; i++) {
		//compares given room name to list of connections in current room
		if (strcmp(roomName, roomNames[currentRm.connections[i]]) == 0) {
			return 1;
		}
	}
	return 0;
}

//runs the game
playGame(struct Room flrP[], char begin[100]) {
	int i;
	int index;		//index for array of rooms
	int steps = 0;
	int gameOver = 0; //controls if the game continues - ends if equal to 1
	char newRoom [100];
	char room[50];
	char* victoryPath[1000];	//holds set of visited rooms

	//get index value for initial room
	for (i = 0; i < 7; i++) {
		if (strcmp(flrP[i].name, begin) == 0) {
			index = i;			

		}
	}


	//loop until end condition is met
	while (gameOver != 1) {
		printf("\nCURRENT LOCATION: %s\n", flrP[index].name);
		
		//show user options for next step
		printf("POSSIBLE CONNECTIONS:");
		for (i = 0; i < flrP[index].connectCount; i++) {
			if (i == flrP[index].connectCount - 1) {
				printf(" %s.", roomNames[flrP[index].connections[i]]); //prints last connection with period instead of comma
			}
			else {
				printf(" %s,", roomNames[flrP[index].connections[i]]);
			}
		}
		printf("\nWHERE TO? >");
		
		//get next room from input
		scanf("%s", newRoom);

		//if user inputs time, prints current date and time
		if (strcmp(newRoom, "time") == 0) {
			timeThread();
			printTime();
		}
		//prints error message if input is invalid
		else if (connectTrue(flrP[index], newRoom) == 0){
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
		}
		else {

			//increment total step count
			steps++;
			
				//match user input to next room and set index
				for (i = 0; i < 7; i++) {
					if (strcmp(flrP[i].name, newRoom) == 0) {
						index = i;						
					}
				}

			//add room to the victory path
			victoryPath[steps] = flrP[index].name;
			
			//check if next room is the exit
			if (strcmp(flrP[index].type, "END_ROOM") == 0) {
				gameOver = 1;
				
			}
		}
	}

	//print victory message
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
	
	//print path excludign starting room
	for (i = 1; i <= steps; i++) {
		printf("%s\n", victoryPath[i]);
	}
}

//gets the current date and time and stores it in a file
//references: http://www.cplusplus.com/forum/beginner/87102/     https://stackoverflow.com/questions/1442116/how-to-get-the-date-and-time-values-in-a-c-program
void* getTime() {
	FILE* myFile;                             
	myFile = fopen("currentTime.txt", "w");  //Create and write to the file
	char timeStr[50];
	struct tm *timePtr;

	time_t present = time(0);
	timePtr = gmtime(&present);  //GMT time

	//current date and time
	strftime(timeStr, sizeof(timeStr), "%I:%M %p %A, %B %d, %Y GMT", timePtr); //format according to https://linux.die.net/man/3/strftime
	fputs(timeStr, myFile);
	fclose(myFile);

}

//reads time from file and prints it
void printTime() {
	FILE* myFile;
	myFile = fopen("currentTime.txt", "r");   //open file for read
	char curTime[50];

	fgets(curTime, 50, myFile);     //read time string
	printf("\n%s\n", curTime);
	fclose(myFile);

}

//manages threads
//references: http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_lock.html	http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_init.html	https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html
void timeThread() {
	pthread_t myThread;
	pthread_mutex_init(&myMutex, NULL);
	pthread_mutex_lock(&myMutex);

	//create time thread
	int tid = pthread_create(&myThread, NULL, getTime, NULL);
	pthread_mutex_unlock(&myMutex);
	pthread_mutex_destroy(&myMutex);

}

int main() {
	char entrance[50];	//holds name of starting room
	struct Room floorPlan[7];	//holds room info
	char* roomDir = getDir();
	fillRms(roomDir, floorPlan);
	strcpy(entrance, getStart(floorPlan));
	playGame(floorPlan, entrance);
	return 0;
}

