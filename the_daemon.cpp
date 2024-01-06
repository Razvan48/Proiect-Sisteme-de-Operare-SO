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

#include "communication.hpp"
#include "job_system.hpp"	// TODO: de folosit

// TODO: DEBUG
std::ofstream fout("the_daemon.output");

//in codul de mai jos am comentat tot ce implica jobs (avem std::map jobs in job_system.hpp)


	char shm_input_name[] = "/shared_buffer_input";
	char shm_output_name[] = "/shared_buffer_output";
	char semaphore_1_name[] = "/shared_semaphore_1";
	char semaphore_2_name[] = "/shared_semaphore_2";

	pid_t pid;
	size_t shm_size_input = 0;
	size_t shm_size_output = 0;
	int shm_input_fd = 0;
	int shm_output_fd = 0;
	char* shared_buffer_input = NULL;
	char* shared_buffer_output = NULL;
	sem_t* sem_1 = NULL;
	sem_t* sem_2 = NULL;
	char* option = NULL;
	int* intInput = NULL;
	char* msg = NULL;

	bool isActive = true;


void init_shm_buffer();

void init_shm_semaphore();

int main(int argc, char* argv[])
{
	// TODO: DEBUG
	fout << "apelare the_daemon -> main()" << std::endl;

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

	// TODO: DEBUG
	fout << "the_daemon -> while() loop" << std::endl;

	while(isActive)
	{
		sem_wait(sem_1); // semaforul a fost creat cu valoarea zero, deci astept sa il incrementeze "da" (<=> astept
						// sa fie rulat ./da -option .... si sa imi trimita o comanda)	

		// TODO: DEBUG
		fout << "\n\nNew input" << std::endl;

		TaskType taskType = getTask(*option);
		std::string dir;
		int priority = 0;
		int id = 0;

		std::string outputMsg;

		switch (taskType)
		{
			case TaskType::ADD:
				priority = *intInput;
				dir = std::string(msg);

				outputMsg = addJob(dir, priority);
			break;
			
			case TaskType::SUSPEND:
				id = *intInput;

				outputMsg = pauseJob(id);
			break;
			
			case TaskType::RESUME:
				id = *intInput;

				outputMsg = unPauseJob(id);
			break;
			
			case TaskType::REMOVE:
				id = *intInput;

				outputMsg = deleteJob(id);
			break;
			
			case TaskType::INFO:
				id = *intInput;
			break;
			
			case TaskType::LIST:
				// TODO: job
			break;
			
			case TaskType::PRINT:
				id = *intInput;
			break;
			
			case TaskType::TERMINATE:
				isActive = false;
				outputMsg = "end the_daemon";
			break;

			case TaskType::DEFAULT:
				// TODO: delete
			break;
		}

		// TODO: DEBUG
		fout << "task: " << static_cast<char>(taskType) << std::endl;
		fout << "dir: " << dir << std::endl;
		fout << "priority: " << priority << std::endl;
		fout << "id: " << id << std::endl;

		// trimite mesaj catre da
		strcpy(shared_buffer_output, outputMsg.c_str());

		sem_post(sem_2); // daemonul spune ca a terminat de procesat stringul din shared memory
	}

	sem_close(sem_1);
	sem_close(sem_2);

	munmap(shared_buffer_input, shm_size_input);
	munmap(shared_buffer_output, shm_size_output);

	// the_daemon e responsabil sa stearga doar
	// 1. sem_1
	// 2. shared_buffer_input
	sem_unlink(semaphore_1_name);
	shm_unlink(shm_input_name);

	fout << std::endl << std::endl << "the_daemon a terminat executia" << std::endl;

	return 0;
}

void init_shm_buffer()
{
	// input buffer
	shm_input_fd = shm_open(shm_input_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // creez memoria partajata (functia se executa o singura data, la pornirea daemonului)
	if(shm_input_fd < 0)
	{
		perror(NULL);
		exit(errno);
	}

	shm_size_input = getpagesize();
	if(ftruncate(shm_input_fd, shm_size_input) == -1)
	{
		perror(NULL);
		shm_unlink(shm_input_name);
		exit(errno);
	}

	shared_buffer_input = (char*)mmap(0, shm_size_input, PROT_READ | PROT_WRITE, MAP_SHARED, shm_input_fd, 0);
	if(shared_buffer_input == MAP_FAILED)
	{
		perror(NULL);
		shm_unlink(shm_input_name);
		exit(errno);
	}

	option = shared_buffer_input; 
	intInput = (int*)(shared_buffer_input + sizeof(char));
	msg = shared_buffer_input + sizeof(char) + sizeof(int);

	*option = '\0';
	*intInput = 0;
	strcpy(msg, "");

	// output buffer
	shm_output_fd = shm_open(shm_output_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); 
	if(shm_output_fd < 0)
	{
		perror(NULL);
		munmap(shared_buffer_input, shm_size_input);
		shm_unlink(shm_input_name);
		exit(errno);
	}
	
	shm_size_output = getpagesize();
	if(ftruncate(shm_output_fd, shm_size_output) == -1)
	{
		perror(NULL);
		munmap(shared_buffer_input, shm_size_input);
		shm_unlink(shm_input_name);
		shm_unlink(shm_output_name);
		exit(errno);
	}

	shared_buffer_output = (char*) mmap(0, shm_size_output, PROT_READ | PROT_WRITE, MAP_SHARED, shm_output_fd, 0);
	if(shared_buffer_output == MAP_FAILED)
	{
		perror(NULL);
		munmap(shared_buffer_input, shm_size_input);
		shm_unlink(shm_input_name);
		shm_unlink(shm_output_name);
		exit(errno);
	}
}

void init_shm_semaphore()
{
	sem_1 = sem_open(semaphore_1_name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR, 0);
	if(sem_1 == SEM_FAILED)
	{
		perror(NULL);
		exit(errno);
	}

	sem_2 = sem_open(semaphore_2_name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR, 1);
	if(sem_2 == SEM_FAILED)
	{
		perror(NULL);
		sem_unlink(semaphore_1_name);
		exit(errno);
	}

	strcpy(shared_buffer_output, "");
}

