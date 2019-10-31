////////////////////////////////////////////////////////////////////////////////
// Main File:        check_queens.c
// This File:        check_queens.c
// Other Files:      N/A
// Semester:         CS 354 Fall 2018
//
// Author:           Austin Wilson
// Email:            awilson34@wisc.edu
// CS Login:         austinw
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          N/A
//
// Online sources:   N/A
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h> 
#include <stdlib.h>  
#include <string.h>   

char *COMMA = ",";

/* Retrieves from the first line of the input file,
 * the number of rows and columns for the board.
 * 
 * fp: file pointer for input file
 * rows: pointer to number of rows
 * cols: pointer to number of columns
 */
void get_dimensions(FILE *fp, int *rows, int *cols) {     
	char *line = NULL; 
	size_t len = 0;
	if (getline(&line, &len, fp) == -1) { 
		printf("Error in reading the file\n");
		exit(1); 
	}

	char *token = NULL; 
	token = strtok(line, COMMA);
	*rows = atoi(token); 

	token = strtok(NULL, COMMA); 
	*cols = atoi(token);
}      


/* Returns 1 if and only if there exists at least one pair
 * of queens that can attack each other.
 * Otherwise returns 0.
 * 
 * board: heap allocated 2D board
 * rows: number of rows
 * cols: number of columns
 */
int check_queens(int **board, int rows, int cols) {           
         int colCounter = 0;
	 int rowCounter = 0;
	 //Loop to check rows for multiple queens
	 for (int i = 0; i < rows; i++) {
		 //Loop to check cols for multiple queens
		 for (int j = 0; j < cols; j++) {
			 if (*(*(board + i) + j) == 1) {
				 colCounter = j + 1;
				 rowCounter = i + 1;
				 //Loop checks rest of row for a queen
				 while(colCounter < cols) {
					 if (*(*(board + i) + colCounter) == 1) {
						 return 1;
					 }
					 colCounter++;
				 }
				 //Loop checks rest of the column for a queen
				 while(rowCounter < rows) {
					 if(*(*(board + rowCounter) + j) == 1) {
						 return 1;
					 }
					 rowCounter++;
				 }
				 //Loop checks diagonal for a queen
				 rowCounter = i;
				 colCounter = j;
				 while (rowCounter < (rows - 1) && colCounter < (cols - 1)) {
					 rowCounter++;
					 colCounter++;
					 if (*(*(board + rowCounter) + colCounter) == 1) {
						 return 1;
					 }
			         }  
		        }
		 }
	 }
	 return 0;
}

/* This program prints true if the input file has any pair
 * of queens that can attack each other, and false otherwise
 * 
 * argc: CLA count
 * argv: CLA value
 * return 0 to show the program exited successfully 
 */
int main(int argc, char *argv[]) {                
        //Check if number of command-line arguments is correct.
	if (argc != 2) {
            printf("Usage: ./check_queens <input_filename>");
	    exit(1);
	}

	//Open the file and check if it opened successfully.
	FILE *fp = fopen(*(argv + 1), "r");
	if (fp == NULL) {
		printf("Cannot open file for reading\n");
		exit(1);
	}

        //Declare local variables.
        int rows, cols;

        //Call get_dimensions to retrieve the board dimensions.i
	get_dimensions(fp, &rows, &cols);

        //Dynamically allocate a 2D array of dimensions retrieved above.
	int **board;
	    board = malloc(sizeof(int*)*rows);
	    if (board != NULL) {
	        for (int i = 0; i < rows; i++) {
	      	    *(board + i) = malloc(sizeof(int*)*cols);
		    if (*(board + i) == NULL) {
		        printf("Error allocating memory.");
			exit(1);
		    } 
	        }
	    }	
	    else {
		printf("Error allocating memory.");    
		exit(1);
	    }

	//Read the file line by line.
	//Tokenize each line wrt comma to store the values in your 2D array.
	char *line = NULL;
	size_t len = 0;
	char *token = NULL;
	for (int i = 0; i < rows; i++) {

		if (getline(&line, &len, fp) == -1) {
			printf("Error while reading the file\n");
			exit(1);	
		}

		token = strtok(line, COMMA);
		//Initialize 2D array
		for (int j = 0; j < cols; j++) {
			*(*(board + i) + j) = atoi(token); 
			token = strtok(NULL, COMMA);	
		}
	}

	//Call the function check_queens and print the appropriate
        //output depending on the function's return value.
	if (check_queens(board, rows, cols) == 1) {
                printf("True\n");
	}
	else {
	        printf("False\n");
	}
        
	//Old attempt below, want to keep for studying purposes
	//free(*(board + 1));
	//free(*board);
	//free(board);
        //board = NULL;

	//Free all dynamically allocated memory
	for (int i = (rows - 1); i > -1; i--) {
		free(*(board + i));
	}
	free(board);
	board = NULL;

	//Close the file.
	if (fclose(fp) != 0) {
		printf("Error while closing the file\n");
		exit(1);	
	}

	return 0;
}    
