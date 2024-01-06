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
	int* intInput = NULL;
	char* msg = NULL;

void init_shm_buffer(); // initializare memorie partajata

void init_shm_semaphore(); // initializare semafor partajat

int main(int argc, char* argv[])
{
	init_shm_buffer(); // obtin memoria partajata (si eventual pornesc daemonul)
	init_shm_semaphore(); // obtin semaforul partajat

	// input parser
	TaskType taskType = TaskType::DEFAULT;
	std::string dir;
	int priority = 0;
	int id = 0;
	
	parseInput(argc, argv, taskType, dir, priority, id);

	// Inputul nu este corect => nu trimite nimic la the_daemon
	if (taskType == TaskType::DEFAULT)
	{
		return 0;
	}
	
	sem_wait(sem_2); // astept sa termine daemonul de lucrat cu memoria partajata
	
	switch (taskType)
	{
        case TaskType::ADD:
			*option = static_cast<char>(taskType);
			*intInput = priority;
			strcpy(msg, dir.c_str());
		break;
        
		case TaskType::SUSPEND:
			*option = static_cast<char>(taskType);
			*intInput = id;
		break;
        
		case TaskType::RESUME:
			*option = static_cast<char>(taskType);
			*intInput = id;
		break;
        
		case TaskType::REMOVE:
			*option = static_cast<char>(taskType);
			*intInput = id;
		break;
        
		case TaskType::INFO:
			*option = static_cast<char>(taskType);
			*intInput = id;
		break;
        
		case TaskType::LIST:
			*option = static_cast<char>(taskType);
		break;
        
		case TaskType::PRINT:
			*option = static_cast<char>(taskType);
			*intInput = id;
		break;
        
		case TaskType::TERMINATE:
			*option = static_cast<char>(taskType);
		break;
	}

	// TODO: delete
	std::cout << "task: " << static_cast<char>(taskType) << '\n';
	std::cout << "intInput: " << *intInput << '\n';
	std::cout << "msg: " << dir << '\n';

	sem_post(sem_1);

	sem_wait(sem_2);

	// raspunsul de la the_daemon
	printf("the_daemon: %s\n", shared_buffer_output);

	// este datoria lui da sa incrementeze sem_2 pentru a putea efectua alta comanda da
	sem_post(sem_2);

	// am terminat 
	sem_close(sem_1);
	sem_close(sem_2);
	
	munmap(shared_buffer_input, shm_size_input);
	munmap(shared_buffer_output, shm_size_output);

	// daca trb sa inchidem the_daemon
	if (taskType == TaskType::TERMINATE)
	{
		// da e responsabil sa stearga doar
		// 1. sem_2 trebuie inchis 
		// 2. shared_buffer_output
		sem_unlink(semaphore_2_name);
		shm_unlink(shm_output_name);
	}

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
		
		// astept sa se termine initializarea daemonului
		// procesul parinte ("da") este radacina unei arborescente de forma: "da" -> "init_daemon" -> "daemon"
		// "init_daemon" initializeaza obiectele de memorie partajata si moare => "daemon" pierde contactul cu terminalul;
		// "da" stie ca initializarea s-a terminat si ca poate sa acceseze memoria partajata
		wait(NULL); 

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

	option = shared_buffer_input;
	intInput = (int*)(shared_buffer_input + sizeof(char));
	msg = shared_buffer_input + sizeof(char) + sizeof(int);

	// output buffer
	shm_output_fd = shm_open(shm_output_name, O_RDWR, S_IRUSR|S_IWUSR); 
	if(shm_output_fd < 0)
	{
		perror(NULL);
		munmap(shared_buffer_input, shm_size_input);
		shm_unlink(shm_input_name); // daca am eroare abia aici, inseamna ca obiectele pentru input s-au creat, deci le sterg
		exit(errno);
	}

	shm_size_output = getpagesize();
	shared_buffer_output = (char*)mmap(0, shm_size_output, PROT_READ | PROT_WRITE, MAP_SHARED, shm_output_fd, 0);
	if(shared_buffer_output == MAP_FAILED)
	{
		perror("map failed");
		munmap(shared_buffer_input, shm_size_input);
		shm_unlink(shm_input_name);
		shm_unlink(shm_output_name);
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

