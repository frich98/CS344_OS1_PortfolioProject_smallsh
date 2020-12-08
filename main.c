// CS 344 - OS - Fall 2020 - Assignment 3
// Frannie Richert
// November 3, 2020
// Compile with --std=gnu99

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

// Prototype Files
#include "cmd_functions.h"
#include "process_functions.h"

// GLOBAL VARIABLE TO HANDLE WHETHER OR NOT AMPERSAND (&) IS ALLOWED
int ampersandAllowed = 1; // 1 = true, 0 = false


/*------------------------------------------------------------------ 

                              MAIN

-------------------------------------------------------------------*/

int main(int argc, char *argv[]){
	
	// -------- Signal Handling for SIGINT  
	// Per the instructions, parent process must ignore SIGINT
	// Background children must ignore sign int
	// But, foreground children must terminate when receive sigint
	// Source: https://stackoverflow.com/questions/12953350/ignore-sigint-signal-in-child-process
	signal(SIGINT, SIG_IGN);
	
	// -------- Signal Handling for SIGINT  
	// Per the instructions, parent process must ignore SIGINT
	// Background children must ignore sign int
	// But, foreground children must terminate when receive sigint
	// Source: https://stackoverflow.com/questions/12953350/ignore-sigint-signal-in-child-process
	signal(SIGTSTP, signalHandlerSIGTSTP);
	
	// ---- First line of command prompt
	printf("\n$ smallsh");
	fflush(stdout);
	
	// ---- Make a linked list of child processes
	struct childProcess* childProcessList = createProcessLinkedList();
	
	// ---- Get smallsh PID # and convert to a string
	int smallshPID = (int)getpid();
	int countPIDdigits = countDigits((long long)smallshPID);
	printf("\nPID = %i, digits = %i", smallshPID, countPIDdigits);
	char* pidStr;
	pidToString(&pidStr, smallshPID, countPIDdigits);
	
	printf("\nPID of Smallsh: %s", pidStr);
			
	// --------------------------------- MAIN LOOP
	// --------------------------------- MAIN LOOP
	// --------------------------------- MAIN LOOP
	
	while(1){
		
		// Each loop of the while, need to go find processes which
		// are not done, and update their status and print
		// to console
		updateChildProcesses(childProcessList);
		
		// --- Get User Input
		int stringInputCheck = 0; // if string == 1, else 0	
		char* userInput;
		stringInputCheck = getUserCommand(&userInput, pidStr);
			
		// --- If user entered a valid string, process their command
		if (stringInputCheck == 1){
			
			// Create a command struct
			struct cmd *userCmd = createCMD(userInput);
			
			// IF ampersands are not allowed, no matter what user has entered
			// set ampersand to 0 or false. 
			if(ampersandAllowed == 0){
				userCmd->ampersandTF = 0;
			}
			
			// --- IF user chose a non-built-in command: 
			if(strcmp(userCmd->command, "exit") 	!= 0 &&
			   strcmp(userCmd->command, "status") 	!= 0 &&
			   strcmp(userCmd->command, "cd") 	!= 0 ){
					
					// --- Build a new child foreground process
					createChildProcess(userCmd, &childProcessList, pidStr, userInput);
			}
			   
			// Built-In Command #3 - Status - takes no arguments 
			// and ignores build-in commands
			if(strcmp(userCmd->command, "status") == 0 ){
				printLastFGProcStatus(childProcessList);	
			}
			
			// Built-In Command #2 - CD - takes at most one arg
			// 
			if(strcmp(userCmd->command, "cd") == 0 ){
				if(userCmd->countArgs == 2){
					changeDirectory(userCmd->commandArgs[0]);
				} else {
					changeDirectory("\0");
				}
			}

			// Built-In Command #1 - Exit - takes no arguments
			if(strcmp(userCmd->command, "exit") == 0 ){
				
				// End all background child processes that are still running
				killChildProcesses(childProcessList);			
				
				// Free dynamically alloc memory
				free(pidStr);	
				
				deleteCmd(userCmd);
				free(userCmd);
				userCmd = NULL;
			
				free(userInput);
				userInput = NULL;
				
				deleteLinkedList(childProcessList);
			
				// exit main, i.e. end program
				exit(0);	
			}
			
			// Free Cmd created dynamically
			deleteCmd(userCmd);
			free(userCmd);
			// Free user Input
			free(userInput);
		}		
	} 
	
	// Dynamically allocated memory - freeing
	free(pidStr);	
	deleteLinkedList(childProcessList);
	
	return 0;
}
