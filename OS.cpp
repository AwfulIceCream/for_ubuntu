#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <signal.h>

// Function to get the memory usage of a process by its PID
long getProcessMemoryUsage(int pid) {
    std::ifstream statm;
    std::string statm_path = "/proc/" + std::to_string(pid) + "/statm";
    statm.open(statm_path.c_str());

    if (!statm) {
        std::cerr << "Error: Unable to open " << statm_path << std::endl;
        return -1;
    }

    long size;
    statm >> size;
    statm.close();
    return size;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <PID> <memory_limit_in_kb>" << std::endl;
        return 1;
    }

    int pid = std::stoi(argv[1]);
    long limit_kb = std::stol(argv[2]);

    if (pid <= 0 || limit_kb < 0) {
        std::cerr << "Invalid PID or memory limit." << std::endl;
        return 1;
    }

    while (true) {
        long memory_usage_kb = getProcessMemoryUsage(pid);

        if (memory_usage_kb == -1) {
            std::cerr << "Process with PID " << pid << " not found." << std::endl;
            return 1;
        }

        std::cout << "Process " << pid << " memory usage: " << memory_usage_kb << " KB" << std::endl;

        if (memory_usage_kb > limit_kb) {
            std::cout << "Process " << pid << " exceeded the memory limit of " << limit_kb << " KB. Terminating..." << std::endl;
            kill(pid, SIGKILL); // Send SIGKILL to terminate the process forcefully
            return 0;
        }

        usleep(1000000); // Sleep for 1 second (adjust as needed)
    }

    return 0;
}

//g++ monitor_memory.cpp -o monitor_memory
//./monitor_memory <PID> <memory_limit_in_kb>
