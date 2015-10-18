#include <iostream>
#include <sstream>
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
    unsigned long long t_user, t_nice, t_system, t_idle, t_iowait, t_irq, t_softirq, t_steal, t_guest, t_guest_nice, t_nonIdle;
    
    while(true) {
        cpu c;
        inputFile >> c.name;
        if(c.name.substr(0,3) != "cpu") break;
        inputFile >> t_user >> t_nice >> t_system >> t_idle >> t_iowait >> t_irq >> t_softirq >> t_steal >> t_guest >> t_guest_nice;
        c.idleTime = t_idle + t_iowait;
        t_nonIdle = t_user + t_nice + t_system + t_irq + t_softirq + t_steal;
        c.totalTime = c.idleTime + t_nonIdle;
        cpus.push_back(c);
    }
    
    return cpus;
}

std::vector<cpu_data> updateCPUData(std::vector<cpu>& cpus) {
    ifstream inputFile("/proc/stat");
    unsigned long long t_user, t_nice, t_system, t_idle, t_iowait, t_irq, t_softirq, t_steal, t_guest, t_guest_nice, t_nonIdle, idx, delta_total, delta_idle;
    std::vector<cpu_data> data;
    struct cpu_data element;
    
    for (int idx = 0; idx < cpus.size(); idx++) {
        inputFile >> cpus[idx].name;
        
        
        cpus[idx].prevIdleTime = cpus[idx].idleTime;
        cpus[idx].prevTotalTime = cpus[idx].totalTime;
        
        inputFile >> t_user >> t_nice >> t_system >> t_idle >> t_iowait >> t_irq >> t_softirq >> t_steal >> t_guest >> t_guest_nice;
        
        cpus[idx].idleTime = t_idle + t_iowait;
        t_nonIdle = t_user + t_nice + t_system + t_irq + t_softirq + t_steal;
        cpus[idx].totalTime = cpus[idx].idleTime + t_nonIdle;
        
        delta_total = cpus[idx].totalTime - cpus[idx].prevTotalTime;
        delta_idle = cpus[idx].idleTime - cpus[idx].prevIdleTime;
        
        
        cpus[idx].usagePercentage = delta_total == 0 ? 0 : (delta_total - delta_idle)/(delta_total*1.0);
        
        element.name=cpus[idx].name;
        element.total=delta_total;
        element.idle=delta_idle;
        element.usage=cpus[idx].usagePercentage * 100;
        data.push_back(element);
    }
    return data;
}

// Fetch the uptime from the stat file in the location passed as an argument
float getUpTime(const string& path, int& cpu)
{
    ifstream inputFile(path.c_str());
    string tmp;
    /*
    NOTE: We had a bug here. Some process names are like '(vfs-worker {"pi)' (they do have spaces, and therefore, we get the wrong time, or cpu).
    Therefore, a workaround is to match the status character (in pos 3 -- http://man7.org/linux/man-pages/man5/proc.5.html),
    Thus, we will keep parsing untill we hit the status character, and then fallback to the original way Youssef came up with.
    (Also, we will ignore 10 positions )
    - Yehia & Safaa
    */
    int x = 0;
    do {
        x++;
        inputFile >> tmp;
    } while(tmp != "R" &&tmp != "S" &&tmp != "D" &&tmp != "Z" &&tmp != "T" &&tmp != "t" &&tmp != "W" &&tmp != "X" &&tmp != "x" &&tmp != "K" &&tmp != "P");
        
    for (int i=0; i<10; i++) inputFile.ignore(10000, ' ');
    unsigned long long utime, stime, cutime, cstime;
    inputFile >> utime >> stime >> cutime >> cstime;
    for (int i=0; i<23; i++) inputFile.ignore(10000, ' ');
    inputFile >> cpu;
    return utime + stime + cutime + cstime;
}

// Fetch the uptime from the stat file in the location passed as an argument
int getCPU(const string& path)
{
    return atoi(exec(string("cat ").append(path).append(" | cut -d \\  -f 39")).c_str());
}

// Parse the thread stat file and get the relevant information needed
thread getThreadInfo(const string& path, const string& ppid)
{
    thread t;
    ifstream inputFile((path + string("/stat")).c_str());
    inputFile >> t.pid >> t.name; //are these the first 2 entries in the file???
    for (int i=0; i<10; i++) inputFile.ignore(10000, ' ');
    unsigned long long utime, stime, cutime, cstime;
    inputFile >> utime >> stime >> cutime >> cstime;
    t.curUpTime = utime + stime + cutime + cstime;
    t.ppid = ppid;
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
            info.threads.push_back(getThreadInfo((taskPath + string(pDirent->d_name)).c_str(), info.pid));
            
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

std::vector<process_data> updateProcessData(vector<process>& existingProcesses, std::vector<cpu>& cpus)
{
    struct process_data element;
    std::vector<process_data> data;
    for (int i=0; i<existingProcesses.size(); i++)
    {
        /*update threads before looping over them*/
        for (int j=0; j<existingProcesses[i].threads.size(); j++)
        {
            string path = "/proc/" + existingProcesses[i].pid + "/task/" + existingProcesses[i].threads[j].pid + "/stat";
            swap(existingProcesses[i].threads[j].curUpTime, existingProcesses[i].threads[j].prevUpTime);
            int _cpu;
            existingProcesses[i].threads[j].curUpTime = getUpTime(path, _cpu);
            existingProcesses[i].threads[j].cpu = _cpu;
            /*
            YOUSSEF: Why do we need to divide by hertz? (I assume all inputs from files are of the same type/unit?)
            */
            double threadRunningTimeInSeconds = (existingProcesses[i].threads[j].curUpTime - existingProcesses[i].threads[j].prevUpTime);
            existingProcesses[i].threads[j].usagePercentage = threadRunningTimeInSeconds / ((cpus[_cpu+1].totalTime - cpus[_cpu+1].idleTime)*1.0);
            element.name = existingProcesses[i].threads[j].name;
            element.pid = existingProcesses[i].threads[j].pid;
            element.ppid = existingProcesses[i].threads[j].ppid;
            element.time = threadRunningTimeInSeconds;
            element.cpu = existingProcesses[i].threads[j].cpu;
            element.usage = existingProcesses[i].threads[j].usagePercentage*100;
            data.push_back(element);
        }
    }
    return data;
}

vector<int> getProcessCPULoad(vector<process>& existingProcesses, std::vector<cpu>& cpus, string pid)
{
    vector<int> CPULoad;
    int index;
    for (int i=0; i<existingProcesses.size(); i++){
        if (existingProcesses[i].pid == pid) {
            index = i;
            break;
        }
    }
    for (int i = 0; i < cpus.size(); i++) {
        double totalProcessCoreTime = 0;
        for (int j = 0; j < existingProcesses[index].threads.size(); j++) {
            ostringstream ss;
            ss << "cpu" << existingProcesses[index].threads[j].cpu;
            if(cpus[i].name == ss.str()) {
                totalProcessCoreTime += (existingProcesses[index].threads[j].curUpTime - existingProcesses[index].threads[j].prevUpTime);
            }
        }
        CPULoad.push_back(totalProcessCoreTime/(cpus[i].totalTime-cpus[i].idleTime));
    }
    return CPULoad;
}
