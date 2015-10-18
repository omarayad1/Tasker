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

int main() {
    vector<process> existingProcesses = refreshProcesses();
    std::vector<cpu> existingCPU = refreshCPU();
    
    for (int i = 0; i < existingCPU.size(); i++) {
        cout << existingCPU[i].name << ", idle= " << existingCPU[i].idleTime << ", total= " << existingCPU[i].totalTime << endl;
    }
    
    int x = 0;
    while (x < 2)
    {
        x++;
        // The actual number we want is curUpTime - prevUpTime which I calculate bellow
        // That's the time the thread has been running for the past second
        // We need to monitor the running time of each thread of the process 
        // To find out the total time the process has run on each cpu
        // We can't just monitor the process itself because each of its
        // Threads might be running on a separate cpu
        // Note: I have not parsed the cpu information (how many cpus there are, their running time (calculated as curUpTime - prevUpTime), etc...)
        //          Or which cpu each thread is running on, we will need both
        
        // Updates the uptimes of the existingProcesses
        
        
        /*
        NOTE: We need to load CPU stats before the process, in order to calculate the process usage in the past second, not the one before. 
        */
        
        updateCPUData(existingCPU);
        updateProcessData(existingProcesses, existingCPU);
        
        sleep(1);
    }
    
    std::vector<int> cpu_load = getProcessCPULoad(existingProcesses, existingCPU, "5");
    for (int i = 0; i < cpu_load.size(); i++) {
        cout << "cpu" << i << ": " << cpu_load[i] << "%" << endl;
    }
    
    return 0;
}