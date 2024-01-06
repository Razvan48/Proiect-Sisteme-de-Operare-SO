#pragma once

#include <iostream>
#include <fstream>

#include <vector>
#include <string>
#include <map>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

// TODO: DEBUG
std::ofstream outJobs("jobs.output");

//putem sa mutam implementarile ulterior intr-un cpp

int newId = 0; //variabila globala ce genereaza un nou id pentru fiecare task nou

//important: map-ul de jobs si de pathToId nu vor fi niciodata accesate de un job, ci doar de job system, deci nu trb mutex

const int TIMP_ASTEPTARE_THREAD = 5; //cat asteapta un thread pana incearca sa-si reia activitatea dupa ce user-ul l-a suspendat

struct Job;

void* threadWork(void* job);

enum class JobStatus
{
	Running,
	Paused,
	Ending
};

struct Job
{
	std::string path;
	int priority;
	pthread_t thr;
	pthread_mutex_t m;
	JobStatus status;
	int nrBytesCrt;
	int nrBytesTotal; //calculam procentul de finalizare ca 1.0 * nrBytesCrt / nrBytesTotal
	
	Job() = default;
	
	Job(const std::string& path, int priority, JobStatus status):
		path(path), priority(priority), status(status), nrBytesCrt(0), nrBytesTotal(0)
	{
	
	}
};

std::map<int, Job> jobs;
std::map<std::string, int> pathToId; //folosim asta ca sa aflam daca path-ul e deja in lucru de un job deja existent

std::string getPriority(int priority)
{
	if (priority == 1)
	{
		return "low";
	}

	if (priority == 2)
	{
		return "normal";
	}

	if (priority == 3)
	{
		return "high";
	}

	return "ERROR";
}

std::string addJob(const std::string& path, int priority = 1)
{
	// TODO: DEBUG
	outJobs << "\n\naddJobs()" << std::endl;
	outJobs << "path: " << path << std::endl;
	outJobs << "priority: " << priority << std::endl;
	
	// verifica daca exista path-ul in sistem
	{
		DIR *dir = opendir(path.c_str());

		if (dir == nullptr)
		{
			return "Directory '" + path + "' not found";
		}

		closedir(dir);
	}

	if (pathToId.find(path) == pathToId.end()) // nu exista jobul
	{
		++newId;
		pathToId[path] = newId;
		jobs[pathToId[path]].path = path;
		jobs[pathToId[path]].priority = priority;
		jobs[pathToId[path]].status = JobStatus::Running;
		
		if (pthread_mutex_init(&(jobs[pathToId[path]].m), NULL))
		{
			outJobs << "Error: In addJob pthread_mutex_init failed\n";
			perror(NULL);
			return "ERROR";
		}

		/* TODO: Error getting file status: No such file or directory
		if (pthread_create(&(jobs[pathToId[path]].thr), NULL, threadWork, &(jobs[pathToId[path]])))
		{
			outJobs << "Error: In addJob pthread_create failed\n";
			perror(NULL);
			return "ERROR";
		}
		*/
		
		return "Created analysis task with ID '" + std::to_string(pathToId[path]) + "' for '" + path + "' and priority '" + getPriority(priority) + '\'';
	}
	else //exista jobul (in acest caz doar dau update la prioritate)
	{
		pthread_mutex_lock(&(jobs[pathToId[path]].m));
		jobs[pathToId[path]].priority = priority;
		pthread_mutex_unlock(&(jobs[pathToId[path]].m));
		
		return "Directory '" + path + "' is already included in the analysis with ID '" + std::to_string(pathToId[path]) + "'. Only updated the priority to '" + std::to_string(priority) + '\'';
	}
}

void stopJob(int id) //seteaza flag-ul de status la un job ca Ending (thread-ul va incerca apoi sa se incheie cand acest lucru e safe)
{
	//verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		printf("Error: stopJob(%d): Invalid job id\n", id);
		return;
	}	

	pthread_mutex_lock(&(jobs.find(id)->second.m));
	jobs.find(id)->second.status = JobStatus::Ending;
	pthread_mutex_unlock(&(jobs.find(id)->second.m));
}

std::string pauseJob(int id) // seteaza flag-ul de status la un job ca Paused (thread-ul se va pune pe sleep cand va fi safe)
{
	// verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		return "ID '" + std::to_string(id) + "' not found";
	}
	
	pthread_mutex_lock(&(jobs.find(id)->second.m));
	jobs.find(id)->second.status = JobStatus::Paused;
	pthread_mutex_unlock(&(jobs.find(id)->second.m));

	return "Suspended task with ID '" + std::to_string(id) + '\'';
}

std::string unPauseJob(int id) // seteaza flag-ul de status la un job ca Running
{
	//verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		return "ID '" + std::to_string(id) + "' not found";
	}
	
	pthread_mutex_lock(&(jobs.find(id)->second.m));
	jobs.find(id)->second.status = JobStatus::Running;
	pthread_mutex_unlock(&(jobs.find(id)->second.m));

	return "Resumed task with ID '" + std::to_string(id) + '\'';
}

// TODO: de facut un task pt asta
void changePriority(int id, int priority) //reseteaza priority-ul unui job
{
	//verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		printf("Error: changePriority(%d, %d): Invalid job id\n", id, priority);
		return;
	}
	
	pthread_mutex_lock(&(jobs.find(id)->second.m));
	jobs.find(id)->second.priority = priority;
	pthread_mutex_unlock(&(jobs.find(id)->second.m));
}

std::string deleteJob(int id) //sterge job-ul cu id-ul dat
{
	//verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		return "ID '" + std::to_string(id) + "' not found";
	}
	
	/* TODO: EROARE: Error: In deleteJob(1) pthread_join failed -> No such file or directory 
	pthread_mutex_lock(&(jobs.find(id)->second.m));
	std::string pathJob = jobs.find(id)->second.path;
	pthread_mutex_unlock(&(jobs.find(id)->second.m));
	stopJob(id);

	if (pthread_join(jobs.find(id)->second.thr, NULL))
	{
		printf("Error: In deleteJob(%d) pthread_join failed\n", id);
		perror(NULL);
		return "ERROR";
	}

	pthread_mutex_destroy(&(jobs.find(id)->second.m));
	
	pathToId.erase(pathJob);
	jobs.erase(id);
	*/

	return "Removed task with ID '" + std::to_string(id) + '\'';
}

void processDirectory(const std::string& path, int& nrBytes, Job& job)
{
	//cod de verificat status thread
	//Paused
	pthread_mutex_lock(&job.m);
	while (job.status == JobStatus::Paused)
	{
		pthread_mutex_unlock(&job.m);
		
		///alt cod
		sleep(TIMP_ASTEPTARE_THREAD);
		
		pthread_mutex_lock(&job.m);
	}
	pthread_mutex_unlock(&job.m);
	
	//Ending
	pthread_mutex_lock(&job.m);
	if (job.status == JobStatus::Ending)
	{
		pthread_mutex_unlock(&job.m);
		return;
	}
	pthread_mutex_unlock(&job.m);
	//aici se incheie cod de verificare status thread

	DIR *dir = opendir(path.c_str());

	if (!dir)
	{
		printf("Error: In processDirectory(%s) opendir failed\n", path);
		perror("Error opening directory");
		return;
	}
	
	struct dirent *entry;
	struct stat fileStat;

	while ((entry = readdir(dir)) != NULL)
	{
		char filePath[1024];
		//snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);

		if (stat(filePath, &fileStat) < 0)
		{
			perror("Error getting file status");
			continue;
		}

		//este fisier sau director?
		if (S_ISDIR(fileStat.st_mode))
		{
			// Ignora directoarele "." (curent) si ".." (parintele)
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			{
				continue;
			}

			// Apelam recursiv
			processDirectory(filePath, nrBytes, job);
		}
		else //fisier
		{
			// Afisare fisier
			//printf("File: %s\n", filePath);

			// Adunam numarul de bytes
			
			pthread_mutex_lock(&job.m);
			nrBytes += fileStat.st_size;
			pthread_mutex_unlock(&job.m);
		}
	}
	
	closedir(dir);
}

void* threadWork(void* job) //functia rulata de fiecare thread in parte
{
	//cod de verificat status thread
	//Paused
	pthread_mutex_lock(&(((Job*)job)->m));
	while (((Job*)job)->status == JobStatus::Paused)
	{
		pthread_mutex_unlock(&(((Job*)job)->m));
		
		///alt cod
		sleep(TIMP_ASTEPTARE_THREAD);
		
		pthread_mutex_lock(&(((Job*)job)->m));
	}
	pthread_mutex_unlock(&(((Job*)job)->m));
	
	//Ending
	pthread_mutex_lock(&(((Job*)job)->m));
	if (((Job*)job)->status == JobStatus::Ending)
	{
		pthread_mutex_unlock(&(((Job*)job)->m));
		return NULL;
	}
	pthread_mutex_unlock(&(((Job*)job)->m));
	//aici se incheie cod de verificare status thread
	
	
	///cod propriu-zis

	pthread_mutex_lock(&(((Job*)job)->m));
	std::string path = ((Job*)job)->path;
	pthread_mutex_unlock(&(((Job*)job)->m));
	
	processDirectory(path, ((Job*)job)->nrBytesTotal,*((Job*)job)); //mai intai vedem cati bytes sunt in total, apoi mai parcurgem inca o data
	//facem 2 parcurgeri in loc de una doar ca sa putem afisa o evolutie liniara procentului de finalizare atunci cand il cere userul
	
	
	//alt cod pt a verifica daca user-ul vrea sa inchida thread-ul
	
	//Paused
	pthread_mutex_lock(&(((Job*)job)->m));
	while (((Job*)job)->status == JobStatus::Paused)
	{
		pthread_mutex_unlock(&(((Job*)job)->m));
		
		///alt cod
		sleep(TIMP_ASTEPTARE_THREAD);
		
		pthread_mutex_lock(&(((Job*)job)->m));
	}
	pthread_mutex_unlock(&(((Job*)job)->m));
	
	//Ending
	pthread_mutex_lock(&(((Job*)job)->m));
	if (((Job*)job)->status == JobStatus::Ending)
	{
		pthread_mutex_unlock(&(((Job*)job)->m));
		return NULL;
	}
	pthread_mutex_unlock(&(((Job*)job)->m));
	
	//cod propriu-zis
	
	
	processDirectory(path, ((Job*)job)->nrBytesCrt, (*((Job*)job)));
	
	//un thread se incheie doar daca utilizatorul a cerut explicit asta (altfel sta degeaba intr-un while)
	pthread_mutex_lock(&(((Job*)job)->m));
	while (((Job*)job)->status != JobStatus::Ending)
	{
		pthread_mutex_unlock(&(((Job*)job)->m));
		
		//alt cod
		sleep(TIMP_ASTEPTARE_THREAD);
		
		pthread_mutex_lock(&(((Job*)job)->m));
	}
	
	return NULL; //
}

void displayJobs() //afiseaza job-urile active
{
	for (auto& it : jobs)
	{
		pthread_mutex_lock(&(it.second).m);
		printf("Id: %d Path: %s Number of bytes processed %d ", it.first, (it.second).path, (it.second).nrBytesCrt);
		if ((it.second).nrBytesTotal > 0)
			printf("Done: %lf\n", 1.0 * (it.second).nrBytesCrt / (it.second).nrBytesTotal);
		else
			printf("Done: %lf\n", 0.0);
		pthread_mutex_unlock(&(it.second).m);
	}
}

