#include <iostream>
#include <fstream>
#include <string>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <sys/wait.h>

#include "communication.hpp"
#include "inputParser.hpp"
#include "job_system.hpp"	// TODO: de folosit

#define KILL 'k'
#define ADD_MSG 'a'
#define PRINT 'p'


	char shm_input_name[] = "/shared_buffer_input";
	char shm_output_name[] = "/shared_buffer_output";
	char semaphore_1_name[] = "/shared_semaphore_1";
	char semaphore_2_name[] = "/shared_semaphore_2";

	size_t shm_size_input = 0;
	size_t shm_size_output = 0;
	int shm_input_fd = 0;
	int shm_output_fd = 0;
	char* shared_buffer_input = NULL;
	char* shared_buffer_output = NULL;
	sem_t* sem_1 = NULL;
	sem_t* sem_2 = NULL;
	char* option = NULL;
	char* priority = NULL;
	char* msg = NULL;

void init_shm_buffer(); // initializare memorie partajata

void init_shm_semaphore(); // initializare semafor partajat

int main(int argc, char* argv[])
{
	if (argc < 2)
	{		
		printf("continutul help-ului.......\n");
		return -1;
	}
	if (strcmp(argv[1], "-a") == 0 && argc < 3)
	{		
		printf("continutul help-ului.......\n");
		return -1;
	}

	init_shm_buffer(); // obtin memoria partajata (si eventual pornesc daemonul)
	init_shm_semaphore(); // obtin semaforul partajat

	/*	TODO: input parser
	TaskType taskType = TaskType::DEFAULT;
	std::string dir;
	int p;
	int id;
	
	parseInput(argc, argv, taskType, dir, p, id);

	switch (taskType)
	{
        case TaskType::ADD:
			*option = static_cast<char>(taskType);
			// TODO: *directory = dir
			// TODO: *priority = p
		break;
        
		case TaskType::SUSPEND:
			*option = static_cast<char>(taskType);
			// TODO: *taskId = id
		break;
        
		case TaskType::RESUME:
			*option = static_cast<char>(taskType);
			// TODO: *taskId = id
		break;
        
		case TaskType::REMOVE:
			*option = static_cast<char>(taskType);
			// TODO: *taskId = id
		break;
        
		case TaskType::INFO:
			*option = static_cast<char>(taskType);
			// TODO: *taskId = id
		break;
        
		case TaskType::LIST:
			*option = static_cast<char>(taskType);
		break;
        
		case TaskType::PRINT:
			*option = static_cast<char>(taskType);
			// TODO: *taskId = id
		break;
        
		case TaskType::TERMINATE:
			*option = static_cast<char>(taskType);
		break;

        case TaskType::DEFAULT:
			// Inputul nu este corect => nu trimite nimic la the_daemon
		break;
	}
	*/

	char opt = '\0';

	if(strcmp(argv[1], "-k") == 0) 
		opt = KILL;
	else
		if(strcmp(argv[1], "-a") == 0)
			opt = ADD_MSG;
		else
			if(strcmp(argv[1], "-p") == 0)
				opt = PRINT;
			else
			{
				printf("continutul help-ului.......\n");
				return -1;
			} // setez optiunea (primul octet din memoria partajata)
	
	sem_wait(sem_2); // astept sa termine daemonul de lucrat cu memoria partajata
	*option = opt;

	if(*option == PRINT)
	{
		sem_post(sem_1);
		sem_wait(sem_2);
		printf("da: %s\n", shared_buffer_output); // afisez din memoria partajata	
	}
	else
	{
		if(*option == ADD_MSG)
		strcpy(msg, argv[2]); // scriu mesajul (calea) in memoria partajata

	sem_post(sem_1); // daemonul era blocat in sem_wait(sem) (semaforul e zero) pana termin de scris in memoria partajata;
					// il eliberez si imi termin executia
	}

	sem_close(sem_1);
	sem_close(sem_2);
	munmap(shared_buffer_input, shm_size_input);
	munmap(shared_buffer_output, shm_size_output);

	return 0;
}


void init_shm_buffer()
{
	// input buffer
	shm_input_fd = shm_open(shm_input_name, O_RDWR, S_IRUSR|S_IWUSR); // obtin memoria partajata creata de daemon
	if(errno == ENOENT) // daca primesc "no such file or directory" inseamna ca daemon-ul nu e pornit si il pornesc
	{
		pid_t pid = fork();
		if(pid < 0)
			exit(errno);

		if(pid == 0) // proces copil
		{
			char* argv_c[30] = {"the_daemon", NULL};
			execve("./the_daemon", argv_c, NULL); // rulez executabilul "the daemon"
			perror("nu a pornit daemonul");
		}
		wait(NULL); // astapt sa se termine initializarea daemonului
	// procesul parinte ("da") este radacina unei arborescente de forma: "da" -> "init_daemon" -> "daemon"
	// "init_daemon" initializeaza obiectele de memorie partajata si moare => "daemon" pierde contactul cu terminalul;
	// "da" stie ca initializarea s-a terminat si ca poate sa acceseze memoria partajata

		shm_input_fd = shm_open(shm_input_name, O_RDWR, S_IRUSR|S_IWUSR);
	}
	if(shm_input_fd < 0)
	{
		perror(NULL);
		exit(errno);
	}

	shm_size_input = getpagesize();
	shared_buffer_input = (char*)mmap(0, shm_size_input, PROT_READ | PROT_WRITE, MAP_SHARED, shm_input_fd, 0);
	if(shared_buffer_input == MAP_FAILED)
	{
		perror("map failed");
		shm_unlink(shm_input_name);
		exit(errno);
	} // s-a mapat memoria partajata (un sir de caractere de 4096)
	
	option = shared_buffer_input; // optiune
	priority = shared_buffer_input + sizeof(char); // prioritate, daca este cazul
	msg = shared_buffer_input + 2*sizeof(char); // de aici incepe mesajul efectiv (calea catre directorul de analizat)


	// output buffer
	shm_output_fd = shm_open(shm_output_name, O_RDWR, S_IRUSR|S_IWUSR); 
	if(shm_output_fd < 0)
	{
		perror(NULL);
		shm_unlink(shm_input_name); // daca am eroare abia aici, inseamna ca obiectele pentru input s-au creat, deci le sterg
		munmap(shared_buffer_input, shm_size_input);
		exit(errno);
	}

	shm_size_output = getpagesize();
	shared_buffer_output = (char*)mmap(0, shm_size_output, PROT_READ | PROT_WRITE, MAP_SHARED, shm_output_fd, 0);
	if(shared_buffer_output == MAP_FAILED)
	{
		perror("map failed");
		shm_unlink(shm_input_name);
		shm_unlink(shm_output_name);
		munmap(shared_buffer_input, shm_size_input);
		exit(errno);
	}
}

void init_shm_semaphore()
{
	sem_1 = sem_open(semaphore_1_name, O_RDWR);
	if(sem_1 == SEM_FAILED)
	{
		perror(NULL);
		exit(errno);
	}

	sem_2 = sem_open(semaphore_2_name, O_RDWR);
	if(sem_2 == SEM_FAILED)
	{
		perror(NULL);
		exit(errno);
	}
}
