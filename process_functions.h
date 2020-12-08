// CS 344 - OS - Fall 2020
// Assignment 3, smallsh
// FUNCTION PROTOTYPES - for process stuff
// Frannie Richert
// November 3, 2020

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

#include "cmd_functions.h"

#ifndef PROCESS_FUNCTIONS_H
#define PROCESS_FUNCTIONS_H

// GLOBAL VARIABLE TO HANDLE WHETHER OR NOT AMPERSAND (&) IS ALLOWED
// will define and put a value to this in main
// just putting this here so the compiler won't complain about this global
// variable not being declared in this file.
int ampersandAllowed; // 1 = true, 0 = false


/*--------------------------------------------------------------------- 
Purpose: A struct to hold process information, 
will be combined into a linked list of processes
----------------------------------------------------------------------*/
struct childProcess
{
	int pid; // process id #, will cast pid_t to int in the creation func
	bool builtInCommand;  // builtInFunctions: exit, status, cd
	bool runInBackground;  // background = true, foreground = false
	bool terminatedNormally;  // true for normal, false for not normal
	bool hasFinished;  // will be false if user runs background process initially, will get updated later
	int exitStatusNum;  // exit status, could be null
	int termSignalNum;   // terminating signal, could be null
	struct childProcess *next; // linked to next process in list	
	struct childProcess *prev;  // linked to prior process in list
};

/*--------------------------------------------------------------------- 
Purpose: Remove child process from list based on PID #
Inputs: linked list head pointer, int PID #
Outputs: status, if successful = 1, else 0 
Source(s):
----------------------------------------------------------------------*/
int removeChildFromList(struct childProcess** list, int PID);

/*--------------------------------------------------------------------- 
Purpose: Print linked list
Inputs: linked list of childProcess structs
Outputs: none
Source: https://www.geeksforgeeks.org/linked-list-set-1-introduction/
----------------------------------------------------------------------*/
void printActiveProcessLinkedList(struct childProcess* list);

/*--------------------------------------------------------------------- 
Purpose: Count Processes in Linked List
Inputs: linked list of childProcess structs
Outputs: int # processes
----------------------------------------------------------------------*/
int countListProcesses(struct childProcess* list);


/*--------------------------------------------------------------------- 
Purpose: BUILt IN COMMAND #3
		 Return exit status or term signal of last foreground process
         ran by our shell. the three built-in commands DO NOT count
		 as foreground processes for the purposes of this command.
		 Print either exit or term status
Inputs: linked list of childProcess structs
Outputs: none directly, will print out values inside function to console
----------------------------------------------------------------------*/
void printLastFGProcStatus(struct childProcess* list);

/*--------------------------------------------------------------------- 
Purpose: Free entire linked list
Inputs: linked list head pointer
Outputs: https://stackoverflow.com/questions/6417158/c-how-to-free-nodes-in-the-linked-list
----------------------------------------------------------------------*/
void deleteLinkedList(struct childProcess* list);


/*--------------------------------------------------------------------- 
Purpose: Free dynamically allocated memory in main parent smallsh
Inputs: all variables that need to be freed
Outputs: struct childProcess pointer
----------------------------------------------------------------------*/
void freeDynMem(char* pidStr, char* userInput, struct cmd* userCmd, struct childProcess* list);

/*--------------------------------------------------------------------- 
Purpose: Add child process to linked list, at the end
Inputs: linked list double pointer, new child process
Outputs: none
Source(s):Using part of the movie code from assignment 1 and 2 for the
          linked list 
		  http://piyushgolani.blogspot.com/2012/09/why-use-double-pointer-when-adding-node.html
----------------------------------------------------------------------*/
void addChildToList(struct childProcess** list, struct childProcess* newChild);

/*--------------------------------------------------------------------- 
Purpose: Create linked list
Inputs: none
Outputs: struct childProcess pointer
----------------------------------------------------------------------*/
struct childProcess* createProcessLinkedList();

/*--------------------------------------------------------------------- 
Purpose: signal handler for sigint, default for a foreground child
for sigint should be to terminate the process!
Inputs: signal number
Outputs: none
----------------------------------------------------------------------*/
void signalHandlerSIGINT(int signum);

/*--------------------------------------------------------------------- 
Purpose: signal handler for sigstp, only affecs PARENT PROCESS
and updates the global variable ampersandAccept
Inputs: user command, linked list of processes to add new child to
Outputs: none
----------------------------------------------------------------------*/
void signalHandlerSIGTSTP(int signum);

/*--------------------------------------------------------------------- 
Purpose: Create a child process, using fork, exec, and waitpid
Inputs: user command, linked list of processes to add new child to
Outputs: none
Source(s): 
1) How to create a child process from lecture notes: https://repl.it/@cs344/42execvforklsc#main.c, 
2) to get execvp to work on windows running ubuntu: https://stackoverflow.com/questions/11912878/gcc-error-gcc-error-trying-to-exec-cc1-execvp-no-such-file-or-directory
3) WNOHANG https://canvas.oregonstate.edu/courses/1784217/pages/exploration-process-api-monitoring-child-processes?module_item_id=19893096
4) for input file handling from lecture notes - -> https://repl.it/@cs344/54redirectc#main.c && https://canvas.oregonstate.edu/courses/1784217/pages/exploration-processes-and-i-slash-o?module_item_id=19893106
----------------------------------------------------------------------*/
void createChildProcess(struct cmd* userCmd, struct childProcess** list, char* pidStr, char* userInput);

/*--------------------------------------------------------------------- 
Purpose: Update the termination and exit status of child processes
		 that have run in the background and print their termination
		 or exit status to the console
Inputs: linked list of child processes
Outputs: nothing directly, updating a linked list and printing to console
Source: from linux manpages on wait pid: https://linux.die.net/man/2/waitpid, waitpid(): on success, returns the process ID of the child whose state has changed; if WNOHANG was specified and one or more child(ren) specified by pid exist, but have not yet changed state, then 0 is returned. On error, -1 is returned.
----------------------------------------------------------------------*/
void updateChildProcesses(struct childProcess* list);

/*--------------------------------------------------------------------- 
Purpose: When user has chosen exit, this function will be called
		Kills all non-terminated background jobs
Inputs: linked list of childProcess jobs
Outputs: none
Source: 
----------------------------------------------------------------------*/
void killChildProcesses(struct childProcess* list);


#endif