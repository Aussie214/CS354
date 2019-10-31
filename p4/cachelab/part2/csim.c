/* Name:Austin Wilson
 * CS login:austinw
 * Section(s):Lecture 002
 *
 * csim.c - A cache simulator that can replay traces from Valgrind
 *     and output statistics such as number of hits, misses, and
 *     evictions.  The replacement policy is LRU.
 *
 * Implementation and assumptions:
 *  1. Each load/store can cause at most one cache miss plus a possible eviction.
 *  2. Instruction loads (I) are ignored.
 *  3. Data modify (M) is treated as a load followed by a store to the same
 *  address. Hence, an M operation can result in two cache hits, or a miss and a
 *  hit plus a possible eviction.
 *
 * The function print_summary() is given to print output.
 * Please use this function to print the number of hits, misses and evictions.
 * This is crucial for the driver to evaluate your work. 
 */

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

/****************************************************************************/
/***** DO NOT MODIFY THESE VARIABLE NAMES ***********************************/

/* Globals set by command line args */
int s = 0; /* set index bits */
int E = 0; /* associativity */
int b = 0; /* block offset bits */
int verbosity = 0; /* print trace if set */
char* trace_file = NULL;

/* Derived from command line args */
int B; /* block size (bytes) B = 2^b */
int S; /* number of sets S = 2^s In C, you can use the left shift operator */

/* Counters used to record cache statistics */
int hit_cnt = 0;
int miss_cnt = 0;
int evict_cnt = 0;
/*****************************************************************************/


/* Type: Memory address 
 * Use this type whenever dealing with addresses or address masks
 */                    
typedef unsigned long long int mem_addr_t;

/* NOTE: 
 * You might (not necessarily though) want to add an extra field to this struct
 * depending on your implementation
 * 
 * For example, to use a linked list based LRU,
 * you might want to have a field "struct cache_line * next" in the struct 
 */                    
typedef struct cache_line {                    
    int counter;
    char valid;
    mem_addr_t tag;
    struct cache_line * next;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;


/* The cache we are simulating */
cache_t cache;  

/* init_cache - 
 * Allocate data structures to hold info regrading the sets and cache lines
 * use struct "cache_line_t" here
 * Initialize valid and tag field with 0s.
 * use S (= 2^s) and E while allocating the data structures here
 */                    
void init_cache() {
	S = 1 << s;
	B = 1 << b;
	//Create first level of cache array
	cache = malloc(S * sizeof(cache_set_t));
	if (cache == NULL) {
		printf("Cache failed to allocate memory.");
		exit(1);
	}
	//Create second level of cache array
	for (int i = 0; i < S; i++) {
		cache_set_t set = malloc(sizeof(cache_line_t));
		if (set == NULL) {
			printf("Cache failed to allocate memory for cache sets.");
			exit(1);
		}
		cache_line_t* curr_line = set;
		//Add lines to set
		for (int j = 1; j < E; j++) {
			curr_line->next = malloc(sizeof(cache_line_t));
			if (curr_line->next == NULL) {
				printf("Error allocating memory for lines in set");
				exit(1);
			}	
			//Initialize data fields
			curr_line = curr_line->next;
			curr_line->valid = 0;
			curr_line->tag = 0;
		}
		//place set in cache
		cache[i] = set;
	}
}


/*  free_cache - free each piece of memory you allocated using malloc 
 * inside init_cache() function
 */                    
void free_cache() { 
	cache_line_t* curr_line = NULL;
	cache_line_t* next_line = NULL;
	//free sets
	for (int i = 0; i < S; i++) {
		curr_line = cache[i];
		while (curr_line != NULL) {
			next_line = curr_line->next;
			free(curr_line);
			curr_line = next_line;
		}
	}
	//free cache
	free(cache);
	cache = NULL;
}

/* compute the least recently used line in the given set
 * and return a pointer to that line
 */
cache_line_t* compute_LRU(cache_line_t* first_line) {	
	int LRU = first_line->counter;
	cache_line_t* LRU_line = first_line;
	while (first_line != NULL) {
		int line_cnt = first_line->counter;
		if (line_cnt < LRU) {
			LRU = line_cnt;
			LRU_line = first_line;
		}
		first_line = first_line->next;
	}
	return LRU_line;
}

/* get the max counter from lines in a set
 *
 */
int get_Max(cache_line_t* line) {
	int old_counter = 0;
	while (line != NULL) {
		int counter = line->counter;
	 	if (counter > old_counter) {
			old_counter = counter;
		}	
		line = line->next;
	}
	return old_counter++;
}

/*   access_data - Access data at memory address addr.
 *   If it is already in cache, increase hit_cnt
 *   If it is not in cache, bring it in cache, increase miss count.
 *   Also increase evict_cnt if a line is evicted.
 *   you will manipulate data structures allocated in init_cache() here
 */                    
void access_data(mem_addr_t addr) {
//	printf("got to access_data");
	//Set up vars
	int hit = 0;
	mem_addr_t s_bits = addr >> b;
	mem_addr_t set_num = s_bits & (S - 1);
	mem_addr_t t_bits = addr >> (b + s);
	mem_addr_t tag = t_bits & ((int)pow(2, 64 - (b + s)) - 1);
	//Set up lines
	cache_line_t* first_line = cache[set_num];
	cache_line_t* curr_line = first_line;
	mem_addr_t curr;
	cache_line_t* to_replace = NULL;
	//Update counter
	int updated_counter = get_Max(curr_line);
	//Search lines in set
	while (curr_line != NULL) {
		curr = curr_line->tag;
		//curr >>= (s + b);
		//Hit
		if (curr == tag && curr_line->valid) {
			hit = 1;
			to_replace = curr_line;
		break;	
		}
		if (!to_replace && curr_line->valid == 0) {
			to_replace = curr_line;
		}
		curr_line = curr_line->next;
	}
	//Hit
	if (hit) {
		to_replace->counter = updated_counter;
		hit_cnt++;
		if (verbosity) {
			printf("hit ");
		}
		return;
	}
	//Miss
	else {
		miss_cnt++;
		//No eviction
		if (to_replace != NULL) {
			if (verbosity) {
				printf("miss ");
			}
			cache_line_t* current = first_line;
			while (current->next != NULL)
			{
				if(current->valid == '0')
				{
					to_replace = current;
					to_replace->tag = addr;
                        		to_replace->valid = 1;
                        		to_replace->counter = updated_counter;
                        		return;
				}
				current = current->next;
			}
		}
		//Eviction
		to_replace = compute_LRU(first_line);
		evict_cnt++;
		if (verbosity) {
			printf("eviction ");
		}
		to_replace->tag = addr;
                to_replace->valid = 1;
                to_replace->counter = updated_counter;
	}
}

/* replay_trace - replays the given trace file against the cache 
 * reads the input trace file line by line
 * extracts the type of each memory access : L/S/M
 * YOU MUST TRANSLATE one "L" as a load i.e. 1 memory access
 * YOU MUST TRANSLATE one "S" as a store i.e. 1 memory access
 * YOU MUST TRANSLATE one "M" as a load followed by a store i.e. 2 memory accesses 
 */                    
void replay_trace(char* trace_fn) {                      
    char buf[1000];
    mem_addr_t addr = 0;
    unsigned int len = 0;
    FILE* trace_fp = fopen(trace_fn, "r");

    if (!trace_fp) {
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while (fgets(buf, 1000, trace_fp) != NULL) {
        if (buf[1] == 'S' || buf[1] == 'L' || buf[1] == 'M') {
            sscanf(buf+3, "%llx,%u", &addr, &len);
      
            if (verbosity) {
                printf("%c %llx,%u ", buf[1], addr, len);
	    }
	    access_data(addr);
	    if (buf[1] == 'M') {
		access_data(addr);
	    }

            if (verbosity)
                printf("\n");
        }
    }

    fclose(trace_fp);
}

/*
 * print_usage - Print usage info
 */                    
void print_usage(char* argv[]) {                 
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

/*
 * print_summary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded.
 */                    
void print_summary(int hits, int misses, int evictions) {                
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}

/*
 * main - Main routine 
 */                    
int main(int argc, char* argv[]) {                      
    char c;
    
    // Parse the command line arguments: -h, -v, -s, -E, -b, -t 
    while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (c) {
            case 'b':
                b = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'h':
                print_usage(argv);
                exit(0);
            case 's':
                s = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 'v':
                verbosity = 1;
                break;
            default:
                print_usage(argv);
                exit(1);
        }
    }

    /* Make sure that all required command line args were specified */
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        print_usage(argv);
        exit(1);
    }

    /* Initialize cache */
    init_cache();

    replay_trace(trace_file);

    /* Free allocated memory */
    free_cache();

    /* Output the hit and miss statistics for the autograder */
    print_summary(hit_cnt, miss_cnt, evict_cnt);
    return 0;
}
