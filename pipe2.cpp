#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdio.h>

///asta merge in continuu in fundal

int main(int argc, char* argv[])
{
	int inputInterfata = open("inputInterfata.txt", O_WRONLY | O_CREAT);
	int inputDaemon = open("inputDaemon.txt", O_RDWR | O_CREAT);

	flock mutexInterfata; //lock
	mutexInterfata.l_type = F_WRLCK;
	mutexInterfata.l_start = 0;
	mutexInterfata.l_whence = SEEK_SET;
	mutexInterfata.l_len = 0;
	fcntl(inputInterfata, F_SETLKW, &mutexInterfata); //am blocat fisierul inputInterfata
	
	ftruncate(inputInterfata, 0);
	
	write(inputInterfata, "true", 4);
	
	
	mutexInterfata.l_type = F_UNLCK;
	fcntl(inputInterfata, F_SETLK, &mutexInterfata); //unlock
	
	
	flock mutexDaemon;
	mutexDaemon.l_type = F_RDLCK;
	mutexDaemon.l_start = 0;
	mutexDaemon.l_whence = SEEK_SET;
	mutexDaemon.l_len = 0;
	
	while (true)
	{
		mutexDaemon.l_type = F_RDLCK;
		fcntl(inputDaemon, F_SETLKW, &mutexDaemon); //am blocat
		
		//read
		char buffer[256];
		read(inputDaemon, buffer, sizeof(buffer));
		
		ftruncate(inputDaemon, 0);
		
		mutexDaemon.l_type = F_UNLCK;
		fcntl(inputDaemon, F_SETLK, &mutexDaemon);
		
		printf(buffer);
		
		if (strcmp(buffer, "stop") == 0)
			break;
		
		
	}
	
	printf("stop");
	
	
	
	close(inputInterfata);
	close(inputDaemon);
	
	return 0;
}
