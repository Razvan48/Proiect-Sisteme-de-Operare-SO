#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdio.h>

///interfata

int main(int argc, char* argv[])
{
	int inputInterfata = open("inputInterfata.txt", O_RDONLY | O_CREAT);
	int inputDaemon = open("inputDaemon.txt", O_WRONLY | O_CREAT);
	
	//vedem in ce stare e daemonul
	
	printf("test");

	flock mutexInterfata; //lock
	mutexInterfata.l_type = F_RDLCK;
	mutexInterfata.l_start = 0;
	mutexInterfata.l_whence = SEEK_SET;
	mutexInterfata.l_len = 0;
	fcntl(inputInterfata, F_SETLKW, &mutexInterfata); //am blocat fisierul inputInterfata
	
	
	char buffer[256];
	read(inputInterfata, buffer, sizeof(buffer));
	
	
	mutexInterfata.l_type = F_UNLCK;
	fcntl(inputInterfata, F_SETLK, &mutexInterfata); //unlock
	
	
	if (strcmp(buffer, "true") != 0)
	{
		pid_t pidDaemon = fork();
		
		if (pidDaemon == 0)
		{
			execve("pipe2", nullptr, nullptr);
			
			printf("nu e ok");
			
			return 0; //nu se va ajunge aici
		}
	}
	
	//scriem comanda pentru daemon
	
	flock mutexDaemon;
	mutexDaemon.l_type = F_WRLCK;
	mutexDaemon.l_start = 0;
	mutexDaemon.l_whence = SEEK_SET;
	mutexDaemon.l_len = 0;
	fcntl(inputDaemon, F_SETLKW, &mutexDaemon); //am blocat
	
	write(inputDaemon, argv[1], sizeof(argv[1]));
	
	mutexDaemon.l_type = F_UNLCK;
	fcntl(inputDaemon, F_SETLK, &mutexDaemon);
	
	close(inputInterfata);
	close(inputDaemon);
	
	printf("test2");
	
	return 0;
}
