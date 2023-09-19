#include <windows.h>
#include <iostream>
#include <string>
#include <psapi.h>

using namespace std;

void MonitorProcessMemoryUsage(DWORD processId, SIZE_T maxWorkingSetSize)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	if (hProcess == NULL)
	{
		cout << "Failed to open process (" << GetLastError() << ")." << endl;
		return;
	}

	while (true)
	{
		Sleep(5000);

		PROCESS_MEMORY_COUNTERS pmc;
		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		{
			SIZE_T workingSetSize = pmc.WorkingSetSize;

			if (workingSetSize > maxWorkingSetSize)
			{
				cout << "Memory limit exceeded. Terminating process." << endl;
				TerminateProcess(hProcess, 1);
				CloseHandle(hProcess);
				break;
			}
			else
			{
				cout << "Memory usage is within limits." << endl;
			}
		}

		Sleep(1000);
	}
}

int main()
{
	DWORD processId;
	SIZE_T maxWorkingSetSize;

	cout << "Enter the Process ID to monitor: ";
	cin >> processId;

	cout << "Enter the maximum working set size (in bytes): ";
	cin >> maxWorkingSetSize;

	MonitorProcessMemoryUsage(processId, maxWorkingSetSize);

	return 0;
}