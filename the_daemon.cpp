#include <iostream>
#include <fstream>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <semaphore.h>
#include <chrono>
#include <time.h>

#include "communication.hpp"	// TODO: de folosit
#include "job_system.hpp"	// TODO: de folosit

//in codul de mai jos am comentat tot ce implica jobs (avem std::map jobs in job_system.hpp)

#define KILL 'k'
#define ADD_MSG 'a'
#define PRINT 'p'

	pid_t pid;
	size_t shm_size = 0;
	size_t shm_semaphore_size = 0;
	char shm_name[] = "/shared_buffer";
	char shm_semaphore_name[] ="/shared_semaphore";
	int shm_fd = 0;
	int shm_semaphore_fd = 0;
	char* shm_ptr = NULL;
	void* shm_semaphore_ptr = NULL;
	sem_t* sem_1 = NULL;
	sem_t* sem_2 = NULL;
	bool isActive = true;
	char* option = NULL;
	char* msg = NULL;


void init_shm_buffer();

void init_shm_semaphore();

// void jobs();

int main(int argc, char* argv[])
{
	init_shm_buffer(); // creez memoria partajata o singura data la pornirea daemonului
	init_shm_semaphore(); // creez semaforul partajat o singura data la pornirea daemonului

	pid = fork(); // creez un fiu, care va fi daemonul efectiv
	if(pid < 0)
		return errno; 
	if(pid > 0) 
	{
		wait(NULL);
		return 0;
	} // parintele se termina; fiul (daemonul) a pierdut contactul cu terminalul; 
		// procesul "da" care a pornit parintele se opreste din wait(NULL), stiind ca daemonul functioneaza
	
	if(setsid() < 0) // creez o noua sesiune pentru a ma detasa de orice terminal
	{
		perror(NULL);
		return errno;
	}	

	pid = fork(); // creez o noua sesiune pentru a ma detasa de orice terminal
	if(pid < 0)
		return errno; 
	if(pid > 0) 
	{
		return 0;
	}

	while(isActive)
	{
		sem_wait(sem_1); // semaforul a fost creat cu valoarea zero, deci astept sa il incrementeze "da" (<=> astept
						// sa fie rulat ./da -option .... si sa imi trimita o comanda)

		if(*option == KILL)
		{
			isActive = false;
			continue; // 
		}

		// jobs();    // "munca" efectiva a daemonului

		sem_post(sem_2); // daemonul spune ca a terminat de procesat stringul din shared memory

	}

	shm_unlink(shm_name);
	shm_unlink(shm_semaphore_name);
	
	return 0;
}



void init_shm_buffer()
{
	shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // creez memoria partajata (functia se executa o singura data, la pornirea daemonului)
	if(shm_fd < 0)
	{
		perror(NULL);
		exit(errno);
	}
	shm_size = getpagesize();
	if(ftruncate(shm_fd, shm_size) == -1)
	{
		perror(NULL);
		shm_unlink(shm_name);
		exit(errno);
	}

	shm_ptr = (char*) mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(shm_ptr == MAP_FAILED)
	{
		perror(NULL);
		shm_unlink(shm_name);
		exit(errno);
	}

	option = shm_ptr; // optiunea - primul octet
	msg = shm_ptr + sizeof(char); // mesajul (calea) restul
	*option = '\0';
	strcpy(msg, "");
}

void init_shm_semaphore()
{
	shm_semaphore_fd = shm_open(shm_semaphore_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // creez semaforul partajat (functia se executa o singura data, la pornirea daemonului)
	if(shm_semaphore_fd < 0)
	{
		perror(NULL);
		exit(errno);
	}
	shm_semaphore_size = getpagesize();
	if(ftruncate(shm_semaphore_fd, shm_size) == -1)
	{
		perror(NULL);
		shm_unlink(shm_semaphore_name);
		exit(errno);
	}

	shm_semaphore_ptr = mmap(0, shm_semaphore_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_semaphore_fd, 0);
	if(shm_semaphore_ptr == MAP_FAILED)
	{
		perror(NULL);
		shm_unlink(shm_semaphore_name);
		exit(errno);
	}

	sem_1 = (sem_t*) shm_semaphore_ptr;
	sem_2 = (sem_t*) shm_semaphore_ptr + sizeof(sem_t);
	sem_init(sem_1, 1, 0); // initializ semaforul cu zero, pentru ca daemonul sa se blocheze in asteptare pana la prima comanda de la "da"
	sem_init(sem_2, 1, 1); // initializ semaforul cu unu, pentru ca "da" sa poata scrie prima comanda
}

/* am comentat asta pentru ca avem std::map de jobs
void jobs()
{
	if(*option == ADD_MSG)
		{
			auto time_stamp = std::chrono::system_clock::now();
			std::time_t current_time_epoch = std::chrono::system_clock::to_time_t(time_stamp);
			char the_time[200] = "";
			sprintf(the_time, " - timestamp  %s", std::ctime(&current_time_epoch));
			strcat(msg, the_time); // doar adaug data si ora la mesajul primit de la "da"
		}
}
*/
