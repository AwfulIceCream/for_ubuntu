#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>

#if defined(__linux__)
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#endif

using namespace std;

#if defined(__linux__)
rlim_t getProcessMemoryUsageLinux(pid_t pid)
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

#if defined(_WIN32)
SIZE_T getProcessMemoryUsageWindows(DWORD pid)
{
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (processHandle == nullptr)
    {
        cerr << "Failed to open process. Error code: " << GetLastError() << endl;
        return 0;
    }

    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(processHandle, &pmc, sizeof(pmc)))
    {
        CloseHandle(processHandle);
        return pmc.WorkingSetSize;
    }

    CloseHandle(processHandle);
    cerr << "Failed to get process memory info. Error code: " << GetLastError() << endl;
    return 0;
}

void monitorMemory(DWORD pid, SIZE_T maxMemoryBytes)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        SIZE_T memoryUsage = getProcessMemoryUsageWindows(pid);

        if (memoryUsage > maxMemoryBytes)
        {
            cout << "Memory limit exceeded. Terminating process." << endl;
            HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
            if (processHandle != nullptr)
            {
                TerminateProcess(processHandle, 1);
                CloseHandle(processHandle);
            }
            else
            {
                cerr << "Failed to terminate process. Error code: " << GetLastError() << endl;
            }
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
    DWORD processId;
    cout << "Enter the Process ID to monitor: ";
    cin >> processId;
#endif

#if defined(__linux__) || defined(_WIN32)
    SIZE_T maxMemoryBytes;
    cout << "Enter the maximum memory usage (in bytes): ";
    cin >> maxMemoryBytes;
    monitorMemory(processId, maxMemoryBytes);
#else
    cerr << "Unsupported platform." << endl;
#endif

    return 0;
}
/*cross.cpp: In function ‘int main()’:
cross.cpp:137:5: error: ‘SIZE_T’ was not declared in this scope
  137 |     SIZE_T maxMemoryBytes;
      |     ^~~~~~
cross.cpp:139:12: error: ‘maxMemoryBytes’ was not declared in this scope
  139 |     cin >> maxMemoryBytes;
      |            ^~~~~~~~~~~~~~
*/
