#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
using namespace std;


struct thread{
    string name;
    int pid;
    int cpu;
    float curUpTime, prevUpTime;
    thread():curUpTime(0), prevUpTime(0){};
};

struct process{
    string name;
    int pid;
    int cpu;
    vector<thread> threads;
    process* parent;
};

bool isInteger(const string & s)
{
   char * p ;
   strtol(s.c_str(), &p, 10);
   return (*p == 0) ;
}

thread getThreadInfo(const string& path)
{
    thread t;
    ifstream inputFile((path + string("/stat")).c_str());
    inputFile >> t.pid >> t.name;
    for(int i=0; i<11; i++) inputFile.ignore(10000, ' ');
    float utime, stime, cutime, cstime;
    inputFile >> utime >> stime >> cutime >> cstime;
    t.curUpTime = utime + stime + cutime + cstime;
    return t;
}

process getProcessInfo(const string& path)
{
    // Parse the process stat file and get the relevant information needed
    process info;
    ifstream inputFile((path + string("/stat")).c_str());
    inputFile >> info.pid >> info.name;
    /*
    for(int i=0; i<11; i++) inputFile.ignore(10000, ' ');
    float utime, stime, cutime, cstime;
    inputFile >> utime >> stime >> cutime >> cstime;
    info.curUpTime = utime + stime + cutime + cstime;
    */
    
    // Parse the threads of the process and push them in the process struct
    struct dirent *pDirent;
    DIR *pDir;
    string taskPath = path + string("/task/");
    pDir = opendir (taskPath.c_str());
    while ((pDirent = readdir(pDir)) != NULL) {
        if (isInteger(pDirent->d_name))
        {
            printf ("[%s]\n", pDirent->d_name);
            thread t = getThreadInfo((taskPath + string(pDirent->d_name)).c_str());
            cout << "Thread: " << t.pid << ' ' << t.name << ' ' << t.curUpTime << endl;
        }
        
    }
    return info;
}

int main() {
    struct dirent *pDirent;
    DIR *pDir;

    pDir = opendir ("/proc");

    while ((pDirent = readdir(pDir)) != NULL) {
        if (isInteger(pDirent->d_name))
        {
            string path = "/proc/";
            path.append(pDirent->d_name);
            process p = getProcessInfo(path);
            cout << "Belong to Process: " << p.pid << ' ' <<  p.name << endl;  
            cout << endl << endl;
        }
    }
    closedir (pDir);

    return 0;
}
