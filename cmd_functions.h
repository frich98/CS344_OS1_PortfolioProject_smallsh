// CS 344 - OS - Fall 2020
// Assignment 3, smallsh
// FUNCTION PROTOTYPES
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

#ifndef CMD_FUNCTIONS_H
#define CMD_FUNCTIONS_H


/*--------------------------------------------------------------------- 
Purpose: A struct to hold user command information
Variables Included:
full command string
command
arg character string array, 512 max, putting 513 for extra '\0' indicator at end
input file if applicable
output file if applicable
& sign true (1) or false (0)
----------------------------------------------------------------------*/
struct cmd
{
	char* 	fullCmdStr;
	char* 	command;
	char** 	commandArgs;  // end arg will == '\0'
	int 	countArgs;
	char* 	inputFile;
	char* 	outputFile;
	int 	ampersandTF;	
	struct 	cmd *next;
};


/*--------------------------------------------------------------------- 
Purpose: Fill Out Struct of Command
Input: user entered command string AFTER $$ expansion, 
format of input -->   
command [arg1 arg2 ...] [< input_file] [> output_file] [&]
Output: cmd struct pointer
Source(s):
1) Code for movie struct from Assignment 1 and 2 as baseline
----------------------------------------------------------------------*/
struct cmd *createCMD(char *cmdInput);

/*--------------------------------------------------------------------- 
Purpose: Built-In Command # 3 (CD), change working directory of smallsh
        with no args, changes curr wd to directory specified in HOME environment
		variable. assume input to function will have length 1 and have '\0' as its
		only character
		if one arg is given, it is the path of a directory to change to which 
		can be either relative, or absolute
Input: user provided argument, can be blank or filled in 
Output: no output, void
Source(s):
----------------------------------------------------------------------*/
void changeDirectory(char* path);

/*--------------------------------------------------------------------- 
Purpose: Free a cmd struct
Input: pointer to cmd struct 
Output: void
Source(s):
1) Code for movie struct deleteMovie from Assignment 1 and 2 as baseline
----------------------------------------------------------------------*/
void deleteCmd(struct cmd *acmd);

/*--------------------------------------------------------------------- 
Purpose: Return String of User Input, AFTER $$ expansion
Input: double char pointer userInput, will fill out in this function
       string shellPID number
Output: int value. if command is a valid string, return 1.
        else, return 0 (#, blank lines, all spaces)
----------------------------------------------------------------------*/
int getUserCommand(char** userInput, char* shellPID);

/*--------------------------------------------------------------------- 
Purpose: Convert Shell PID to String Array
Input: pid_t PID ID # of shell
Output: Character array pointer
----------------------------------------------------------------------*/
void pidToString(char** pidStr, int pid, int countDigits);

/*--------------------------------------------------------------------- 
Purpose: Count digits in an integer
Input: a long long integer
Output: an integer
Source: https://codeforwin.org/2016/10/c-program-to-count-number-of-digits-in-number.html
----------------------------------------------------------------------*/
int countDigits(long long input);

#endif