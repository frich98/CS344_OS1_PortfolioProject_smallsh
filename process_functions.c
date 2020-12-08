// CS 344 - OS - Fall 2020
// Assignment 3, smallsh
// FUNCTION DEFINITIONS - for process stuff
// Frannie Richert
// November 3, 2020

#include "process_functions.h"

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


/*--------------------------------------------------------------------- 
Purpose: Remove child process from list based on PID #
Inputs: linked list head pointer, int PID #
Outputs: status, if successful = 1, else 0 
Source(s):
----------------------------------------------------------------------*/
int removeChildFromList(struct childProcess** list, int PID){
	
	// Loop through all elements in list
	while((*list)->pid != PID && (*list) != NULL){
		(*list) = (*list)->next;
	}
	
	// No match, return 0
	if((*list) == NULL){
		return 0;	
	} 
	//Found a match! remove the item from the list
	else if((*list)->pid == PID){
		
		(*list)->prev->next = (*list)->next;
		(*list)->next->prev = (*list)->prev;
		
		(*list)->next = NULL;
		(*list)->prev = NULL;
		free((*list));
		return 1;
	}	
	
	return 0;
}

/*--------------------------------------------------------------------- 
Purpose: Print linked list
Inputs: linked list of childProcess structs
Outputs: none
Source: https://www.geeksforgeeks.org/linked-list-set-1-introduction/
----------------------------------------------------------------------*/
void printActiveProcessLinkedList(struct childProcess* list){	

	while(list != NULL){
		if(list->hasFinished == 0){
			printf("\nPID = %i", list->pid);
			fflush(stdout);
		}
		list = list->next;
	}
}

/*--------------------------------------------------------------------- 
Purpose: Count Processes in Linked List
Inputs: linked list of childProcess structs
Outputs: int # processes
----------------------------------------------------------------------*/
int countListProcesses(struct childProcess* list){	
	int count = 0;
	while(list != NULL){
		count += 1;
		list = list->next;
	}
	return count;
}



/*--------------------------------------------------------------------- 
Purpose: BUILt IN COMMAND #3
		 Return exit status or term signal of last foreground process
         ran by our shell. the three built-in commands DO NOT count
		 as foreground processes for the purposes of this command.
		 Print either exit or term status
Inputs: linked list of childProcess structs
Outputs: none directly, will print out values inside function to console
----------------------------------------------------------------------*/
void printLastFGProcStatus(struct childProcess* list){	
	
	// Will hold current position in list
	struct childProcess* temp = list; 
	
	// counter for position
	int idx = 0;
	
	// Will hold chosen position in list
	struct childProcess* chosenProcess = list; 
	
	if(temp == NULL){
		printf("\nexit value %i", 0);
		fflush(stdout);
		return;
	}
	
	// -- Find position of last foreground process
	while(temp != NULL){
		//printf("\nCurrent process: %i", temp->pid);
		fflush(stdout);
		if(temp->runInBackground == false){
			chosenProcess = temp;
		}
		idx += 1;
		temp = temp->next;
	}
	
	// Ended normally, print exit status
	if(chosenProcess->terminatedNormally == 1){
		printf("\nexit value %i", chosenProcess->exitStatusNum);
		fflush(stdout);
	}
	// Ended abnormally, print term signal
	else {
		printf("\nterminated by signal %i", chosenProcess->termSignalNum);
		fflush(stdout);
	}
	
}

/*--------------------------------------------------------------------- 
Purpose: Free entire linked list
Inputs: linked list head pointer
Outputs: https://stackoverflow.com/questions/6417158/c-how-to-free-nodes-in-the-linked-list
----------------------------------------------------------------------*/
void deleteLinkedList(struct childProcess* list){
	
	// TESTING - making sure list is as expected
	// printActiveProcessLinkedList(list);

	struct childProcess* temp = list;
	
	// Loop through all elements in list
	while(list != NULL){
		//printf("\nwe are in the middle of deleting linked list");
		temp = list;
		list = list->next;
		free(temp);
	}
}


/*--------------------------------------------------------------------- 
Purpose: Free dynamically allocated memory in main parent smallsh
Inputs: all variables that need to be freed
Outputs: struct childProcess pointer
----------------------------------------------------------------------*/
void freeDynMem(char* pidStr, char* userInput, struct cmd* userCmd, struct childProcess* list){	
	// Free dynamically alloc memory				
	free(pidStr);
	free(userInput);
	deleteCmd(userCmd);
	free(userCmd);
	deleteLinkedList(list);
}


/*--------------------------------------------------------------------- 
Purpose: Add child process to linked list, at the end
Inputs: linked list double pointer, new child process
Outputs: none
Source(s):Using part of the movie code from assignment 1 and 2 for the
          linked list 
		  http://piyushgolani.blogspot.com/2012/09/why-use-double-pointer-when-adding-node.html
----------------------------------------------------------------------*/
void addChildToList(struct childProcess** list, struct childProcess* newChild){
	
	// Is this the first node in the linked list?
	if ((*list) == NULL)
	{
		// This is the first node in the linked link
		// Set the head and the tail to this node
		(*list) = newChild;
	}
	
	// If not, loop through items until we find last element
	else {
		struct childProcess* root = *list;
		while(root->next != NULL){
			root = root->next;
		}
		// now the last node, set it's next value
		// equal to the new child, and newChild's previous value is the prev last node
		root->next = newChild;
		newChild->prev = root; 
	}	
}

/*--------------------------------------------------------------------- 
Purpose: Create linked list
Inputs: none
Outputs: struct childProcess pointer
----------------------------------------------------------------------*/
struct childProcess* createProcessLinkedList(){	
	// The head of the linked list
    struct childProcess* head = NULL;
	return head;	
}



/*--------------------------------------------------------------------- 
Purpose: signal handler for sigint, default for a foreground child
for sigint should be to terminate the process!
Inputs: signal number
Outputs: none
----------------------------------------------------------------------*/
void signalHandlerSIGINT(int signum) {  
	exit(1);
}

/*--------------------------------------------------------------------- 
Purpose: signal handler for sigstp, only affecs PARENT PROCESS
and updates the global variable ampersandAccept
Inputs: user command, linked list of processes to add new child to
Outputs: none
----------------------------------------------------------------------*/
void signalHandlerSIGTSTP(int signum){

	// flip flopping value
	if(ampersandAllowed == 1){
		char* msg = "Entering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, msg, 50);	
		ampersandAllowed = 0;
	} else {
		char* msg = "Exiting foreground-only mode (& is allowed again)\n";
		write(STDOUT_FILENO, msg, 51);
		ampersandAllowed = 1;
	}
	char* newColon = ": ";
	write(STDOUT_FILENO, newColon, 2);
}


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
void createChildProcess(struct cmd* userCmd, struct childProcess** list, char* pidStr, char* userInput){
	
	// Parse user command and build args array, see 2nd source
	// NOTE, args count includes a '\0' as last element, do not include
	int lenArr = 1 + userCmd->countArgs - 1 + 1; // last 1 for NULL

	// Create and fill in a new array
	char** newArgsArr = calloc(lenArr, sizeof(char*));
	int i;
	for(i = 0; i < lenArr; i++){
		if(i == 0){
			newArgsArr[i] = calloc(strlen(userCmd->command) + 1, sizeof(char));
			strcpy(newArgsArr[i], userCmd->command);
		} else if (i == (lenArr - 1)){
			newArgsArr[i] = NULL;
		} else {
			newArgsArr[i] = calloc(strlen(userCmd->commandArgs[i-1]) + 1, sizeof(char));
			strcpy(newArgsArr[i], userCmd->commandArgs[i-1]);
		}
	}
	
	// Used in waitpid below
	int childStatus;
	int kidID = 0; // used when user puts a process in background
		
	// --- Fork off a child from the parent (i.e. smallsh)
	pid_t kidProcess = fork();

	// --- Execute Child Process	
	if(kidProcess == -1){
		perror("fork()\n");
		exit(1);
	}
	else if (kidProcess == 0){
		
		// -------- Signal Handling for SIGTSTP  
		//  all child processes ignore SIGTSTP (CTRL + Z)
		signal(SIGTSTP, SIG_IGN);
		
		int targetFD_read;
		int targetFD_write;
		

		// --- If the user had an input file name argument
		if(strcmp(userCmd->inputFile, " \0") != 0){
			targetFD_read = open(userCmd->inputFile, O_RDONLY, 0640);
			fcntl(targetFD_read, F_SETFD, FD_CLOEXEC);
			// Use dup2 to point FD 0, i.e. defaults to reading from terminal
			//int resultr = 
			dup2(targetFD_read, 0);
			// Making sure file didn't open incorrectly
			if (targetFD_read == -1) {
				printf("cannot open the file named %s for input, ", userCmd->inputFile);
				fflush(stdout);
				perror("");
			}
		} 
		// user DOES NOT have an input file && background process, redirect to /dev/null
		else if (userCmd->ampersandTF == 1){
			targetFD_read = open("/dev/null", O_RDONLY, 0640);
			fcntl(targetFD_read, F_SETFD, FD_CLOEXEC);
			// Use dup2 to point FD 0, i.e. defaults to reading from terminal
			//int resultr = 
			dup2(targetFD_read, 0);
			// Making sure file didn't open incorrectly
			if (targetFD_read == -1) {
				printf("cannot open /dev/null for input");
				fflush(stdout);
				perror("");
			}
		} else {
			targetFD_read = 1; // successful
		}
		
		// --- If the user had an output file name argument
		if(strcmp(userCmd->outputFile, " \0") != 0){
			targetFD_write = open(userCmd->outputFile, O_WRONLY | O_CREAT |
			O_TRUNC, 0640);
			fcntl(targetFD_write, F_SETFD, FD_CLOEXEC);
			// Use dup2 to point FD 1, i.e. defaults to writing to terminal
			//int resultw = 
			dup2(targetFD_write, 1);
			// Making sure file didn't open incorrectly
			if (targetFD_write == -1) {
				printf("cannot open the file named %s for output, ", userCmd->outputFile);
				fflush(stdout);
				perror("");
			}
		} 
		// user DOES NOT have an output file && background process, redirect to /dev/null
		else if (userCmd->ampersandTF == 1){
			targetFD_write = open("/dev/null", O_WRONLY | O_CREAT |
			O_TRUNC, 0640);
			fcntl(targetFD_write, F_SETFD, FD_CLOEXEC);
			// Use dup2 to point FD 1, i.e. redirecting to terminal write
			//int resultw = 
			dup2(targetFD_write, 1);
			// Making sure file didn't open incorrectly
			if (targetFD_write == -1) {
				printf("cannot open /dev/null for output");
				fflush(stdout);
				perror("");
			}
		} else {
			targetFD_write = 1; // successful
		}
					
		// If the child is a foreground process, DO NOT IGNORE SIGINT
		// and terminate the process
		if(userCmd->ampersandTF == 0){
			//printf("ARE WE GETTING HERE??");
			fflush(stdout);
			signal(SIGINT, signalHandlerSIGINT);
		}
		
		
		// In the child process
		//printf("\nCHILD(%d) running command.\n", kidProcess);
		
		// --- Use an exec function to run the command
		// execVP, replacing current program,
		// this takes an array of arguments, and 
		// looks for executable in PATH envir var
	
		if(targetFD_read != -1 && targetFD_write != -1){
			
			// Run Execvp command with newly built array
			execvp(newArgsArr[0], newArgsArr);
		
			// --- exec returns only on error (from lecture notes)
			perror(newArgsArr[0]); 
		
		}
	

		// --- Free newly created dyn arr mem
		for(i = 0; i < lenArr; i++){
			if(i < (lenArr - 1)){  // last NULL char does not need to be freed
				free(newArgsArr[i]);
			} 
		}
		free(newArgsArr);
		
		freeDynMem(pidStr, userInput, userCmd, *list);
		
		exit(1);  // not successful, will kill child process
		
	} else {
		
		// -- If background, need to store pid somewhere immediately
		kidID = (int)kidProcess;
		if(userCmd->ampersandTF == 1){
			printf("\nbackground pid is %d", kidID);
			fflush(stdout);
		}

		//printf("\n pid is %d", kidID);

		// In the parent process, wait for child's termination
		//The third flag is WNOHANG and specifying it makes waitpid non-blocking, i.e., if WNOHANG flag is specified and no child process has terminated, then waitpid will return immediately with the return value of 0,
		
		// If user wants to run the process in the background, use WNOHANG
		if(userCmd->ampersandTF == 1){
			kidProcess = waitpid(kidProcess, &childStatus, WNOHANG);
			//printf("\nKid Process value: %i", (int)kidProcess);
		} 
		// else, it is a foreground process, wait for process to finish
		// before returning control to smallsh
		else {
			kidProcess = waitpid(kidProcess, &childStatus, 0);
		}
	
		//printf("\nKid Process is equal to %i", kidID);
		//printf("\nWe got here!!!");
	} 		
	
	// --- Create a new child process struct
	struct childProcess* newChild = malloc(sizeof(struct childProcess));
		
	if(userCmd->ampersandTF == 1){
		newChild->pid = kidID;
		newChild->runInBackground = true;
		newChild->hasFinished = false;
	} else {
		newChild->pid = (int)kidProcess;
		newChild->runInBackground = false;
		newChild->hasFinished = true;
	}
	
	//printf("\nKid Process is equal to %i", newChild->pid);
	fflush(stdout);
	newChild->next = NULL;
	newChild->prev = NULL;
	newChild->builtInCommand = false;

	// if child terminated normally
	if(userCmd->ampersandTF == 0){
		if(WIFEXITED(childStatus)){
			newChild->terminatedNormally = true;
			newChild->exitStatusNum = WEXITSTATUS(childStatus);
			newChild->termSignalNum = 0;
			//printf("\nChild Exited Normally With Status: %i\n", newChild->exitStatusNum);
			fflush(stdout);
		 } 
		 // child terminated abnormally
		 else if (WIFSIGNALED(childStatus)){
			newChild->terminatedNormally = false;
			newChild->exitStatusNum = 0;
			newChild->termSignalNum = WTERMSIG(childStatus);
			// response from sigint
			if(newChild->termSignalNum == 2){
				printf("\nterminated by signal %i", newChild->termSignalNum);	
				fflush(stdout);	
			}
		
		 }
	} 
	// if our process hasn't finished yet, will be updated in a later
	// loop 
	else {
			newChild->terminatedNormally = NULL;
			newChild->exitStatusNum = 0;
			newChild->termSignalNum = 0;
	}
	// Add it to the list
	addChildToList(list, newChild);
	//printActiveProcessLinkedList(*list);
	
	//printf("\nNumber of total processes: %i", countListProcesses(*list));
	//fflush(stdout);

	// --- Free newly created dyn arr mem
	for(i = 0; i < lenArr; i++){
		if(i < (lenArr - 1)){  // last NULL char does not need to be freed
			free(newArgsArr[i]);
		} 
	}
	free(newArgsArr);
}


/*--------------------------------------------------------------------- 
Purpose: Update the termination and exit status of child processes
		 that have run in the background and print their termination
		 or exit status to the console
Inputs: linked list of child processes
Outputs: nothing directly, updating a linked list and printing to console
Source: from linux manpages on wait pid: https://linux.die.net/man/2/waitpid, waitpid(): on success, returns the process ID of the child whose state has changed; if WNOHANG was specified and one or more child(ren) specified by pid exist, but have not yet changed state, then 0 is returned. On error, -1 is returned.
----------------------------------------------------------------------*/
void updateChildProcesses(struct childProcess* list){
	
	// used to keep track of proces status in waitpid
	int status;
	int waitPIDReturn = 0;

	while(list != NULL){	

		//printf("\nwe are in the middle of updating child processes");
	
		if(list->runInBackground == 1 && list->hasFinished != 1){
			
			waitPIDReturn = waitpid(list->pid, &status, WNOHANG);
			
			if(waitPIDReturn != 0){
				if(WIFEXITED(status)){
					list->terminatedNormally = true;
					list->exitStatusNum = WEXITSTATUS(status);
					list->termSignalNum = 0;
					printf("\nbackground pid %d is done: exit value %i", list->pid, list->exitStatusNum);
					fflush(stdout);
				 } 
				 // child terminated abnormally
				 else if (WIFSIGNALED(status)){
					list->terminatedNormally = false;
					list->exitStatusNum = 0;
					list->termSignalNum = WTERMSIG(status);
					printf("\nbackground pid %d is done: terminated by signal %i", list->pid, list->termSignalNum);
					fflush(stdout);
				 }
				
				list->hasFinished = true;	
			}
		}
		
		list = list->next;
	}
}

/*--------------------------------------------------------------------- 
Purpose: When user has chosen exit, this function will be called
		Kills all non-terminated background jobs
Inputs: linked list of childProcess jobs
Outputs: none
Source: 
----------------------------------------------------------------------*/
void killChildProcesses(struct childProcess* list){
	
	// used to keep track of proces status in waitpid
	while(list != NULL){	
		//printf("\nwe are in middle of killing child processes");
		if(list->runInBackground == 1 && list->hasFinished == 0){
			//printf("\nKilling background process with id %i", list->pid);
			//fflush(stdout);
			kill(list->pid, 9);			
			list->hasFinished = 1;
		}		
		list = list->next;
	}
	
	//printf("\n");
	//fflush(stdout);
}
