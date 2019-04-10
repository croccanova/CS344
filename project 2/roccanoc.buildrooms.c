/*	Christian Roccanova
*	CS344-400
*	Program 2 - adventure
*	Randomly builds rooms for a text based adventure game from a set options
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

char* roomTypes[3] = { "START_ROOM", "END_ROOM", "MID_ROOM" };   //Use this to assign a room type per assignment specs
char* roomNames[10] = { "dungeon", "cellar", "cavern", "corridor", "treasury", "courtyard", "barracks", "arena", "kitchen", "stables" };   //List of potential room names
char directory[100];

//Room struct holds room information
struct Room {
	char* name;
	char* type;
	int connections[6];
	int connectCount;
};

//creates the directory that room files will be written to
char* makeFolder() {
	
	sprintf(directory, "roccanoc.rooms.%d", getpid());
	mkdir(directory, 0777);  //make folder and set permissions
	return directory;
}

//assigns types to rooms
void assignType(struct Room roomHold[7])
{
	int a, b;
	int i;

	//randomize start and end rooms
	a = rand() % 7;
	b = rand() % 7;

	//reroll if they are the same
	while (a == b) {
		b = rand() % 7;
	}

	//assign room type
	for (i = 0; i < 7; i++) {
		
		if (i == a) {
			roomHold[i].type = roomTypes[0];	//start room
		}
		else if (i == b) {
			roomHold[i].type = roomTypes[1]; //end room
		}
		else {
			roomHold[i].type = roomTypes[2];
		}
	}
}

//chooses the 7 rooms to use
void pickRooms(struct Room flrP[7]) 
{
	int i;
	int j;
	int k;
	int reRoll;
	
	//loop 7 times for 7 rooms
	for (i = 0; i < 7; i++) {
		
		reRoll = 1;
		while (reRoll == 1) {
			j = rand() % 10;      //randomly generate a index for room name
			reRoll = 0;
			
			//prevents duplicates
			for (k = 0; k < 7; k++) {
				if (flrP[k].name == roomNames[j]) {
					reRoll = 1;
				}
			}
		}
		flrP[i].name = roomNames[j];

	}
}

//generates a random number of connections between rooms
void connectRooms(struct Room flrP[7]) {
	int i;
	int j;
	int k;
	int reRoll;
	int conIndex;	//index of the room to be connected to

	//connection counts to 0
	for (i = 0; i < 7; i++) {
		flrP[i].connectCount = 0;
	}
					
	//loop through each room making connections
	for (i = 0; i < 7; i++) {

		int totalConnections = rand() % 4 + 3; //3 - 6 connections

		while (flrP[i].connectCount < totalConnections) {
			
			reRoll = 1;

			//loops until a valid room is rolled
			while (reRoll == 1) {               
				reRoll = 0;
				conIndex = rand() % 7;	//randomly generate index of room to connect to
				
				//prevents connecting room to itself
				if (conIndex == i) {
					reRoll = 1;
				}

				//prevents duplicate connections
				for (k = 0; k < flrP[i].connectCount; k++) {
					if (flrP[i].connections[k] == conIndex) {
						reRoll = 1;
					}
				}
			}

			flrP[i].connections[flrP[i].connectCount] = conIndex;	//connects current room to indexed room
			flrP[i].connectCount++;       //increment connection count
			flrP[conIndex].connections[flrP[conIndex].connectCount] = i; //connects indexed room back to current room
			flrP[conIndex].connectCount++;
		}
	}
}



//add room information to file
void buildFiles(struct Room flrP[7], char* folder) {
	int i;
	int j;
	
	//move to folder
	chdir(folder);

	//loop once for each room file
	for (i = 0; i < 7; i++) {       
		FILE* myFile = fopen(flrP[i].name, "a"); //opens file - "a" for append
		
		//print room name
		fprintf(myFile, "ROOM NAME: %s\n", flrP[i].name);
				
		//prints connections
		for (j = 0; j < flrP[i].connectCount; j++) {
			fprintf(myFile, "CONNECTION %d: %s\n", j + 1, flrP[flrP[i].connections[j]].name);
		}

		//print room type
		fprintf(myFile, "ROOM TYPE: %s\n", flrP[i].type);

		fclose(myFile);
	}
}

int main() {
	srand(time(NULL)); //seed random number generation
	char* roomsFolder = makeFolder();
	struct Room floorPlan[7];
	pickRooms(floorPlan);
	assignType(floorPlan);
	connectRooms(floorPlan);
	buildFiles(floorPlan, roomsFolder);

	return 0;
}