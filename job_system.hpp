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
	int nrFiles;
	int nrDir;
	bool finished;
	
	Job() = default;
	
	Job(const std::string& path, int priority, JobStatus status):
		path(path), priority(priority), status(status), nrBytesCrt(0), nrBytesTotal(0), nrFiles(0), nrDir(0), finished(false)
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

		if (pthread_create(&(jobs[pathToId[path]].thr), NULL, threadWork, &(jobs[pathToId[path]])))
		{
			outJobs << "Error: In addJob pthread_create failed\n";
			perror(NULL);
			return "ERROR";
		}

     	int newPriority;
        switch (priority) { // set priority
            case 1:
                newPriority = sched_get_priority_min(SCHED_OTHER); // schedule policy min
                break;
            case 2:
                newPriority = sched_get_priority_max(SCHED_OTHER) / 2; // normal
                break;
            case 3:
                newPriority = sched_get_priority_max(SCHED_OTHER); // high
                break;
            default:
                outJobs << "Invalid priority level" << std::endl;
                return "ERROR";
        }

       if (pthread_setschedprio(jobs[pathToId[path]].thr, newPriority) != 0) {
            outJobs << "Failed to set thread priority" << std::endl;
            return "ERROR";
        }
		
		return "Created analysis task with ID '" + std::to_string(pathToId[path]) + "' for '" + path + "' and priority '" + getPriority(priority) + '\'';
	}
	else // exista jobul (in acest caz doar dau update la prioritate)
	{
		pthread_mutex_lock(&(jobs[pathToId[path]].m));
		jobs[pathToId[path]].priority = priority;

     	int newPriority;
        switch (priority) { // set priority
            case 1:
                newPriority = sched_get_priority_min(SCHED_OTHER); // schedule policy min
                break;
            case 2:
                newPriority = sched_get_priority_max(SCHED_OTHER) / 2; // normal
                break;
            case 3:
                newPriority = sched_get_priority_max(SCHED_OTHER); // high
                break;
            default:
                outJobs << "Invalid priority level" << std::endl;
                return "ERROR";
        }

        if (pthread_setschedprio(jobs[pathToId[path]].thr, newPriority) != 0) {
            outJobs << "Failed to set thread priority" << std::endl;
            return "ERROR";
        }

		pthread_mutex_unlock(&(jobs[pathToId[path]].m));
		return "Directory '" + path + "' is already included in the analysis with ID '" + std::to_string(pathToId[path]) + "'. Only updated the priority to '" + std::to_string(priority) + '\'';
	}
	
	printf("ok");
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

std::string pauseJob(int id) // seteaza flag-ul de status a un job ca Paused (thread-ul se va pune pe sleep cand va fi safe)
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
	// verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		return "ID '" + std::to_string(id) + "' not found";
	}
	
	pthread_mutex_lock(&(jobs.find(id)->second.m));
	jobs.find(id)->second.status = JobStatus::Running;
	pthread_mutex_unlock(&(jobs.find(id)->second.m));

	return "Resumed task with ID '" + std::to_string(id) + '\'';
}

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

 	int newPriority;
    switch (priority) {
        case 1:
            newPriority = sched_get_priority_min(SCHED_OTHER); // scheduling policy, min
            break;
        case 2:
            newPriority = sched_get_priority_max(SCHED_OTHER) / 2; // normal
            break;
        case 3:
            newPriority = sched_get_priority_max(SCHED_OTHER); // max
            break;
        default:
            outJobs << "Invalid priority level" << std::endl;
    }

    if (pthread_setschedprio(jobs.find(id)->second.thr, newPriority) != 0) { // set priority
        outJobs << "Failed to set thread priority" << std::endl;
        return;
    }

	pthread_mutex_unlock(&(jobs.find(id)->second.m));
}

std::string deleteJob(int id) //sterge job-ul cu id-ul dat
{
	// verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		return "ID '" + std::to_string(id) + "' not found";
	}
	
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

	return "Removed task with ID '" + std::to_string(id) + '\'';
}

void processDirectory(const std::string& path, int& nrBytes, Job& job, int indexOfScan)
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
		snprintf(filePath, sizeof(filePath), "%s/%s", path.c_str(), entry->d_name);

		if (stat(filePath, &fileStat) < 0)
		{
			perror("Error getting file status");
			continue;
		}

		// este shortcut
		if (S_ISLNK(fileStat.st_mode)) {
			nrBytes += fileStat.st_size;
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
			processDirectory(filePath, nrBytes, job, indexOfScan);
			
			if (indexOfScan == 1) // a doua parcurgere
			{
				pthread_mutex_lock(&job.m);
				++job.nrDir;
				pthread_mutex_unlock(&job.m);
			}
		}
		else //fisier
		{
			// Afisare fisier
			//printf("File: %s\n", filePath);

			// Adunam numarul de bytes
			
			pthread_mutex_lock(&job.m);
			nrBytes += fileStat.st_size;
			if (indexOfScan == 1) // a doua parcurgere
				++job.nrFiles;
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
	
	processDirectory(path, ((Job*)job)->nrBytesTotal,*((Job*)job), 0); //mai intai vedem cati bytes sunt in total, apoi mai parcurgem inca o data
	//facem 2 parcurgeri in loc de una doar ca sa putem afisa o evolutie liniara procentului de finalizare atunci cand il cere userul
	
	pthread_mutex_lock(&(((Job*)job)->m));
	((Job*)job)->finished = true;
	pthread_mutex_unlock(&(((Job*)job)->m));
	
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
	
	
	processDirectory(path, ((Job*)job)->nrBytesCrt, (*((Job*)job)), 1);
	
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

std::string displayJobs() //afiseaza job-urile active
{
	std::string msg = 
	" ID  PRI   Path                  Done Status              Details\n";
//	" 12  ***   /home/user/...        45% in progress          2306 files, 11 dir"

	/* TODO: pune tot in msg:
		- ID
		- PRI 			-> * / ** / ***
		- Path 			-> /home/user/...
		- Done Status 	-> 45% in progress
		- Details 		-> 2306 files, 11 dirs
	*/

	for (auto& it : jobs)
	{
		pthread_mutex_lock(&(it.second).m);
		msg += std::to_string(it.first) + " ";
		for (int i = 0; i < it.second.priority; ++i)
			msg += "*";
		msg += " ";
		msg += it.second.path + " ";
		if (it.second.nrBytesTotal > 0)
			msg += std::to_string((int)(100.0 * it.second.nrBytesCrt / it.second.nrBytesTotal));
		else
			msg += "0";
		msg += "% in progress ";
		
		msg += std::to_string(it.second.nrFiles) + " files, ";
		msg += std::to_string(it.second.nrDir) + " dir";
		
		msg += "\n";
		
		pthread_mutex_unlock(&(it.second).m);
	}

	return msg;
}

std::string infoJob(int id)
{
	// verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		return "ID '" + std::to_string(id) + "' not found";
	}

	// TODO: cred ca aici ar trb sa afisam doar status (preparing, inprogress, done)
	
	std::string msg = 
	" ID  PRI   Path                  Done Status              Details\n";
	
	auto it = jobs.find(id);
	
	pthread_mutex_lock(&(it->second).m);
	msg += std::to_string(it->first) + " ";
	for (int i = 0; i < (it->second).priority; ++i)
		msg += "*";
	msg += " ";
	msg += (it->second).path + " ";
	if ((it->second).nrBytesTotal > 0)
		msg += std::to_string((int)(100.0 * (it->second).nrBytesCrt / (it->second).nrBytesTotal));
	else
		msg += "0";
	msg += "% in progress ";
	
	msg += std::to_string((it->second).nrFiles) + " files, ";
	msg += std::to_string((it->second).nrDir) + " dir";
	
	msg += "\n";
	
	pthread_mutex_unlock(&(it->second).m);

	return msg;
}

std::string printAnalysisReportDirectory(const std::string& root, const std::string& sub, const int& nrBytesTotal, int& nrBytes)
{
	std::string path = root + sub;
	std::string msg;
	
	// determina dimensiunea lui path
	DIR *dir = opendir(path.c_str());

	if (!dir)
	{
		return "ERROR";
	}
	
	struct dirent *direntp;

	while ((direntp = readdir(dir)) != NULL)
	{
		// Ignora directoarele "." (curent) si ".." (parintele)
		if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
		{
			continue;
		}

		if (direntp->d_type == DT_LNK) // symbolic link
		{
			std::string filePath = path + "/" + direntp->d_name;
			
			struct stat st;
			stat(filePath.c_str(), &st);
			nrBytes += st.st_size;
		}
		else if (direntp->d_type == DT_REG)	// regular file
		{
			std::string filePath = path + "/" + direntp->d_name;

			struct stat st;
			stat(filePath.c_str(), &st);
			nrBytes += st.st_size;
		}
		else if (direntp->d_type == DT_DIR)	// directory
		{
			std::string subDir = sub + '/' + direntp->d_name;

			int nrBytesSub = 0;
			msg += printAnalysisReportDirectory(root, subDir, nrBytesTotal, nrBytesSub);

			if (sub.size() == 0)
			{
				msg += "\n|";
			}

			nrBytes += nrBytesSub;
		}
	}
	
	closedir(dir);

	std::string currentMsg;

	if (sub.size() == 0)
	{
		// path
		currentMsg = path;
	}
	else
	{
		// path
		currentMsg = "\n|-" + sub;
	}

	// usage
	currentMsg += "   " + std::to_string(100.0 * nrBytes / nrBytesTotal) + "%";

	// size
	currentMsg += "   " + std::to_string(1.0 * nrBytes / 1024.0 / 1024.0) + "MB";

	// new line
	if (sub.size() == 0)
	{
		currentMsg += "\n|";
	}

	return currentMsg + msg;
}

std::string printAnalysisReport(int id)
{
	// verifica mai intai daca jobul este gata
	auto it = jobs.find(id);

	pthread_mutex_lock(&(it->second).m);
	
	if (!it->second.finished)
	{
		pthread_mutex_unlock(&(it->second).m);
		return "The task with ID " + std::to_string(it->first) + " is not finished";
	}
 
	// TODO: foloseste CACHE daca il avem
	int nrBytes = 0;
	std::string msg = "\nPath          Usage          Size\n";
	msg += printAnalysisReportDirectory(it->second.path, "", it->second.nrBytesTotal, nrBytes);

	// TODO: debug -> nu avem mereu acelasi nr de bytes
	outJobs << std::endl << it->second.nrBytesTotal << " /vs/ " << nrBytes << std::endl;

	pthread_mutex_unlock(&(it->second).m);

	return msg;
} 

