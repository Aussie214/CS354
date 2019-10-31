////////////////////////////////////////////////////////////////////////////////
// Main File:        sendsig.c
// This File:        sendsig.c
// Other Files:      N/A
// Semester:         CS 354 Fall 2018 002
//
// Author:           Austin Wilson
// Email:            awilson34@wisc.edu
// CS Login:         austinw
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
//////////////////////////// 80 columns wide ///////////////////////////////////

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Usage: <signal type> <pid>\n");
		exit(1);
	}
	if (!(strcmp(argv[1], "-u"))) {
		if (kill(atoi(argv[2]), SIGUSR1) == -1) {
			printf("kill returned an error. exiting now");
			exit(1);
		}
	}       
	else if (!(strcmp(argv[1],"-i"))) {
		if (kill(atoi(argv[2]), SIGINT) == -1) {
			printf("kill returned an error. exiting now");
                        exit(1);
		}
	}
	else {
		printf("Usage: <signal type> <pid>");
		exit(1);
	}	
	return 0;
}
