#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include "Parser.h"
using namespace std;


int hertz = sysconf(_SC_CLK_TCK);

struct thread{
    string name;
    string pid;
    
    int cpu; // still needs to be parsed
    unsigned long curUpTime, prevUpTime;
    thread():curUpTime(0), prevUpTime(0){};
};

struct process{
    string name;
    string pid;
    string ppid;
    vector<thread> threads;
};

bool isInteger(const string & s)
{
   char * p ;
   strtol(s.c_str(), &p, 10);
   return (*p == 0) ;
}

// Fetch the uptime from the stat file in the location passed as an argument
float getUpTime(const string& path)
{
    ifstream inputFile(path.c_str());
    for (int i=0; i<13; i++) inputFile.ignore(10000, ' ');
    unsigned long utime, stime, cutime, cstime;
    inputFile >> utime >> stime >> cutime >> cstime;
    return utime + stime + cutime + cstime;
}

// Parse the thread stat file and get the relevant information needed
thread getThreadInfo(const string& path)
{
    thread t;
    ifstream inputFile((path + string("/stat")).c_str());
    inputFile >> t.pid >> t.name;
    for(int i=0; i<11; i++) inputFile.ignore(10000, ' ');
    unsigned long utime, stime, cutime, cstime;
    inputFile >> utime >> stime >> cutime >> cstime;
    t.curUpTime = utime + stime + cutime + cstime;
    return t;
}

process getProcessInfo(const string& path)
{
    // Parse the process stat file and get the relevant information needed
    process info;
    ifstream inputFile((path + string("/stat")).c_str());
    inputFile >> info.pid >> info.name >> info.ppid >> info.ppid;

    // Parse the threads of the process and push them in the process struct
    struct dirent *pDirent;
    DIR *pDir;
    string taskPath = path + string("/task/");
    pDir = opendir (taskPath.c_str());
    while ((pDirent = readdir(pDir)) != NULL) 
        if (isInteger(pDirent->d_name)) 
            info.threads.push_back(getThreadInfo((taskPath + string(pDirent->d_name)).c_str()));
            
    return info;
}


// Returns an updated list of the existing processes
vector<process> refreshProcesses()
{
    vector<process> existingProcesses;
    struct dirent *pDirent;
    DIR *pDir;
    pDir = opendir ("/proc");
    while ((pDirent = readdir(pDir)) != NULL) 
    {
        if (isInteger(pDirent->d_name)) 
        {
            string path = "/proc/";
            path.append(pDirent->d_name);
            existingProcesses.push_back(getProcessInfo(path));
        }
    }
    closedir (pDir);
    return existingProcesses;
}

void updateProcessData(vector<process>& existingProcesses)
{
    for (int i=0; i<existingProcesses.size(); i++)
    {
        for (int j=0; j<existingProcesses[i].threads.size(); j++)
        {
            string path = "/proc/" + existingProcesses[i].pid + "/task/" + existingProcesses[i].threads[j].pid + "/stat";
            swap(existingProcesses[i].threads[j].curUpTime, existingProcesses[i].threads[j].prevUpTime);
            existingProcesses[i].threads[j].curUpTime = getUpTime(path);
            double threadRunningTimeInSeconds = (existingProcesses[i].threads[j].curUpTime - existingProcesses[i].threads[j].prevUpTime)/(hertz*1.0);
            cout << existingProcesses[i].threads[j].pid << ' ' << threadRunningTimeInSeconds << endl;
        }
        cout << endl;
    }
}

vector<int> getProcessCPULoad(vector<process>& existingProcesses, int index)
{
    vector<int> CPULoad;
    CPULoad.push_back(0.1);
    CPULoad.push_back(0.2);
    CPULoad.push_back(0.3);
    CPULoad.push_back(0.4);
    CPULoad.push_back(0.5);
    CPULoad.push_back(0.6);
    CPULoad.push_back(0.7);
    return CPULoad;
}



int main() {
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
        updateProcessData(existingProcesses);
        sleep(1);
    }
    return 0;
}
