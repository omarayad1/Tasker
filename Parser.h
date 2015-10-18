#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
using namespace std;


extern int hertz;
struct thread{
    string name;
    string pid;
    string ppid;
    int cpu;
    float usagePercentage;
    unsigned long long curUpTime, prevUpTime;
    thread():curUpTime(0), prevUpTime(0){};
};

struct process{
    string name;
    string pid;
    string ppid;
    vector<thread> threads;
};

struct cpu {
    string name;
    float usagePercentage;
    unsigned long long totalTime, prevTotalTime, idleTime, prevIdleTime;
    cpu(): totalTime(0), prevTotalTime(0), idleTime(0), prevIdleTime(0) {};
};

struct cpu_data {
	string name;
	unsigned long long total;
	unsigned long long idle;
	float usage;
};

struct process_data {
	string name;
	string pid;
    string ppid;
	double time;
	int cpu;
	float usage;
};

std::vector<cpu> refreshCPU();
std::vector<cpu_data> updateCPUData(std::vector<cpu>& cpus);
int getCPU(const string& path);

bool isInteger(const string & s);

// Fetch the uptime from the stat file in the location passed as an argument
float getUpTime(const string& path, int& cpu);

// Parse the thread stat file and get the relevant information needed
thread getThreadInfo(const string& path, const string& ppid);

process getProcessInfo(const string& path);

// Returns an updated list of the existing processes
vector<process> refreshProcesses();

std::vector<process_data> updateProcessData(vector<process>& existingProcesses, std::vector<cpu>& cpus);

vector<int> getProcessCPULoad(vector<process>& existingProcesses, std::vector<cpu>& cpus, string pid);


#endif