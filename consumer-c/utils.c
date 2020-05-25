#include <time.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <semaphore.h>
#include <string.h>

#include <sys/ipc.h>		/* for system's IPC_xxx definitions */
#include <sys/shm.h>		/* for shmget, shmat, shmdt, shmctl */
#include <sys/sem.h>		/* for semget, semctl, semop */

#include "utils.h"
#include "md5.h"


void md5ify(char *inString, char *outString, int size) {
	md5_state_t state;
	md5_byte_t digest[16];
    int i;
    
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)inString, size);
	md5_finish(&state, digest);

    for (i = 0; i < 16; i++)
        sprintf(&outString[i * 2], "%02x", digest[i]);
}

void say(const char *pName, char *pMessage) {
    time_t the_time;
    struct tm *the_localtime;
    char timestamp[256];
    
    the_time = time(NULL);
    
    the_localtime = localtime(&the_time);
    
    strftime(timestamp, 255, "%H:%M:%S", the_localtime);
    
    printf("%s @ %s: %s\n", pName, timestamp, pMessage);
    
}

int get_shared_memory_id (const char *pName, int key, int size, int permissions, int verbose){
	char s[1024];
	int id = 0;
	id = shmget(key, size, IPC_CREAT | IPC_EXCL | permissions);
	if (id == -1) {
		id = 0;
		sprintf(s, "Couldn't get a handle to the shared memory key %d and size %d; errno is %d", key, size, errno);
		say(pName, s);
		return -1;
	}
	if (verbose){
		sprintf(s, "Key %d shared memory's id is %d", key, id);
		say(pName, s);
	}
	return id;
}

int get_semaphore_id (const char *pName, int key, int permissions, int verbose){
	char s[1024];
	int id = 0;
    id = semget(key, 1, IPC_CREAT | IPC_EXCL | permissions);
    if (-1 == id) {
        id = 0;
		sprintf(s, "Couldn't get a handle to the semaphore key %d; errno is %d", key, errno);
        say(pName, s);
		return -1;
    }
	if (verbose){
		sprintf(s, "Key %d semaphore's id is %d", key, id);
		say(pName, s);
	}
	return id;
}


int release_semaphore(const char *pName, int sem_id, int verbose) {
    int rc = 0;
    struct sembuf op[1];
    char s[1024];
    if (verbose)
		say(pName, "Releasing the semaphore.");
    
    op[0].sem_num = 0;
    op[0].sem_op = 1;
    op[0].sem_flg = 0;

    if (-1 == semop(sem_id, op, (size_t)1)) {
        sprintf(s, "Releasing the semaphore id %d failed; errno is %d\n", sem_id, errno);
        say(pName, s);
    }
    
    return rc;
}


int acquire_semaphore(const char *pName, int sem_id, int verbose) {
    int rc = 0;
    struct sembuf op[1];
    char s[1024];
	if (verbose)
		say(pName, "Waiting to acquire the semaphore.");

    op[0].sem_num = 0;
    op[0].sem_op = -1;
    op[0].sem_flg = 0;
    if (-1 == semop(sem_id, op, (size_t)1)) {
        sprintf(s, "Acquiring the semaphore id %d failed; errno is %d\n", sem_id, errno);
        say(pName, s);
    }

    return rc;
}