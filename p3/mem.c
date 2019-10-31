////////////////////////////////////////////////////////////////////////////////
// Main File:        mem.c
// This File:        mem.c
// Other Files:      mem.c, mem.h
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
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//                   N/A
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
//                   1. I had to look up bitwise negation operator (~) to
//                   	remind myself a simple and better looking way to write
//                   	the value. (~0x03) is cleaner than 0xffffffc
//                   	url "https://www.geeksforgeeks.org/operators-c-c/"
//////////////////////////// 80 columns wide ///////////////////////////////////
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "mem.h"

/*
 * This structure serves as the header for each allocated and free block
 * It also serves as the footer for each free block
 * The blocks are ordered in the increasing order of addresses 
 */
typedef struct blk_hdr {                         
        int size_status;
  
    /*
    * Size of the block is always a multiple of 8
    * => last two bits are always zero - can be used to store other information
    *
    * LSB -> Least Significant Bit (Last Bit)
    * SLB -> Second Last Bit 
    * LSB = 0 => free block
    * LSB = 1 => allocated/busy block
    * SLB = 0 => previous block is free
    * SLB = 1 => previous block is allocated/busy
    * 
    * When used as the footer the last two bits should be zero
    */

    /*
    * Examples:
    * 
    * For a busy block with a payload of 20 bytes (i.e. 20 bytes data + an additional 4 bytes for header)
    * Header:
    * If the previous block is allocated, size_status should be set to 27
    * If the previous block is free, size_status should be set to 25
    * 
    * For a free block of size 24 bytes (including 4 bytes for header + 4 bytes for footer)
    * Header:
    * If the previous block is allocated, size_status should be set to 26
    * If the previous block is free, size_status should be set to 24
    * Footer:
    * size_status should be 24
    * 
    */
} blk_hdr;

/* Global variable - This will always point to the first block
 * i.e. the block with the lowest address */
blk_hdr *first_blk = NULL;

/*
 * Note: 
 *  The end of the available memory can be determined using end_mark
 *  The size_status of end_mark has a value of 1
 *
 */

/* 
 * Function for allocating 'size' bytes
 * Returns address of allocated block on success 
 * Returns NULL on failure 
 * Here is what this function should accomplish 
 * - Check for sanity of size - Return NULL when appropriate 
 * - Round up size to a multiple of 8 
 * - Traverse the list of blocks and allocate the beist free block which can accommodate the requested size 
 * - Also, when allocating a block - split it into two blocks
 * Tips: Be careful with pointer arithmetic 
 */                    
void* Alloc_Mem(int size) {                      
	//Check size of requested memory is positive
	if (size <= 0) {
		return NULL;
    	}
    	//Check that Mem_Init was called
	if (first_blk == NULL) {
	    return NULL;
	}
	//Add 4 to account for the header size
    	size += 4;
    	//Check for 8 bit alignment
    	if (size % 8 != 0) {
	    //Add padding
	    size = size + (8 - (size % 8));
	}
    	//Ptr to track header 
    	void* header = first_blk;
    	//Keep track of the best fit header
   	void* best_fit = NULL;
   	//Track size of best fit header to determine for splitting later in program
   	int best_fit_size = 0;
    	//Main loop to search heap array for free block
    	while (*(int*)header != 1) {
	
		//Create new header which will be the new allocated block header   
		blk_hdr* new_header = (blk_hdr*)header;
		//Tracks size of the new allocated block
		int block_size = new_header->size_status & (~0x3);
		//If the next header is free
		if (!(new_header->size_status & 0x1)) {
			//If block is perfect size for requested block size
	    		if (block_size == size) {
				new_header->size_status = new_header->size_status + 3;
				blk_hdr *next_header = header + size;
				//Check to see if next block is end of heap
				if (next_header->size_status != 1) {
		    			//Set p bit to show prev block is allocated
		    			next_header->size_status += 2;
				}
				//Return block to user, giving them address of payload
				return header + sizeof(blk_hdr);
	    		}	
	    		//Block is larger than requested
	    		else if (block_size >= size + 8) {
				//If block is better than prev free block or no best fit block has been tracked yet
				if (block_size < best_fit_size || best_fit_size == 0) {
		    			best_fit_size = block_size;
		   			best_fit = header;
				}
	    		}
		}		
		//Else move forward to the next block
		header += block_size;
    	}
	//Splitting block
	if (best_fit != NULL) {
	    //Create allocated block
	    blk_hdr *alloc_header = best_fit;
	    alloc_header->size_status = size + 3;
	    //Create new free block
	    blk_hdr *free_block = best_fit + size;
	    free_block->size_status = (best_fit_size - size) + 2;
	    //Create footer for new free block
	    *(int*)(best_fit + best_fit_size - 4) = best_fit - size;
	    //Return the payload of the freshly allocated block to the user
	    return best_fit + sizeof(blk_hdr);
	}
    	return NULL;
}

/* 
 * Function for freeing up a previously allocated block 
 * Argument - ptr: Address of the block to be freed up 
 * Returns 0 on success 
 * Returns -1 on failure 
 * Here is what this function should accomplish 
 * - Return -1 if ptr is NULL
 * - Return -1 if ptr is not 8 byte aligned or if the block is already freed
 * - Mark the block as free 
 * - Coalesce if one or both of the immediate neighbours are free 
 */                    
int Free_Mem(void *ptr) {
	//Checking for null pointer	
    	if (ptr == NULL) {
		return -1;
    	}
    	//Check double word alignment
    	if ((int)ptr % 8 != 0) {
		return -1;
    	}
    	//Tracks free block header based on given pointer variable
    	blk_hdr *header = ptr - 4;
    	//Checking for block that has been already freed
    	if (!(header->size_status & 0x1)) {
		return -1;	
   	}
    	//Get the size of block by ignoring a and p bit
    	int free_block_size = header->size_status & (~0x3);
    	//Used to simplify math by being placeholder for current address
    	void* curr_ptr = header;
    	//Tracks successor header 
    	blk_hdr *succ = curr_ptr + free_block_size;
    	//Checking successor block, if free, add to free list
    	if (!(succ->size_status & 0x1)) {
		free_block_size += (succ->size_status & (~0x3));
   	}
   	//If allocated, set p bit to 0
   	else {
		succ->size_status = ((succ->size_status & (~0x3)) + 1);
   	}
   	//Checking previous block, if free, add to free list
   	if (!(header->size_status & 0x02)) {
		int prev_footer = *(int*)(header - 1);
		curr_ptr -= prev_footer;
		free_block_size += prev_footer;
  	}
   	//curr_ptr points to beginning of new free block
   	//free_block_size contains the size of the new free block
   	//Now create new free block 
   	blk_hdr *free_block = curr_ptr;
   	free_block->size_status = free_block_size + 2;
   	*(int*)(curr_ptr + free_block_size - 4) = free_block_size;
   	//Return 0 to show everthing worked correctly
	return 0;
}

/*
 * Function used to initialize the memory allocator
 * Not intended to be called more than once by a program
 * Argument - sizeOfRegion: Specifies the size of the chunk which needs to be allocated
 * Returns 0 on success and -1 on failure 
 */                    
int Init_Mem(int sizeOfRegion)
{                         
    int pagesize;
    int padsize;
    int fd;
    int alloc_size;
    void* space_ptr;
    blk_hdr* end_mark;
    static int allocated_once = 0;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: Init_Mem has allocated space during a previous call\n");
        return -1;
    }
    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, 
                    fd, 0);
    if (MAP_FAILED == space_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
     allocated_once = 1;

    // for double word alignement and end mark
    alloc_size -= 8;

    // To begin with there is only one big free block
    // initialize heap so that first block meets 
    // double word alignement requirement
    first_blk = (blk_hdr*) space_ptr + 1;
    end_mark = (blk_hdr*)((void*)first_blk + alloc_size);
  
    // Setting up the header
    first_blk->size_status = alloc_size;

    // Marking the previous block as busy
    first_blk->size_status += 2;

    // Setting up the end mark and marking it as busy
    end_mark->size_status = 1;

    // Setting up the footer
    blk_hdr *footer = (blk_hdr*) ((char*)first_blk + alloc_size - 4);
    footer->size_status = alloc_size;
  
    return 0;
}

/* 
 * Function to be used for debugging 
 * Prints out a list of all the blocks along with the following information i
 * for each block 
 * No.      : serial number of the block 
 * Status   : free/busy 
 * Prev     : status of previous block free/busy
 * t_Begin  : address of the first byte in the block (this is where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block (as stored in the block header) (including the header/footer)
 */                     
void Dump_Mem() {                        
    int counter;
    char status[5];
    char p_status[5];
    char *t_begin = NULL;
    char *t_end = NULL;
    int t_size;

    blk_hdr *current = first_blk;
    counter = 1;

    int busy_size = 0;
    int free_size = 0;
    int is_busy = -1;

    fprintf(stdout, "************************************Block list***\
                    ********************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, "-------------------------------------------------\
                    --------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => busy block
            strcpy(status, "Busy");
            is_busy = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "Free");
            is_busy = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "Busy");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "Free");
        }

        if (is_busy) 
            busy_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%d\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (blk_hdr*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, "---------------------------------------------------\
                    ------------------------------\n");
    fprintf(stdout, "***************************************************\
                    ******************************\n");
    fprintf(stdout, "Total busy size = %d\n", busy_size);
    fprintf(stdout, "Total free size = %d\n", free_size);
    fprintf(stdout, "Total size = %d\n", busy_size + free_size);
    fprintf(stdout, "***************************************************\
                    ******************************\n");
    fflush(stdout);

    return;
}
