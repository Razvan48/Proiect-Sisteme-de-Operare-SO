#pragma once

#include <iostream>

#include <vector>
#include <string>
#include <map>
#include <thread>
#include <mutex>

#include <memory>

#include <stdlib.h>

//putem sa mutam implementarile ulterior intr-un cpp

int newId = 0; //variabila globala ce genereaza un nou id pentru fiecare task nou

//important: map-ul de jobs si de pathToId nu vor fi niciodata accesate de un job, ci doar de job system, deci nu trb mutex

struct Job;

void threadWork(Job& job);

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
	std::thread thr;
	std::mutex m;
	JobStatus status;
	
	Job() = default;
	
	Job(const std::string& path, int priority):
		path(path), priority(priority), status(JobStatus::Running), thr(threadWork, std::ref(*this)) //sper sa fie ok
	{
		// TO DO: de initializat mutexul thread-ului mai intai (cred ca se intampla de la sine)
	}
};

std::map<int, Job> jobs;
std::map<std::string, int> pathToId; //folosim asta ca sa aflam daca path-ul e deja in lucru de un job deja existent

void addJob(const std::string& path, int priority = 1)
{
	if (pathToId.find(path) == pathToId.end()) // nu exista jobul
	{
		++newId;
		pathToId[path] = newId;
		//jobs[pathToId[path]] = {path, priority};
		//jobs.emplace(std::make_pair((int)pathToId[path], Job(path, priority)));
		//jobs[pathToId[path]](path, priority);
		jobs.insert(pathToId[path], Job(path, priority)); //TO DO: nu merge
	}
	else //exista jobul (in acest caz doar dau update la prioritate)
	{
		jobs[pathToId[path]].m.lock();
		
		jobs[pathToId[path]].priority = priority;
		
		jobs[pathToId[path]].m.unlock();
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
	
	jobs.find(id)->second.m.lock();
	jobs.find(id)->second.status = JobStatus::Ending;
	jobs.find(id)->second.m.unlock();	
}

void pauseJob(int id) // seteaza flag-ul de status la un job ca Paused (thread-ul se va pune pe sleep cand va fi safe)
{
	//verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		printf("Error: pauseJob(%d): Invalid job id\n", id);
		return;
	}
	
	jobs.find(id)->second.m.lock();
	jobs.find(id)->second.status = JobStatus::Paused;
	jobs.find(id)->second.m.unlock();
}

void unPauseJob(int id) // seteaza flag-ul de status la un job ca Running
{
	//verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		printf("Error: unPauseJob(%d): Invalid job id\n", id);
		return;
	}
	
	jobs.find(id)->second.m.lock();
	jobs.find(id)->second.status = JobStatus::Running;
	jobs.find(id)->second.m.unlock();
}

void changePriority(int id, int priority) //reseteaza priority-ul unui job
{
	//verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		printf("Error: changePriority(%d, %d): Invalid job id\n", id, priority);
		return;
	}
	
	jobs.find(id)->second.m.lock();
	jobs.find(id)->second.priority = priority;
	jobs.find(id)->second.m.unlock();
}

void deleteJob(int id) //sterge job-ul cu id-ul dat
{
	//verificam daca exista id-ul respectiv
	if (jobs.find(id) == jobs.end())
	{
		printf("Error: deleteJob(%d): Invalid job id\n", id);
		return;
	}
	
	std::string pathJob = jobs.find(id)->second.path;
	pathToId.erase(pathJob);
	stopJob(id);
	jobs.erase(id);
}

void threadWork(Job& job) //functia rulata de fiecare thread in parte
{
	// TO DO
	
	//mutex lock si unlock-uri folosind job.m
	
	/*
	while (job.status == JobStatus::Running)
	{
		// TO DO
	}
	*/
}






