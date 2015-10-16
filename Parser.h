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
struct thread;
struct process;

bool isInteger(const string & s);

// Fetch the uptime from the stat file in the location passed as an argument
float getUpTime(const string& path);

// Parse the thread stat file and get the relevant information needed
thread getThreadInfo(const string& path);

process getProcessInfo(const string& path);

// Returns an updated list of the existing processes
vector<process> refreshProcesses();

void updateProcessData(vector<process>& existingProcesses);

vector<int> getProcessCPULoad(vector<process>& existingProcesses, int index);


#endif