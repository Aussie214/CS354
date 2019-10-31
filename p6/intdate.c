////////////////////////////////////////////////////////////////////////////////
// Main File:        intdate.c
// This File:        intdate.c
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
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

time_t user_time;
int user_sig = 0;

void handler(int a) {
        user_time = time(NULL);
        //if (*(user_time) == NULL) {
	//	printf("error reading time. exiting now.");
	//}
	printf("PID: ");
        printf("%d", getpid());
        printf(" | Current Time: ");
	if (*(ctime(&user_time)) == -1) {
		printf("error reading time. exiting now.");
		exit(1);
	}
	else {
        	printf("%s", ctime(&user_time));
	}
        alarm(3);
}

void user_handler(int b) {
	if (b == SIGUSR1) {
		user_sig++;
		printf("SIGUSR1 caught!\n");
	}
}

void sig_int(int c) {
	if (c == SIGINT) {
		printf("SIGINT recieved.");
		printf("\n%s", "SIGUSR1 was recieved ");
	        printf("%d", user_sig);
	        printf("%s", " times. Exiting now.\n");
	exit(0);
	}
}

int main() {
	printf("Pid and time will be printed every 3 seconds.");
	printf("\nEnter ^C to end the program.\n");
	//SIGINT Signal
	struct sigaction signal_int;
	memset (&signal_int, 0, sizeof(signal_int));
	signal_int.sa_handler = sig_int;
	if (sigaction(SIGINT, &signal_int, NULL) == -1) {
		printf("sigaction failed. Exiting now.");
	}
	//User Signal
	struct sigaction usr_act;
	memset (&usr_act, 0, sizeof(usr_act));
	usr_act.sa_handler = user_handler;
	if (sigaction(SIGUSR1, &usr_act, NULL) == -1) {
		printf("sigaction failed. Exiting now.");
		exit(1);
	}
	//Alarm Signal
	struct sigaction act;
	memset (&act, 0, sizeof(act)); 
	act.sa_handler = handler; 
	if (sigaction(SIGALRM, &act, NULL) == -1) {
		printf("sigaction failed. Exiting program.");
		exit(1);
	}
        alarm(3);
	while (1) {
	}
	//free(act);
	//free(usr_act);
	//free(signal_int);
	return 0;
}
