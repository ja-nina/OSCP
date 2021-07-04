#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdbool.h>
#include <time.h>
#define SHMSIZE 100
#define SNAMEP "/mutp"
#define SNAMEC "/mutc"
#define FULL "/full"
#define FREE "/free"
#define STORAGE_ID "/mybuffer"


int main(int argc, char* argv[]){
//creating semaphor

    int N = atoi(argv[1]);
    srand( time(0));
    int* buffer;
    int item;
    int fd;
    int pid = getpid();
    int value_of_sem;
    int waiting;
    int low = atoi(argv[2]);
    int running_time = atoi(argv[3]);
    int number_of_producers = atoi(argv[4]);
    sem_t *frees = sem_open(FREE, O_RDWR|O_CREAT, 0600, N);
    sem_t *full = sem_open(FULL, O_RDWR|O_CREAT, 0600, 0);
    sem_t *mutp = sem_open(SNAMEP, O_RDWR|O_CREAT, 0600, 1); 
    sem_t *mutc = sem_open(SNAMEC, O_RDWR|O_CREAT, 0600, 1); 
    sem_t *mem_set = sem_open("/memset", O_RDWR|O_CREAT, 0600, 1);
    if (frees==SEM_FAILED || full==SEM_FAILED || mutc==SEM_FAILED || mutp==SEM_FAILED || mem_set==SEM_FAILED ){ printf("did not open!\n");perror("sem_open");}
	

//SHARED MEMORY
   fd = shm_open(STORAGE_ID, O_EXCL|O_CREAT, 0600);
	if (fd == -1)
	{
		// file exists
		sem_wait(mem_set);
		printf("C{%d} setting up memory\n", pid);
		fd = shm_open(STORAGE_ID,O_CREAT | O_RDWR, 0600);
		buffer = mmap(NULL, N + 2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		sem_post(mem_set);
	}
	else{
		// shm does not exist
		sem_wait(mem_set);
		printf("C{%d} setting up memory (fisrt)\n", pid);
		fd = shm_open(STORAGE_ID, O_CREAT | O_RDWR, 0600);
		ftruncate(fd, N + 2);
  		buffer = mmap(NULL, N + 2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
   		for(int i =0; i<N;i++){
   			buffer[i] = -1;
   		}
   		buffer[N] = 0;
   		buffer[N+1] = 0;
   		sem_post(mem_set);
		}
  time_t start = time(NULL);
  while((time(NULL) - start) < running_time){
  //checking
  //trying to enter critical section
  //each producer waits some time between 1000 microseconds and 1 second
  sem_wait(full);
  printf("C{%d} enters queue to cCS!\n", pid);
  
  sem_wait(mutc);
  printf("C{%d} is inside the cCS!\n", pid);
  msync(buffer, N + 2, MS_SYNC);
  
  //the introduced waiting time makes the competition for CS even fiercer
  waiting = rand() % (1000000 - low + 1) + low;
  usleep(waiting);
  item = rand()%(2*N);
  
  item = buffer[buffer[N + 1]];
  printf("C{%d} consumed \t %d \t at \t %d \n", pid, item, buffer[N + 1]);
  
  buffer[N + 1] = (buffer[N + 1] + 1)%N;
  msync(buffer, N + 2, MS_SYNC);
  //existing CS
  
  sem_post(mutc);
  printf("C{%d} exits cCS!\n", pid);
  sem_post(frees);
  
  
  }
  //this part below is only for the programm to run smoothly and finish afetr a finite amount of time specified.
  //therefore it is not ment to be treated as a solution to the producer-consumer problem!
  sem_getvalue(frees, &value_of_sem);
  while( value_of_sem < number_of_producers){
  	sem_wait(full);
  	sem_wait(mutc);
  	msync(buffer, N + 2, MS_SYNC);
  	item = buffer[buffer[N + 1]];
	buffer[N + 1] = (buffer[N + 1] + 1)%N;
  	msync(buffer, N + 2, MS_SYNC);
	sem_post(mutc);
	sem_post(frees);
	sleep(1);
	sem_getvalue(frees, &value_of_sem);
  	}
  sem_close(frees);
  sem_close(full);
  sem_close(mutp);
  sem_close(mutc);
  sem_close(mem_set);
  sem_unlink("/memset");
  sem_unlink(FREE);
  sem_unlink(FULL);
  sem_unlink(SNAMEP);
  sem_unlink(SNAMEC);
  munmap(buffer, N + 2);
  shm_unlink(STORAGE_ID);
  printf("C{%d} stops executing for good\n", pid);
  return 0;
  
  
}
