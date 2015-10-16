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
    
    int cpu;
    int usagePercentage;
    unsigned long curUpTime, prevUpTime;
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
    unsigned long totalTime, prevTotalTime, idleTime, prevIdleTime;
    cpu(): totalTime(0), prevTotalTime(0), idleTime(0), prevIdleTime(0) {};
};

string exec(string cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}

bool isInteger(const string & s)
{
   char * p ;
   strtol(s.c_str(), &p, 10);
   return (*p == 0) ;
}

std::vector<cpu> refreshCPU() {
    ifstream inputFile("/proc/stat");
    std::vector<cpu> cpus;
    cout << "Get CPUs: " << endl;
    unsigned long t_user, t_nice, t_system, t_idle, t_iowait, t_irq, t_softirq, t_steal, t_guest, t_guest_nice, t_nonIdle;
    
    while(true) {
        cpu c;
        inputFile >> c.name;
        cout << "name: " << c.name << "-" << c.name.substr(0,3) << endl;
        if(c.name.substr(0,3) != "cpu") break;
        inputFile >> t_user >> t_nice >> t_system >> t_idle >> t_iowait >> t_irq >> t_softirq >> t_steal >> t_guest >> t_guest_nice;
        c.idleTime = t_idle + t_iowait;
        t_nonIdle = t_user + t_nice + t_system + t_irq + t_softirq + t_steal;
        c.totalTime = c.idleTime + t_nonIdle;
        cpus.push_back(c);
    }
    
    return cpus;
}

void updateCPUData(std::vector<cpu>& cpus) {
    ifstream inputFile("/proc/stat");
    unsigned long t_user, t_nice, t_system, t_idle, t_iowait, t_irq, t_softirq, t_steal, t_guest, t_guest_nice, t_nonIdle, idx, delta_total, delta_idle;
    cout << "Core Count: " << cpus.size() << endl;
    
    for (int idx = 0; idx < cpus.size(); idx++) {
        inputFile >> cpus[idx].name;
        
        cout << "CPU: " << cpus[idx].name << ", ";
        
        cpus[idx].prevIdleTime = cpus[idx].idleTime;
        cpus[idx].prevTotalTime = cpus[idx].totalTime;
        
        inputFile >> t_user >> t_nice >> t_system >> t_idle >> t_iowait >> t_irq >> t_softirq >> t_steal >> t_guest >> t_guest_nice;
        
        cpus[idx].idleTime = t_idle + t_iowait;
        t_nonIdle = t_user + t_nice + t_system + t_irq + t_softirq + t_steal;
        cpus[idx].totalTime = cpus[idx].idleTime + t_nonIdle;
        
        delta_total = cpus[idx].totalTime - cpus[idx].prevTotalTime;
        delta_idle = cpus[idx].idleTime - cpus[idx].prevIdleTime;
        
        cout << "total: " << delta_total << ", idle: " << delta_idle << ", ";
        
        cpus[idx].usagePercentage = (delta_total - delta_idle)/(delta_total*1.0);
        
        cout << "percentage: " << cpus[idx].usagePercentage * 100 << "%" << endl;
    }
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

// Fetch the uptime from the stat file in the location passed as an argument
int getCPU(const string& path)
{
    return atoi(exec(string("cat ").append(path).append(" | cut -d \\  -f 39")).c_str());
}

// Parse the thread stat file and get the relevant information needed
thread getThreadInfo(const string& path)
{
    thread t;
    ifstream inputFile((path + string("/stat")).c_str());
    inputFile >> t.pid >> t.name; //are these the first 2 entries in the file???
    for(int i=0; i<13; i++) inputFile.ignore(10000, ' ');
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
            existingProcesses[i].threads[j].cpu = getCPU(path); // Just in case the OS moves the thread from one core to another
            double threadRunningTimeInSeconds = (existingProcesses[i].threads[j].curUpTime - existingProcesses[i].threads[j].prevUpTime)/(hertz*1.0);
            cout << existingProcesses[i].threads[j].pid << ' ' << threadRunningTimeInSeconds << ' ' << existingProcesses[i].threads[j].cpu << endl;
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
    std::vector<cpu> existingCPU = refreshCPU();
    
    for (int i = 0; i < existingCPU.size(); i++) {
        cout << existingCPU[i].name << ", idle= " << existingCPU[i].idleTime << ", total= " << existingCPU[i].totalTime << endl;
    }
    
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
        updateCPUData(existingCPU);
        
        sleep(1);
    }
    return 0;
}