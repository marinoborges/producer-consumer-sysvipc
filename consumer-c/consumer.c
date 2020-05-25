/**
* FILENAME:
*		consumer.c
*
* DESCRIPTION :
*       Read shared memory segment using SysV IPC
*
* AUTHOR:
*		Marino Borges
*
* CREATED:
*		05.17.2020
*
* CHANGES:
*		05.18.2020 producer-consumer-sysvipc-0.2
*			fixed CONTENT_MEMORY_SIZE in 5MB for less memory operations
*		
*		05.18.2020 producer-consumer-sysvipc-0.3
*			optimized producer.py imports
* 
**/

#include <sys/ipc.h>		/* for system's IPC_xxx definitions */
#include <sys/shm.h>		/* for shmget, shmat, shmdt, shmctl */
#include <sys/sem.h>		/* for semget, semctl, semop */

#include <signal.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h>
#include <string.h>
#include <unistd.h> 
#include <stdbool.h> 
#include <time.h>

#include "md5.h"
#include "utils.h"

// Shared memory parameters
#define CONTENT_SEMAPHORE_KEY 42
#define CONTENT_MEMORY_KEY 42
#define CONTENTSIZE_SEMAPHORE_KEY 44
#define CONTENTSIZE_MEMORY_KEY 44
#define CONTENTSIZE_MEMORY_SIZE 10
#define CONTENT_MEMORY_SIZE 5000000
#define PERMISSIONS 0600

static const char MY_NAME[] = "consumer";
static int volatile keepRunning = 1;
clock_t start, end;
double cpu_time_used;

void sig_handler(int signo)
{
  if (signo == SIGINT){
    printf("received SIGINT\n");
	keepRunning = 0;
	return;
  }
  if (signo == SIGTERM){
    printf("received SIGTERM\n");
	keepRunning = 0;
  }
  return;
}

int main(int argc, char *argv[]){
    int contentsize_semaphore_id = 0;
    int contentsize_memory_id = 0;
	int content_semaphore_id = 0;
	int content_memory_id = 0;
    int rc;
	int i = 1;
	const char outputdir[100] = "output";
	const char file_basename[100] = "file";
	const char filetype[5] = ".jpg";
	char filename[200], outputfile[300];
    char s[1024];
    char md5ified_message[256];
    void *content_address = NULL;
	void *contentsize_address = NULL;
	char *ptr;
	int contentsize = 0;
	bool verbose = false;
	bool check_md5 = false;
	char opt;

	while ((opt = getopt(argc, argv, "vmh")) != -1) {
        switch (opt) {
        case 'v': verbose = true; break;
        case 'm': check_md5 = true; break;
		case 'h':
			fprintf(stderr, "Usage: %s [-h] [-v] [-m]\n\n", argv[0]);
			fprintf(stderr, "Read shared memory segment using SysV IPC (check MD5 hash optionally)\nWrite file with shared memory content\n\n");
			fprintf(stderr, "optional arguments:\n\t-h\tshow this help message and exit\n");
			fprintf(stderr, "\t-v\tset verbose flag (default: false)\n");
			fprintf(stderr, "\t-m\tset check MD5 flag (default: false)\n");
            exit(EXIT_SUCCESS);
			break;
        default:
			fprintf(stderr, "Try \"%s -h\" for more information\n", argv[0]);
			exit(EXIT_FAILURE);
            break;
        }
    }

	// Catch SIGINT
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");
	// Catch SIGTERM
	if (signal(SIGTERM, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGTERM\n");

    // get a handle to the semaphores
    contentsize_semaphore_id = get_semaphore_id(MY_NAME, CONTENTSIZE_SEMAPHORE_KEY, PERMISSIONS, verbose);
    if (-1 == contentsize_semaphore_id)
	   exit(EXIT_FAILURE);
	content_semaphore_id = get_semaphore_id(MY_NAME, CONTENT_SEMAPHORE_KEY, PERMISSIONS, verbose);
    if (-1 == content_semaphore_id)
	   exit(EXIT_FAILURE);
	
	// get a handle to the shared memory
	contentsize_memory_id = get_shared_memory_id(MY_NAME, CONTENTSIZE_MEMORY_KEY, CONTENTSIZE_MEMORY_SIZE, PERMISSIONS, verbose);
    if (-1 == contentsize_memory_id)
	   exit(EXIT_FAILURE);

	// Attach the contentsize_semaphore_id memory.
	contentsize_address = (char *) shmat(contentsize_memory_id, NULL, 0);
	if ((void *)-1 == contentsize_address) {
		contentsize_address = NULL;
		sprintf(s, "Attaching the contentsize shared memory failed; errno is %d", errno);
		say(MY_NAME, s);
		exit(EXIT_FAILURE);
	}
	
	// get a handle to the shared memory
	content_memory_id = get_shared_memory_id(MY_NAME, CONTENT_MEMORY_KEY, CONTENT_MEMORY_SIZE, PERMISSIONS, verbose);
	if (-1 == content_memory_id)
		exit(EXIT_FAILURE);
	
	// Attach the contentsize_semaphore_id memory.
	content_address = (char *) shmat(content_memory_id, NULL, 0);
	if ((void *)-1 == content_address) {
		content_address = NULL;
		sprintf(s, "Attaching the contentsize shared memory failed; errno is %d", errno);
		say(MY_NAME, s);
		exit(EXIT_FAILURE);
	}
	
	// Start contentsize_address as 0
	strcpy((char *)contentsize_address, "0");
	
	// Release the contentsize_semaphore_id semaphore.
	rc = release_semaphore(MY_NAME, contentsize_semaphore_id, verbose);
	// Release the content_semaphore_id semaphore.
	rc = release_semaphore(MY_NAME, content_semaphore_id, verbose);

	while (keepRunning) {
		// Wait for a memory write on the other side
		while ( contentsize == 0 && keepRunning){
			// Acquire the contentsize_semaphore_id semaphore.
			rc = acquire_semaphore(MY_NAME, contentsize_semaphore_id, verbose);
			
			// Get memory content
			ptr = contentsize_address;
			contentsize = atoi(ptr);
			if (verbose){
				sprintf(s, "contentsize=%d", contentsize);
				say(MY_NAME, s);
			}
			
			// Reset contentsize_address
			strcpy((char *)contentsize_address, "0");

			// Release the contentsize_semaphore_id semaphore.
			rc = release_semaphore(MY_NAME, contentsize_semaphore_id, verbose);
			if (verbose)
				printf("\n");
			// sleep(2);
				
		}
		
		if ( contentsize > 0 && keepRunning){
			// start = clock();
			// Acquire semaphore to get the content
			rc = acquire_semaphore(MY_NAME, content_semaphore_id, verbose);
			
			// sprintf(s, "Content = %s", (char *) content_address);
			// say(MY_NAME, s);
			
			// Write to file
			sprintf(filename, "%s%02d", file_basename, i);
			strcat(filename, filetype);
			sprintf (outputfile, "%s/%s", outputdir, filename);
			FILE *f = fopen(outputfile, "wb");
			if (f == NULL){
				printf("Error opening file: %s!\n", outputfile);
				break;
			}
		
			fwrite((const void*) content_address, sizeof(char), contentsize, f);
			fclose(f);
			if (verbose){
				sprintf(s, "File written (size %d). Path: %s", contentsize, outputfile);
				say(MY_NAME, s);
			}
			// Increment file counter
			i++;
			
			// MD5 the reply and write back
			if (check_md5){
				md5ify((char *)content_address, md5ified_message, contentsize);
				strcpy((char *)content_address, md5ified_message);
				if (verbose){
					sprintf(s, "hash = %s", md5ified_message);
					say(MY_NAME, s);
				}
				
			}
			
			// Release the content_semaphore_id semaphore.
			rc = release_semaphore(MY_NAME, content_semaphore_id, verbose);
		
			// Reset contentsize
			contentsize = 0;
			if (verbose)
				printf("\n");
			// sleep(2);
			// end = clock();
			// cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
			// printf("CPU time=%fs\n", cpu_time_used);
			// exit(-1);
		}
	}	
		
	// END OF LIFE
	// Detach the memory
	if (verbose){
		sprintf(s, "Cleaning up shared memory");
		say(MY_NAME, s);
	}
	if (-1 == shmdt(contentsize_address)) {
		sprintf(s, "Detaching the memory contentsize_address failed; errno is %d", errno);
		say(MY_NAME, s);
	}
	contentsize_address = NULL;
	if (-1 == shmdt(content_address)) {
		sprintf(s, "Detaching the memory content_address failed; errno is %d", errno);
		say(MY_NAME, s);
	}
	content_address = NULL;
	
	// Remove shared memory
	if (-1 == shmctl(contentsize_memory_id, IPC_RMID, NULL)) {
		sprintf(s, "Removing the memory contentsize_memory_id failed; errno is %d", errno);
		say(MY_NAME, s);
	}
	if (-1 == shmctl(content_memory_id, IPC_RMID, NULL)) {
		sprintf(s, "Removing the memory content_memory_id failed; errno is %d", errno);
		say(MY_NAME, s);
	}
	
	// Clean up the semaphore
	if (verbose){
		sprintf(s, "Cleaning up semaphores\n");
		say(MY_NAME, s);
	}
	
	if (-1 == semctl(contentsize_semaphore_id, 0, IPC_RMID)) {
		sprintf(s, "Removing the semaphore contentsize_semaphore_id failed; errno is %d", errno);
		say(MY_NAME, s);
	}
	
	// Clean up the semaphore
	if (-1 == semctl(content_semaphore_id, 0, IPC_RMID)) {
		sprintf(s, "Removing the semaphore content_semaphore_id failed; errno is %d", errno);
		say(MY_NAME, s);
	}
	
    return 0; 
}
// EOF