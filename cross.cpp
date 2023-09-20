#include <iostream>
#include <thread>
#include <chrono>
#include <fstream> // Add this include for ifstream
#include <string>  // Add this include for string

#if defined(__linux__)
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#endif

using namespace std;

#if defined(__linux__)
rlim_t getProcessMemoryUsageLinux(pid_t pid) // Change the function name to match the declaration
{
    string filename = "/proc/" + to_string(pid) + "/status";
    ifstream statusFile(filename.c_str());
    string line;
    rlim_t memoryUsage = 0;

    if (statusFile.is_open())
    {
        while (getline(statusFile, line))
        {
            if (line.compare(0, 7, "VmRSS:") == 0)
            {
                memoryUsage = stol(line.substr(8)) * 1024;
                break;
            }
        }
        statusFile.close();
    }
    return memoryUsage;
}

void monitorMemory(pid_t pid, rlim_t maxMemoryBytes)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        rlim_t memoryUsage = getProcessMemoryUsageLinux(pid);

        if (memoryUsage > maxMemoryBytes)
        {
            cout << "Memory limit exceeded. Terminating process." << endl;
            kill(pid, SIGKILL);
            break;
        }
        else
        {
            cout << "Memory usage is within limits." << endl;
        }
    }
}
#endif

int main()
{
#if defined(__linux__)
    pid_t processId;
    rlim_t maxWorkingSetSize;

    cout << "Enter the Process ID to monitor: ";
    cin >> processId;

    cout << "Enter the maximum working set size (in bytes): ";
    cin >> maxWorkingSetSize;

#elif defined(_WIN32)
    // Windows code here
#endif

#if defined(__linux__) || defined(_WIN32)
    // The rest of the code
#else
    cerr << "Unsupported platform." << endl;
#endif

    return 0;
}
