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
struct thread, process;

bool isInteger(const string & s);

// Fetch the uptime from the stat file in the location passed as an argument
float getUpTime(const string& path);

// Parse the thread stat file and get the relevant information needed
thread getThreadInfo(const string& path);

process getProcessInfo(const string& path);

// Returns an updated list of the existing processes
vector<process> refreshProcesses();

void updateProcessData(vector<process>& existingProcesses);


int main() {
    int hertz = sysconf(_SC_CLK_TCK);
    vector<process> existingProcesses = refreshProcesses();
    while (true)
    {
        // The actual number we want is curUpTime - prevUpTime which I calculate bellow
        // That's the time the thread has been running for the past second
        // We need to monitor the running time of each thread of the process 
        // To find out the total time the process has run on each cpu
        // We can't just monitor the process itself because each of its
        // Threads might be running on a separate cpu
        // Note: I have not parsed the cpu information (how many cpus there are, their running time (calculated as curUpTime - prevUpTime), etc...)
        //          Or which cpu each thread is running on, we will need both
        
        // Updates the uptimes of the existingProcesses

        sleep(1);
    }
    return 0;
}

#endif