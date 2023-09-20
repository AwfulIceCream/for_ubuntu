#include <iostream>
#include <thread>
#include <chrono>

#if defined(__linux__)
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#elif defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#endif

using namespace std;

#if defined(__linux__)
rlim_t GetProcessMemoryUsageLinux(pid_t pid)
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
	DWORD processId = 12345;
	cout << "Enter the Process ID to monitor: ";
	cin >> processId;
#endif

#if defined(__linux__) || defined(_WIN32)
	SIZE_T maxWorkingSetSize = 1000000000;
	cout << "Enter the maximum working set size (in bytes): ";
	cin >> maxWorkingSetSize;
	monitorMemory(processId, maxWorkingSetSize);
#else
	cerr << "Unsupported platform." << endl;
#endif

	return 0;
}
/*In function ‘rlim_t GetProcessMemoryUsageLinux(pid_t)’:
cross.cpp:22:29: error: variable ‘std::ifstream statusFile’ has initializer but incomplete type
   22 |         ifstream statusFile(filename.c_str());
      |                             ^~~~~~~~
cross.cpp: In function ‘void monitorMemory(pid_t, rlim_t)’:
cross.cpp:46:38: error: ‘getProcessMemoryUsageLinux’ was not declared in this scope; did you mean ‘GetProcessMemoryUsageLinux’?
   46 |                 rlim_t memoryUsage = getProcessMemoryUsageLinux(pid);
      |                                      ^~~~~~~~~~~~~~~~~~~~~~~~~~
      |                                      GetProcessMemoryUsageLinux
cross.cpp: In function ‘int main()’:
cross.cpp:133:16: error: redeclaration of ‘rlim_t maxWorkingSetSize’
  133 |         rlim_t maxWorkingSetSize = 1000000000;
      |                ^~~~~~~~~~~~~~~~~
cross.cpp:118:16: note: ‘rlim_t maxWorkingSetSize’ previously declared here
  118 |         rlim_t maxWorkingSetSize;
      |                ^~~~~~~~~~~~~~~~~
*/
