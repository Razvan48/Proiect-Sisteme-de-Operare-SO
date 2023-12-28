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

#include "communication.hpp"	// TODO: de folosit


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
	sem_t* sem_1 = NULL;
	sem_t* sem_2 = NULL;
	char* option = NULL;
	char* msg = NULL;

void init_shm_buffer(); // initializare memorie partajata

void init_shm_semaphore(); // initializare semafor partajat

bool isOption(const char* user_option, const char* option1, const char* option2) 
{
	return strcmp(user_option, option1) == 0 || strcmp(user_option, option2) == 0;
}

void parseInput(int argc, char* argv[])
{
	if (argc == 1)
	{
		printf("da: missing operand\n");
		printf("Try 'da -h, --help' for more information\n");

		return;
	}

	if (isOption(argv[1], "-h", "--help"))
	{
		// comanda trebuie sa aiba fix 2 argumente
		if (argc > 2)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help");
			return;
		}

		printf("Usage: da [OPTION]... [DIR]...\n"
            "Analyze the space occupied by the directory at [DIR]\n"
            "  -a, --add <dir>          analyze a new directory path for disk usage\n"
            "  -p, --priority <1/2/3>   set priority for the new analysis (works only with -a argument)\n"
            "  -S, --suspend <id>       suspend task with <id>\n"
            "  -R, --resume <id>        resume task with <id>\n"
            "  -r, --remove <id>        remove the analysis with the given <id>\n"
            "  -i, --info <id>          print status about the analysis with <id> (pending, progress, d\n"			// TODO: completeaza
            "  -l, --list               list all analysis tasks, with their ID and the corresponding root p\n"		// TODO: completeaza
            "  -p, --print <id>         print analysis report for those tasks that are \"done\"\n"
            "  -t, --terminate          terminates daemon\n\n"														// TODO: de adaugat
        );

		return;
	}

	if (isOption(argv[1], "-a", "--add"))
	{
		if (argc > 5)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help");
			return;
		}

		if (argc <= 2)
		{
			printf("Error: You need to provide a directory for analysis\n");
			return;
		}

		std::string dir(argv[2]);
		int priority = 1;

		// check priority
		if (argc > 3)
		{
			if (isOption(argv[3], "-p", "--priority"))
			{
				if (argc == 5)
				{
					int inputPriority = atoi(argv[4]);

					if (inputPriority != 1 && inputPriority != 2 && inputPriority != 3)
					{
						printf("Error: Please choose a number from the set {1, 2, 3} for priority.\n");
						return;
					}

					priority = inputPriority;
				}
				else
				{
					printf("Error: Choose a number <1,2,3> for priority\n");
					return;
				}
			}
			else
			{
				printf("Error: You can only use the '-p' option with the '-a' option. Please check your command and try again. Use 'da -h' for help\n");
				return;
			}
		}
		
		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista folder-ul in sistem => Directory ’/home/user/my_repo/repo2’ not found
			- verifica daca nu este deja adaugat in daemon => Directory ’/home/user/my_repo/repo2’ is already included in analysis with ID ’2’
			- daca totul merge => Created analysis task with ID ’2’ for ’/home/user/my_repo’ and priority ’high’
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -add
			2. dir
			3. -p
			4. priority
		*/
		return;
	}

	if (isOption(argv[1], "-S", "--suspend"))
	{
		// comanda trebuie sa aiba fix 3 argumente
		if (argc > 3)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help");
			return;
		}

		if (argc != 3)
		{
			printf("Error: Choose an ID\n");
			return;
		}

		int id = atoi(argv[2]);

		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista ID-ul printre task-urile noastra => ID 100 not found
			- daca totul merge => Suspended task with ID ’2’ for ’/home/user/my_repo’ and priority ’high’
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -S
			2. id
		*/
		return;
	}

	if (isOption(argv[1], "-R", "--resume"))
	{
		// comanda trebuie sa aiba fix 3 argumente
		if (argc > 3)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help");
			return;
		}

		if (argc != 3)
		{
			printf("Error: Choose an ID\n");
			return;
		}

		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista ID-ul printre task-urile noastra => ID 100 not found
			- daca totul merge => Resumed task with ID ’2’ for ’/home/user/my_repo’ and priority ’high’
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -R
			2. id
		*/
		return;
	}

	if (isOption(argv[1], "-r", "--remove"))
	{
		// comanda trebuie sa aiba fix 3 argumente
		if (argc > 3)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help");
			return;
		}

		if (argc != 3)
		{
			printf("Error: Choose an ID\n");
			return;
		}

		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista ID-ul printre task-urile noastra => ID 100 not found
			- daca totul merge => Removed task with ID ’2’ for ’/home/user/my_repo’ and priority ’high’
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -r
			2. id
		*/
		return;
	}

	if (isOption(argv[1], "-i", "--info"))
	{
		// comanda trebuie sa aiba fix 3 argumente
		if (argc > 3)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help");
			return;
		}

		if (argc != 3)
		{
			printf("Error: Choose an ID\n");
			return;
		}

		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista ID-ul printre task-urile noastra => ID 100 not found
			- daca totul merge => Removed task with ID ’2’ for ’/home/user/my_repo’ and priority ’high’
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -i
			2. id
		*/
		return;
	}

	if (isOption(argv[1], "-l", "--list"))
	{
		// comanda trebuie sa aiba fix 2 argumente
		if (argc > 2)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help");
			return;
		}

		/*
			TODO: trimite la the_daemon.cpp
			1. -l
		*/
		return;
	}

	if (isOption(argv[1], "-p", "--print"))
	{
		// comanda trebuie sa aiba fix 3 argumente
		if (argc > 3)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help");
			return;
		}

		if (argc != 3)
		{
			printf("Error: Choose an ID\n");
			return;
		}

		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista ID-ul printre task-urile noastra => ID 100 not found
			- daca totul merge => Path Usage Size Amount
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -p
			2. id
		*/
		return;
	}

	if (isOption(argv[1], "-t", "--terminate"))
	{
		// comanda trebuie sa aiba fix 2 argumente
		if (argc > 2)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help");
			return;
		}

		/*
			TODO: trimite la the_daemon.cpp
			1. -t
		*/
		return;
	}

	printf("da: invalid option \'%s\'\n", argv[1]);
   	printf("Try \'da --help\' for more information\n");
}

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

	// TODO
	// parseInput(argc, argv);

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

	if(*option == ADD_MSG)
		strcpy(msg, argv[2]); // scriu mesajul (calea) in memoria partajata

	if(*option == PRINT)
		printf("da: %s\n", msg); // afisez din memoria partajata	

	sem_post(sem_1); // daemonul era blocat in sem_wait(sem) (semaforul e zero) pana termin de scris in memoria partajata;
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
	
	sem_1 = (sem_t*) shm_semaphore_ptr;
	sem_2 = (sem_t*) shm_semaphore_ptr + sizeof(sem_t);
}
