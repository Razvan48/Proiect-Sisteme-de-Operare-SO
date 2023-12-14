#include <iostream>
#include <fstream>
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


#define KILL 'k'
#define ADD_MSG 'a'
#define PRINT 'p'


	char shm_name[] = "/shared_buffer";
	char shm_semaphore_name[] = "/shared_semaphore";
	size_t shm_size = 0;
	size_t shm_semaphore_size = 0;
	int shm_fd = 0;
	int shm_semaphore_fd = 0;
	char* shm_ptr = NULL;
	void* shm_semaphore_ptr = NULL;
	sem_t* sem = NULL;
	char* option = NULL;
	char* msg = NULL;

void init_shm_buffer(); // initializare memorie partajata

void init_shm_semaphore(); // initializare semafor partajat


int main(int argc, char* argv[])
{
	if(argc < 2)
	{		
		printf("continutul help-ului.......\n");
		return -1;
	}
	if(strcmp(argv[1], "-a") == 0 && argc < 3)
	{		
		printf("continutul help-ului.......\n");
		return -1;
	}

	init_shm_buffer(); // obtin memoria partajata (si eventual pornesc daemonul)
	init_shm_semaphore(); // obtin semaforul partajat
	
	if(strcmp(argv[1], "-k") == 0) 
		*option = KILL;
	else
		if(strcmp(argv[1], "-a") == 0)
			*option = ADD_MSG;
		else
			if(strcmp(argv[1], "-p") == 0)
				*option = PRINT;
			else
			{
				printf("continutul help-ului.......\n");
				return -1;
			} // setez optiunea (primul octet din memoria partajata)

	if(*option == ADD_MSG)
		strcpy(msg, argv[2]); // scriu mesajul (calea) in memoria partajata

	if(*option == PRINT)
		printf("da: %s\n", msg); // afisez din memoria partajata	

	sem_post(sem); // daemonul era blocat in sem_wait(sem) (semaforul e zero) pana termin de scris in memoria partajata;
					// il eliberez si imi termin executia


	return 0;
}


void init_shm_buffer()
{
	shm_fd = shm_open(shm_name, O_RDWR, S_IRUSR | S_IWUSR); // obtin memoria partajata creata de daemon
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

		shm_fd = shm_open(shm_name, O_RDWR, S_IRUSR | S_IWUSR);
	}
	if(shm_fd < 0)
	{
		perror(NULL);
		exit(errno);
	}

	shm_size = getpagesize();
	shm_ptr = (char*)mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(shm_ptr == MAP_FAILED)
	{
		perror("map failed");
		shm_unlink(shm_name);
		exit(errno);
	} // s-a mapat memoria partajata (un sir de caractere de 4096)
	
	option = shm_ptr; // primul caracter il tin special pentru transmiterea de optiuni
	msg = shm_ptr + sizeof(char); // de aici incepe mesajul efectiv (calea catre directorul de analizat)
}

void init_shm_semaphore()
{
	shm_semaphore_fd = shm_open(shm_semaphore_name, O_RDWR, S_IRUSR | S_IWUSR);
	if(shm_semaphore_fd < 0)
	{
		perror("no shared memory");
		exit(errno);
	}

	shm_semaphore_size = getpagesize();
	shm_semaphore_ptr = mmap(0, shm_semaphore_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_semaphore_fd, 0);
	if(shm_semaphore_ptr == MAP_FAILED)
	{
		perror("map failed");
		shm_unlink(shm_semaphore_name);
		exit(errno);
	} // am obtinut semaforul partajat cu daemonul
	
	sem = (sem_t*) shm_semaphore_ptr;
}


