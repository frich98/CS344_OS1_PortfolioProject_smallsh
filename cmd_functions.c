// CS 344 - OS - Fall 2020
// Assignment 3, smallsh
// FUNCTION DEFINITIONS
// Frannie Richert
// November 3, 2020

#include "cmd_functions.h"

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

/*--------------------------------------------------------------------- 
Purpose: Built-In Command # 3 (CD)
        change working directory of smallsh
        with no args, changes curr wd to directory specified in HOME environment
		variable. assume input to function will have length 1 and have '\0' as its
		only character
		if one arg is given, it is the path of a directory to change to which 
		can be either relative, or absolute
Input: user provided argument, can be blank or filled in 
Output: no output, void
Source(s): 
https://repl.it/@cs344/45envvarsc#main.c, 
example from module 5 for setenv
https://canvas.oregonstate.edu/courses/1784217/pages/exploration-files?module_item_id=19893088
----------------------------------------------------------------------*/

void changeDirectory(char* path){
	
	char* currDir = calloc(1000, sizeof(char));
	getcwd(currDir, 1000);  // USE GETCWD PER HW INSTRUCTIONS, NOT GETENV()
	//printf("\nCurrent Dir: %s", currDir);
	//fflush(stdout);
	
	//If user did NOT provide a path, assign working dir to HOME envir var
	// use setenvr() to set PWD environmental variable to new wd HOME 
	if(path[0] == '\0'){		
	
		// getting home directory variable value
		// setting size fo 1000 to be safe
		char* homeVar = calloc(1000, sizeof(char));
		strcat(homeVar, getenv("HOME"));
		//printf("\nHome var = %s", homeVar);
		//fflush(stdout);
		
		// Change current directory
		chdir(homeVar);
		
		// Free dynam allo memory
		free(homeVar);
	} 
	
	// Else, user DID provide a path, figure out whether or not path is relative
	// or absolute and move current dir to this path
	else {
		
		// --- Relative path - does NOT start with "/"
		// starts with a period - indicates file is in current WD
		// doesn't start with a period - indicates file is in current WD
		if((path[0] == '.' && path[1] == '/') || (path[0] != '.')){
			
			// if it does start with a ".", need to remove it before concatenating
			if(path[0] == '.'){
				
				// removing "." in string, strlen does NOT include '\0'
				int newPathLen = strlen(path);
				char* newPath = calloc(newPathLen, sizeof(char));
				int i = 0;
				int strIdx = 0;
				for(i = 0; i < strlen(path); i++){					
					if(path[i] != '.'){
						newPath[strIdx] = path[i];
						strIdx += 1;	
					}
				}
				newPath[newPathLen] = '\0';
				
				char* fullPath = calloc(strlen(currDir) + strlen(newPath) + 1, sizeof(char));
				strcat(fullPath, currDir);
				strcat(fullPath, newPath);	
				
				// Change current directory
				chdir(fullPath);

				//Free dynamically alloc memory
				free(fullPath);
				free(newPath);
				
			} else {
				
				char* fullPath = calloc(strlen(currDir) + strlen(path) + 1 + 1, sizeof(char));
				strcat(fullPath, currDir);
				strcat(fullPath, "/");
				strcat(fullPath, path);	
				
				// Change current directory
				chdir(fullPath);
				
				//Free dynamically alloc memory
				free(fullPath);

			}			
		} 

		// starts with two periods - indicates file is in parent WD
		else if(path[0] == '.' && path[1] == '.'){
			
				// removing "." in string, strlen does NOT include '\0'
				int newPathLen = strlen(path)-1;
				char* newPath = calloc(newPathLen, sizeof(char));
				int i = 0;
				int strIdx = 0;
				// looping through original elements of path, moving over to new path
				for(i = 0; i < strlen(path); i++){					
					if(path[i] != '.'){
						newPath[strIdx] = path[i];
						strIdx += 1;
					}
				}				
				newPath[newPathLen-1] = '\0';
				
				// NEED TO FIND PARENT DIRECTORY, go back one / in currDir
				
				// first, find last instance of "/				
				int j = 0;
				int lastSlashPos = 0;
				for(j = strlen(currDir) - 1; j >= 0; j--){
					if(currDir[j] == '/'){
						lastSlashPos = j;
						break;
					}
				}
				
				// second, figure out how many characters we need in parent dir
				int countChars = lastSlashPos;
				char* parentDir = calloc(countChars + 1, sizeof(char));
				
				// third, fill in new array
				for(i = 0; i < countChars; i++){
					parentDir[i] = currDir[i];
				}
				parentDir[countChars] = '\0';				
					
				char* fullPath = calloc(strlen(parentDir) + strlen(newPath) + 1, sizeof(char));
				strcat(fullPath, parentDir);
				strcat(fullPath, newPath);	
				
				// Change current directory
				chdir(fullPath);
				
				//Free dynamically alloc memory
				free(fullPath);
				free(parentDir);
				free(newPath);		
				
		}
		// --- Absolute path - DOES start with a "/"
		else if(path[0] == '/'){
			// Change current directory
			chdir(path);
		}	
	}
	
	char* currDir2 = calloc(1000, sizeof(char));
	getcwd(currDir2, 1000);  // USE CWD PER HW INSTRUCTIONS, NOT GETENV()
	//printf("\nCurrent Dir: %s", currDir2);
	//fflush(stdout);
	
	// freeing dynamically alloc memory
	free(currDir);
	free(currDir2);
	
}

/*--------------------------------------------------------------------- 
Purpose: Fill Out Struct of Command
Input: user entered command string AFTER $$ expansion, 
format of input -->   
command [arg1 arg2 ...] [< input_file] [> output_file] [&]
Output: cmd struct pointer
Source(s):
1) Code for movie struct from Assignment 1 and 2 as baseline
----------------------------------------------------------------------*/

struct cmd *createCMD(char* cmdInput){
	
	struct cmd *currCMD = malloc(sizeof(struct cmd));
	
	// --- First, save full command to struct
	currCMD->fullCmdStr = calloc(strlen(cmdInput) + 1, sizeof(char));
	strcpy(currCMD->fullCmdStr, cmdInput);	
	//printf("\nStep1: %s", currCMD->fullCmdStr);
	
	// For use with strtok_r
    char *saveptr;	

    // --- Second, start token to parse space separated values, command 
    char *token = strtok_r(cmdInput, " ", &saveptr);
	
	// only one word in command
	if(token == NULL){
		currCMD->command = calloc(strlen(cmdInput) + 1, sizeof(char));
		strcpy(currCMD->command, cmdInput);		
	} 
	// > 1 space separated word in command
	else{
		currCMD->command = calloc(strlen(token) + 1, sizeof(char));
		strcpy(currCMD->command , token);
	}
	
	//printf("\nStep2: %s", currCMD->command);
	

	// --- Third, full set of arguments, can be a max of 512 arguments
	int countArgs = 0;
	
	// Max of 512 args, leaving an extra for '\0' to signify end of args
	currCMD->commandArgs = calloc(513, sizeof(char*));
	
	// loop until no more tokens left
	if(token != NULL){
		token = strtok_r(NULL, " ", &saveptr);	
	}

	// if we do find a special character, skip the while loop 
	int specialCharFound = 0;
	
	while(token != NULL && specialCharFound != 1){		
		// These are reserved for other actions
		if(strcmp(token, "<") == 0 || strcmp(token, ">") == 0 || strcmp(token, "&") == 0){
			specialCharFound = 1;
		} 
		// continue counting args
		else {
			currCMD->commandArgs[countArgs] = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currCMD->commandArgs[countArgs], token);
			countArgs += 1;
			//printf("\nArg#: %i, Arg Name: %s", countArgs-1, currCMD->commandArgs[countArgs-1]);
		}		
		// advance token ONLY IF haven't found a special char
		if(specialCharFound != 1){
			token = strtok_r(NULL, " ", &saveptr);	
		}	
	} // end of while
	
	// last element will be '\0'
	currCMD->countArgs = countArgs + 1;	 // includes \0 at end, hence + 1
	currCMD->commandArgs[countArgs] = calloc(1, sizeof(char));
	currCMD->commandArgs[countArgs][0] = '\0';	

	//printf("\nStep3: %i", currCMD->countArgs);
	
	// Fourth and Fifth - INPUT REDIRECTION CAN HAPPEN BEFORE OR AFTER OUTPUT REDIRECTION	
	int inputFound = 0;
	int outputFound = 0;
	
	// special char will be 1 if found in previous while loop
	while(specialCharFound == 1 && token != NULL){
		
		// Fourth, check and see if we have an input director "<"
		// do not need to advance token since it is still there from
		// check in while loop
		if(strcmp(token, "<") == 0){
			token = strtok_r(NULL, " ", &saveptr);	// move to file name
			currCMD->inputFile = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currCMD->inputFile, token);	
			inputFound = 1;
		} 
		
		// Fifth, check and see if we have an output director ">"
		// do need to advance token
		else if(strcmp(token, ">") == 0){
			token = strtok_r(NULL, " ", &saveptr); // move to file name
			currCMD->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currCMD->outputFile, token);	
			outputFound = 1;
		} 
		
		// Moving on, out of while loop to check if &
		else if ((strcmp(token, ">") != 0 && strcmp(token, "<") != 0) || token == NULL){		
			specialCharFound = 0;  // will jump us out of while loop			
		}
			
		// only advance token if we have found a special character < or > 
		if(specialCharFound != 0){
			token = strtok_r(NULL, " ", &saveptr);
		}		
	} // end of while
	
	
	// For those cases where there were no input or output redirection	
	if(inputFound == 0){
		currCMD->inputFile = calloc(1 + 1, sizeof(char));
		currCMD->inputFile[0] = ' ';
		currCMD->inputFile[1] = '\0';	
	}
	
	if(outputFound == 0){
		currCMD->outputFile = calloc(1 + 1, sizeof(char));
		currCMD->outputFile[0] = ' ';
		currCMD->outputFile[1] = '\0';	
	}	
	
	//printf("\nStep4: %s", currCMD->inputFile);
	//printf("\nStep5: %s", currCMD->outputFile);
		
	// Sixth, check if last character is the & sign only if token not null		
	if(token != NULL){
		if(strcmp(token, "&") == 0){
			currCMD->ampersandTF = 1;
		} 
	} else {
		currCMD->ampersandTF = 0;
	}
	
	// Set the next node to NULL in the newly created cmd entry
    currCMD->next = NULL;
	
	//printf("\nStep6: %i", currCMD->ampersandTF);
		
	return currCMD;
	
}

/*--------------------------------------------------------------------- 
Purpose: Free a cmd struct
Input: pointer to cmd struct 
Output: void
Source(s):
1) Code for movie struct deleteMovie from Assignment 1 and 2 as baseline
----------------------------------------------------------------------*/

void deleteCmd(struct cmd *acmd){
	
	if(acmd != NULL){
		free(acmd->fullCmdStr);
		free(acmd->command);
		free(acmd->inputFile);
		free(acmd->outputFile);
		
		// last element of command args WAS NOT dynamically allocated
		// it is the null character

		int i;
		for(i = 0; i < acmd->countArgs; i++){
			//printf("\nFreed %i\n", i);
			free(acmd->commandArgs[i]);			
		}	
		free(acmd->commandArgs);
	}	
}


/*--------------------------------------------------------------------- 
Purpose: Return String of User Input, AFTER $$ expansion
Input: double char pointer userInput, will fill out in this function
       string shellPID number
Output: int value. if command is a valid string, return 1.
        else, return 0 (#, blank lines, all spaces)
Notes: 
1) Your shell must support command lines with a maximum length of 2048 characters,
   and a maximum of 512 arguments. 
2) 
Sources: 
1) Uing fgets as opposed to scanf to handle blank lines
https://stackoverflow.com/questions/42265038/how-to-check-if-user-enters-blank-line-in-scanf-in-c
2) Clearing the stdout with fflush
source: https://stackoverflow.com/questions/11575102/issue-in-c-language-using-fgets-after-printf-as-fgets-runs-before-printf
----------------------------------------------------------------------*/

int getUserCommand(char** userInput, char* shellPID){
	
	// various counts and position trackers
	int i = 0;
	int countBlanks = 0;
	int countChars = 0;
	int countDollarSigns = 0; // for expansion decision, need 2 in a row
		
	// print command line start ":"
	printf("\n: ");	
	fflush(stdout);
		
	// array to hold string user input temporarily, will assign to userInput later	
	// one extra for null terminator
	// using calloc here because it initializes all values to 0
	*userInput = calloc(2048+1, sizeof(char));
	
	// NOTE fgets stores the newline char at the end of the string
	// source: https://www.ibm.com/support/knowledgecenter/ssw_ibm_i_74/rtref/fgets.htm
	fflush(stdout);	
	fgets(*userInput, sizeof(char) * (2048+1), stdin);
	fflush(stdout);	
	
	// overwrite the new line icharacter with 0
	// source: http://sekrit.de/webdocs/c/beginners-guide-away-from-scanf.html
	(*userInput)[strcspn(*userInput, "\n")] = 0;

	// Checking if newline was entered OR user entered a comment as FIRST CHARACTER
	if((strlen(*userInput) == 1 && (*userInput)[0] == '\n') || (*userInput)[0]== '#'){
		return 0;		
	}
	
	// Moving past leading spaces, if there are any, to see if we have a comment or new line
	else if ((*userInput)[0] == ' '){		
		countBlanks += 1;
		countChars += 1;
		i += 1;		
		while((*userInput)[i] == ' '){		
			i += 1;		
			countBlanks += 1;
			countChars += 1;
		}		
		// Found a comment or new line after the space or end of string - NOT a string
		if((*userInput)[i] == '#' || (*userInput)[i] == '\0' || (*userInput)[i] == '\n'){
			return 0;
		}
	}
	
	// else, continue checking the string
	else {	

		while((*userInput)[i] != '\0'){				
		
			// --- Space (i.e. blank)
			if((*userInput)[i] == ' '){
				countBlanks += 1;
			}  
			// --- If we move on to a non dollar sign space, reset counter
			else if ((*userInput)[i] != '$' && countDollarSigns == 1){
				countDollarSigns = 0;
			}
			// --- Dollar Signs Check - 2 in a row, expand & replace w/ PID
			else if ((*userInput)[i] == '$'){
			
				// increment dollar signs
				countDollarSigns += 1;				
				
				// Need to expand user input with smallsh PID for $$ in string
				// ONLY if we have two in a row
				
				// -- EXPAND
				if(countDollarSigns == 2 && (*userInput)[i-1] == '$'){	
				
					countDollarSigns = 0; //reset
					
					// make a new string array, that is strlen(PID) - 2 bigger than original plus the null character at the end
					int newStrLen = strlen(*userInput) + strlen(shellPID) - 2 + 1;
					char* newStr = calloc(newStrLen, sizeof(char));	
					//printf("\nNew String Length: %i", newStrLen);
					
					// --- three cases:					
					
					// 1) fill in values BEFORE first $ in string with original
					int j;
					for(j = 0; j < (i - 1); j++){
						newStr[j] = (*userInput)[j];
						//printf("\n $-Expansion - -> before: %c", newStr[j]);
					}
					
					// 2) fill in PID values starting at first $ to end of PID length
					int idx = 0;
					for(j = (i-1); j < (i + (int)strlen(shellPID)); j++){
						newStr[j] = shellPID[idx];
						//printf("\n $-Expansion - -> PID: %c", newStr[j]);
						idx += 1;						
					}
					
					// 3) fill in original values AFTER last PID # in new str
					idx = i+1; // start 1 after current place which is 2nd $
					for(j = (i + (int)strlen(shellPID)); j < newStrLen; j++){
						newStr[j] = (*userInput)[idx];
						//printf("\n $-Expansion - -> after: %c", newStr[j]);
						idx += 1;
					}
					
					newStr[newStrLen] = '\0'; // null terminator	
					
					// Set userInput to new string, free old user input
					char* temp = *userInput;
					*userInput = newStr;
					newStr = temp;
					free(newStr);
					
					// redefine i to be at last digit of newly added PID #
					// next i increment before while loops adds 1 more
					i = (i-2) + strlen(shellPID);
					
				}
				// DO NOT expand
				else if (countDollarSigns == 2 && (*userInput)[i-1] != '$'){
					countDollarSigns = 0; // reset				
				}		
				 
			}			
			// increment char counter indices
			i += 1;
			countChars += 1;	
		} // end while 
		
		
		// Checking if all spaces/blanks
		if(countBlanks == countChars){
			//printf("\nBlanks: %i, Chars: %i", countBlanks, countChars);
			return 0;	
		}		
		
	} // end else if	
	
	return 1;
}

/*--------------------------------------------------------------------- 
Purpose: Convert Shell PID to String Array
Input: pid_t PID ID # of shell
Output: Character array pointer
Source(s): https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c#:~:text=You%20can%20use%20itoa(),to%20convert%20any%20value%20beforehand.
----------------------------------------------------------------------*/
void pidToString(char** pidStr, int pid, int countDigits){	
	// need to save room for the null terminator at the end, hence +1
	printf("\nPID = %i, digits = %i", pid, countDigits);
	*pidStr = calloc(countDigits+1, sizeof(char));
	// THIS IS WHERE I WAS WRONG!!!!! THE SIZE WAS NUMBER OF BYTES
	// NOT JUST NUMBER OF ITEMS!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// FIXED ON NOVEMBER 22, 2020!!!!!!!!!!!!!!!!!!!!!!!!
	snprintf(*pidStr, (countDigits) * sizeof(int), "%i", pid);
	fflush(stdout);
	pidStr[countDigits+1] = '\0';		
	printf("\nPID = %s", *pidStr);
}

/*--------------------------------------------------------------------- 
Purpose: Count digits in an integer
Input: a long long integer
Output: an integer
Source: https://codeforwin.org/2016/10/c-program-to-count-number-of-digits-in-number.html
----------------------------------------------------------------------*/
int countDigits(long long input){
	int count = 0;	
	// divide by 10 each time until num is zero, when we've
	// hit the "bottom"
	// example: 100. 100/10, count = 1, 10/10, count = 2, 1/10, count = 3
	do{
		count++;
		input /= 10;
	} while(input!=0);	
	return count;
}	