#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

namespace fs = std::filesystem;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// MemTotal:        4020924 kB
// MemFree:          388936 kB
// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string label, units;
  float value;
  float totalMem, freeMem;

  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (!stream.is_open()) return 0.0;

  for (std::string line; std::getline(stream, line); ) {
    std::istringstream lineStream(line);
    lineStream >> label >> value >> units;
    if (label.find("MemTotal") != std::string::npos) {
      totalMem = value;
    }
    if (label.find("MemFree") != std::string::npos) {
      freeMem = value;
    }
  }

  return 1.0f - (freeMem / totalMem);
}

// 5011.80 15473.20
// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  long upTime, idleTime;

  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (!stream.is_open()) return 0.0;

  if (std::string line; std::getline(stream, line)) {
    std::istringstream lineStream(line);
    lineStream >> upTime >> idleTime;
  }

  return upTime;
}

// cpu  27170 66 10006 536208 1047 0 2312 0 0 0
// cpu0 3738 0 1205 66760 89 0 644 0 0 0
// TODO: Read and return the number of jiffies for the system
#define JIFFIE_COUNT 10
long LinuxParser::Jiffies() {
  string label;
  long jiffies[JIFFIE_COUNT];
  long total_jiffies = 0;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (!stream.is_open()) return 0;

  for (std::string line; std::getline(stream, line); ) {
    std::istringstream lineStream(line);
    lineStream >> label;

    if (label.compare("cpu") == 0) {
       for (int i = 0; i < JIFFIE_COUNT; i++) {
         lineStream >> jiffies[i];
         total_jiffies += jiffies[i];
       }
    }
    continue;
  }

  std::cout << "Jiffies: " << total_jiffies << std::endl;
  return total_jiffies;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
// 2058 (terminator) S 1694 1415 1415 0 -1 4194304 20252 103 33 0 214 175 0 0
// 20 0 5 0 3150 868159488 16762 18446744073709551615 4337664 7035557
// 140729268194400 0 0 0 0 16781312 65538 0 0 0 17 0 0 0 16 0 0 9387440 9677624
// 35516416 140729268197852 140729268197889 140729268197889 140729268199396 0
// 14 15 16 17
#define FIRST_JIFFIE 14
long LinuxParser::ActiveJiffies(int pid) {
  fs::path pid_path(kProcDirectory + "/" + std::to_string(pid));
  if (!fs::is_directory(pid_path)) return 0;

  std::ifstream stream(kProcDirectory + "/" + std::to_string(pid) + "/stat");
  if (!stream.is_open()) return 0;

  long total_jiffies = 0;
  long jiffie = 0;
  std::string temp;
  if (std::string line; std::getline(stream, line)) {
    std::istringstream lineStream(line);
    for (int i = 0; i < FIRST_JIFFIE - 1; i++) lineStream >> temp;
    for (int j = 0; j < 4; j++) {
      lineStream >> jiffie;
      total_jiffies += jiffie;
    }
  }

  std::cout << "Active Jiffies (" << pid << "): " << total_jiffies << std::endl;
  return total_jiffies;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long jiffies = Jiffies() - IdleJiffies();
  std::cout << "Active Jiffies: " << jiffies << std::endl;
  return jiffies;
}

// TODO: Read and return the number of idle jiffies for the system
#define IDLE_JIFFIE 4
long LinuxParser::IdleJiffies() {
  string label;
  long idle_jiffies = 0;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (!stream.is_open()) return 0;

  for (std::string line; std::getline(stream, line); ) {
    std::istringstream lineStream(line);
    lineStream >> label;

    if (label.compare("cpu") == 0) {
      for (int i = 0; i < (IDLE_JIFFIE - 1); i++) lineStream >> idle_jiffies;
      lineStream >> idle_jiffies;
    }

    continue;
  }

  std::cout << "Idle Jiffies: " << idle_jiffies << std::endl;
  return idle_jiffies;
}

// DONE: Read and return CPU utilization
vector<long> LinuxParser::CpuUtilization() {
  string label;
  vector<long> jiffies;
  long jiffie;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (!stream.is_open()) return {};

  for (std::string line; std::getline(stream, line); ) {
    std::istringstream lineStream(line);
    lineStream >> label;

    if (label.compare("cpu") == 0) {
      for (int i = 0; i < CPUStates::kSize_; i++) {
        lineStream >> jiffie;
        jiffies.push_back(jiffie);
      }
    }
    continue;
  }

  return jiffies;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { return 0; }

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { return 0; }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return 0; }
