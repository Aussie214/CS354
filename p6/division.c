////////////////////////////////////////////////////////////////////////////////
// Main File:        division.c
// This File:        division.c
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
#include <stdlib.h>
#include <signal.h>
#include <string.h>

int total = 0;
//Handlers
void handler_exit(int b) {
        if (b == SIGINT) {
        	printf("\nTotal number of operations successfully completed: ");
                printf("%d", total);
                printf("\nThe program will be terminated.\n");
		exit(0);
        }
}
void handler_fpe(int a) {
        if (a == SIGFPE){
                printf("\nError: a division by 0 operation was attempted.");
                printf("\nTotal number of operations completed successfully: ");
                printf("%d", total);
                printf("\nThe program will be terminated.\n");
                exit(0);
        }
}
int main(int argc, char *argv[]) {
	char int_one[100];
	char int_two[100];
	int one;
	int two;
	//SIGFPE Handler
	struct sigaction sig_fpe;
        memset (&sig_fpe, 0, sizeof(sig_fpe));
        sig_fpe.sa_handler = handler_fpe;
        if (sigaction(SIGFPE, &sig_fpe, NULL) == -1) {
		printf("sigaction failed. Exiting now.");
		exit(1);
	}
	//^C Handler
	struct sigaction sig_exit;
        memset (&sig_exit, 0, sizeof(sig_exit));
        sig_exit.sa_handler = handler_exit;
        if (sigaction(SIGINT, &sig_exit, NULL) == -1) {
		printf("sigaction failed. Exiting now.");
	}
	while(1) {
		//Grab numbers from user
		printf("Enter first integer: ");
		if (fgets(int_one, 100, stdin) == NULL) {
			printf("fgets returned an error. exiting now.");
			exit(1);
		}
		one = atoi(int_one);
		printf("Enter second integer: ");
		if (fgets(int_two, 100, stdin) == NULL) {
			printf("fgets returned an error. exiting now.");
			exit(1);
		}
		two = atoi(int_two);
		//Calculate result and print it out
		printf("%d", one);
		printf(" / ");
		printf("%d", two);
		printf(" is ");
		printf("%d", one/two);
		printf(" with a remainder of ");
		printf("%d", one % two);
		printf("\n");
		total++;
	}
}
