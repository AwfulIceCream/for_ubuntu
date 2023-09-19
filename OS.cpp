#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>

using namespace std;

void MonitorProcessMemoryUsage(pid_t processId, rlim_t maxWorkingSetSize)
{
    while (true)
    {
        usleep(5000000); // Sleep for 5 seconds (in microseconds)

        struct rusage usage;
        getrusage(RUSAGE_CHILDREN, &usage);
        rlim_t workingSetSize = usage.ru_maxrss * 1024; // Convert to bytes

        if (workingSetSize > maxWorkingSetSize)
        {
            cout << "Memory limit exceeded. Terminating process." << endl;
            kill(processId, SIGKILL); // Terminate the process
            break;
        }
        else
        {
            cout << "Memory usage is within limits." << endl;
        }
    }
}

int main()
{
    pid_t processId;
    rlim_t maxWorkingSetSize;

    cout << "Enter the Process ID to monitor: ";
    cin >> processId;

    cout << "Enter the maximum working set size (in bytes, 0 for unlimited): ";
    cin >> maxWorkingSetSize;

    if (maxWorkingSetSize > 0)
    {
        struct rlimit limit;
        limit.rlim_cur = maxWorkingSetSize;
        limit.rlim_max = maxWorkingSetSize;
        
        if (setrlimit(RLIMIT_AS, &limit) != 0)
        {
            perror("Failed to set memory limit");
            return 1;
        }
    }

    MonitorProcessMemoryUsage(processId, maxWorkingSetSize);

    return 0;
}
