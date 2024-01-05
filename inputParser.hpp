#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <string.h>

#include "communication.hpp"

bool isOption(const char* user_option, const char* option1, const char* option2) 
{
	return strcmp(user_option, option1) == 0 || strcmp(user_option, option2) == 0;
}

void parseInput(int argc, char* argv[], TaskType& taskType, std::string& dir, int& priority, int& id)
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

        taskType = TaskType::ADD;
		dir = std::string(argv[2]);
		priority = 1;

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
			trimite la the_daemon.cpp
			1. -add     -> TaskType
			2. dir      -> string
			3. priority -> int
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
        if (id == 0)
        {
            printf("Error: ID must be a number > 0\n");
            return;
        }

        taskType = TaskType::SUSPEND;

		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista ID-ul printre task-urile noastra => ID 100 not found
			- daca totul merge => Suspended task with ID ’2’ for ’/home/user/my_repo’ and priority ’high’
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -S   -> TaskType
			2. id   -> int
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

		int id = atoi(argv[2]);
        if (id == 0)
        {
            printf("Error: ID must be a number > 0\n");
            return;
        }

        taskType = TaskType::RESUME;

		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista ID-ul printre task-urile noastra => ID 100 not found
			- daca totul merge => Resumed task with ID ’2’ for ’/home/user/my_repo’ and priority ’high’
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -R   -> TaskType
			2. id   -> int
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

		int id = atoi(argv[2]);
        if (id == 0)
        {
            printf("Error: ID must be a number > 0\n");
            return;
        }

        taskType = TaskType::REMOVE;

		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista ID-ul printre task-urile noastra => ID 100 not found
			- daca totul merge => Removed task with ID ’2’ for ’/home/user/my_repo’ and priority ’high’
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -r   -> TaskType
			2. id   -> id
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

		int id = atoi(argv[2]);
        if (id == 0)
        {
            printf("Error: ID must be a number > 0\n");
            return;
        }

        taskType = TaskType::INFO;

		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista ID-ul printre task-urile noastra => ID 100 not found
			- daca totul merge => Removed task with ID ’2’ for ’/home/user/my_repo’ and priority ’high’
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -i   -> TaskType
			2. id   -> int
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

        taskType = TaskType::LIST;

		/*
			TODO: trimite la the_daemon.cpp
			1. -l   -> TaskType
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

		int id = atoi(argv[2]);
        if (id == 0)
        {
            printf("Error: ID must be a number > 0\n");
            return;
        }

        taskType = TaskType::PRINT;

		/*
			TODO: de pus in the_daemon.cpp
			- verifica daca exista ID-ul printre task-urile noastra => ID 100 not found
			- daca totul merge => Path Usage Size Amount
		*/

		/*
			TODO: trimite la the_daemon.cpp
			1. -p   -> TaskType
			2. id   -> int
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

        taskType = TaskType::TERMINATE;

		/*
			TODO: trimite la the_daemon.cpp
			1. -k   -> TaskType
		*/
		return;
	}

	printf("da: invalid option \'%s\'\n", argv[1]);
   	printf("Try \'da --help\' for more information\n");
}

