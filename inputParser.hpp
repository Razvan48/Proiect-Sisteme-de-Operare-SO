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

		taskType = TaskType::DEFAULT;
		return;
	}

	if (isOption(argv[1], "-h", "--help"))
	{
		taskType = TaskType::DEFAULT;

		// comanda trebuie sa aiba fix 2 argumente
		if (argc > 2)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help\n");
			return;
		}

		printf("Usage: da [OPTION]... [DIR]...\n"
            "Analyze the space occupied by the directory at [DIR]\n"
            "  -a, --add <dir>          analyze a new directory path for disk usage\n"
            "  -p, --priority <1/2/3>   set priority for the new analysis (works only with -a argument)\n"
            "  -S, --suspend <id>       suspend task with <id>\n"
            "  -R, --resume <id>        resume task with <id>\n"
            "  -r, --remove <id>        remove the analysis with the given <id>\n"
            "  -i, --info <id>          print status about the analysis with <id> (pending, progress, done)\n"
            "  -l, --list               list all analysis tasks, with their ID and the corresponding root path\n"
            "  -p, --print <id>         print the analysis report for the task with <id> if it is \"done\"\n"
            "  -t, --terminate          terminates the daemon\n\n"
        );

		return;
	}

	if (isOption(argv[1], "-a", "--add"))
	{
		if (argc > 5)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help");
			taskType = TaskType::DEFAULT;
			return;
		}

		if (argc <= 2)
		{
			printf("Error: You need to provide a directory for analysis\n");
			taskType = TaskType::DEFAULT;
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
						printf("Error: Choose a number <1,2,3> for priority\n");
						taskType = TaskType::DEFAULT;
						return;
					}

					priority = inputPriority;
				}
				else
				{
					printf("Error: Choose a number <1,2,3> for priority\n");
					taskType = TaskType::DEFAULT;
					return;
				}
			}
			else
			{
				printf("Error: You can only use the '-p' option with the '-a' option. Please check your command and try again. Use 'da -h' for help\n");
				taskType = TaskType::DEFAULT;
				return;
			}
		}

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
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help\n");
			taskType = TaskType::DEFAULT;
			return;
		}

		if (argc != 3)
		{
			printf("Error: Choose an ID\n");
			taskType = TaskType::DEFAULT;
			return;
		}

		int inputID = atoi(argv[2]);
        if (inputID == 0)
        {
            printf("Error: ID must be a number > 0\n");
			taskType = TaskType::DEFAULT;
            return;
        }

        taskType = TaskType::SUSPEND;
		id = inputID;

		/*
			trimite la the_daemon.cpp
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
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help\n");
			taskType = TaskType::DEFAULT;
			return;
		}

		if (argc != 3)
		{
			printf("Error: Choose an ID\n");
			taskType = TaskType::DEFAULT;
			return;
		}

		int inputID = atoi(argv[2]);
        if (inputID == 0)
        {
            printf("Error: ID must be a number > 0\n");
			taskType = TaskType::DEFAULT;
            return;
        }

        taskType = TaskType::RESUME;
		id = inputID;

		/*
			trimite la the_daemon.cpp
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
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help\n");
			taskType = TaskType::DEFAULT;
			return;
		}

		if (argc != 3)
		{
			printf("Error: Choose an ID\n");
			taskType = TaskType::DEFAULT;
			return;
		}

		int inputID = atoi(argv[2]);
        if (inputID == 0)
        {
            printf("Error: ID must be a number > 0\n");
			taskType = TaskType::DEFAULT;
            return;
        }

        taskType = TaskType::REMOVE;
		id = inputID;

		/*
			trimite la the_daemon.cpp
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
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help\n");
			taskType = TaskType::DEFAULT;
			return;
		}

		if (argc != 3)
		{
			printf("Error: Choose an ID\n");
			taskType = TaskType::DEFAULT;
			return;
		}

		int inputID = atoi(argv[2]);
        if (inputID == 0)
        {
            printf("Error: ID must be a number > 0\n");
			taskType = TaskType::DEFAULT;
            return;
        }

        taskType = TaskType::INFO;
		id = inputID;

		/*
			trimite la the_daemon.cpp
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
			taskType = TaskType::DEFAULT;
			return;
		}

        taskType = TaskType::LIST;

		/*
			trimite la the_daemon.cpp
			1. -l   -> TaskType
		*/
		return;
	}

	if (isOption(argv[1], "-p", "--print"))
	{
		// comanda trebuie sa aiba fix 3 argumente
		if (argc > 3)
		{
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help\n");
			taskType = TaskType::DEFAULT;
			return;
		}

		if (argc != 3)
		{
			printf("Error: Choose an ID\n");
			taskType = TaskType::DEFAULT;
			return;
		}

		int inputID = atoi(argv[2]);
        if (inputID == 0)
        {
            printf("Error: ID must be a number > 0\n");
			taskType = TaskType::DEFAULT;
            return;
        }

        taskType = TaskType::PRINT;
		id = inputID;

		/*
			trimite la the_daemon.cpp
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
			printf("Error: Too many arguments provided. Please check your command and try again. Use 'da -h, --help' for help\n");
			taskType = TaskType::DEFAULT;
			return;
		}

        taskType = TaskType::TERMINATE;

		/*
			trimite la the_daemon.cpp
			1. -k   -> TaskType
		*/
		return;
	}

	printf("da: invalid option \'%s\'\n", argv[1]);
   	printf("Try \'da --help\' for more information\n");
}

