#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "format.h"

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
  filestream.close();

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
  stream.close();

  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  fs::path proc_path(kProcDirectory);

  if (!fs::is_directory(proc_path)) return {};

  for (const auto& file : fs::directory_iterator(proc_path)) {
    std::string name = file.path().filename();
    if (file.is_directory() && IsNumber(name)) {
      pids.push_back(std::stoi(name, nullptr, 0));
    }
  }

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

  for (std::string line; std::getline(stream, line);) {
    std::istringstream lineStream(line);
    lineStream >> label >> value >> units;
    if (label.find("MemTotal") != std::string::npos) {
      totalMem = value;
    }
    if (label.find("MemFree") != std::string::npos) {
      freeMem = value;
    }
  }
  stream.close();

  return 1.0f - (freeMem / totalMem);
}

// 5011.80 15473.20
// DONE: Read and return the system uptime
ulong LinuxParser::UpTime() {
  long upTime, idleTime;

  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (!stream.is_open()) return 0.0;

  if (std::string line; std::getline(stream, line)) {
    std::istringstream lineStream(line);
    lineStream >> upTime >> idleTime;
  }
  stream.close();

  return upTime;
}

// cpu  27170 66 10006 536208 1047 0 2312 0 0 0
// DONE: Read and return the number of jiffies for the system
unsigned long LinuxParser::Jiffies() {
  vector<unsigned long> jiffies = CpuUtilization();
  unsigned long total_jiffies = 0;

  for (unsigned long jiffie : jiffies) {
    total_jiffies += jiffie;
  }

  return total_jiffies;
}

// DONE: Read and return the number of active jiffies for a PID
// 2058 (terminator) S 1694 1415 1415 0 -1 4194304 20252 103 33 0 214 175 0 0
// 20 0 5 0 3150 868159488 16762 18446744073709551615 4337664 7035557
// 140729268194400 0 0 0 0 16781312 65538 0 0 0 17 0 0 0 16 0 0 9387440 9677624
// 35516416 140729268197852 140729268197889 140729268197889 140729268199396 0
// 14 15 16 17
unsigned long LinuxParser::ActiveJiffies(int pid) {
  unsigned long total_jiffies = 0;
  vector<string> stat = LinuxParser::ProcessStat(pid);
  ulong utime = stoul(stat[LinuxParser::utime_]);
  ulong stime = stoul(stat[LinuxParser::stime_]);
  ulong cutime = stoul(stat[LinuxParser::cutime_]);
  ulong cstime = stoul(stat[LinuxParser::cstime_]);

  total_jiffies = utime + stime + cutime + cstime;

  return total_jiffies;
}

// DONE: Read and return the number of active jiffies for the system
unsigned long LinuxParser::ActiveJiffies() {
  unsigned long jiffies = Jiffies();
  unsigned long idle_jiffies = IdleJiffies();

  return jiffies - idle_jiffies;
}

// DONE: Read and return the number of idle jiffies for the system
unsigned long LinuxParser::IdleJiffies() {
  vector<unsigned long> jiffies = CpuUtilization();

  return jiffies[CPUStates::kIdle_];
}

// DONE: Read and return CPU utilization
vector<unsigned long> LinuxParser::CpuUtilization() {
  string label;
  vector<unsigned long> jiffies;
  unsigned long jiffie;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (!stream.is_open()) return {};

  for (std::string line; std::getline(stream, line);) {
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

  stream.close();
  return jiffies;
}

bool LinuxParser::IsNumber(const std::string& s) {
  return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {
                         return !std::isdigit(c);
                       }) == s.end();
}

std::vector<std::string> LinuxParser::ProcessStat(int pid) {
  vector<std::string> info;
  std::string entry;

  fs::path pid_path(kProcDirectory + "/" + std::to_string(pid));
  if (!fs::is_directory(pid_path)) return {};

  std::ifstream stream(kProcDirectory + "/" + std::to_string(pid) + "/stat");
  if (!stream.is_open()) return {};

  if (std::string line; std::getline(stream, line)) {
    std::istringstream lineStream(line);
    while (!lineStream.eof()) {
      lineStream >> entry;
      info.push_back(entry);
    }
  }
  stream.close();

  return info;
}

std::string LinuxParser::ProcessStatus(int pid, std::string param) {
  std::string key;
  std::string entry;

  fs::path pid_path(kProcDirectory + "/" + std::to_string(pid));
  if (!fs::is_directory(pid_path)) return string();

  std::ifstream stream(kProcDirectory + "/" + std::to_string(pid) + "/status");
  if (!stream.is_open()) return string();

  while (!stream.eof()) {
    if (std::string line; std::getline(stream, line)) {
      std::istringstream lineStream(line);
      lineStream >> key >> entry;
      if (key.find(param) != std::string::npos) {
        break;
      }
    }
  }
  stream.close();

  return entry;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() { return Pids().size(); }

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  vector<int> pids = Pids();
  fs::path proc_path(kProcDirectory);
  int count = 0;

  if (!fs::is_directory(proc_path)) return 0;

  for (int pid : pids) {
    vector<std::string> pid_info = ProcessStat(pid);
    std::string state = pid_info[ProcessStat::state_];
    if (state.find("R") != std::string::npos) {
      count++;
    }
  }

  return count;
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::string command;

  fs::path pid_path(kProcDirectory + "/" + std::to_string(pid));
  if (!fs::is_directory(pid_path)) return string();

  std::ifstream stream(kProcDirectory + "/" + std::to_string(pid) + "/cmdline");
  if (!stream.is_open()) return string();

  if (std::string line; std::getline(stream, line)) {
    command = line;
  }
  stream.close();

  return command;
}

// DONE: Read and return the memory used by a process
std::string LinuxParser::Ram(int pid) {
  string ram = ProcessStatus(pid, PID_VM_DATA);
  if (IsNumber(ram)) ram = std::to_string(std::stoi(ram) / 1024);

  return ram;
}

// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::string uid = ProcessStatus(pid, "Uid");
  if (uid.empty()) return string();

  std::istringstream lineStream(uid);
  lineStream >> uid;

  return uid;
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  std::string uid = Uid(pid);
  if (uid.empty()) return string();

  std::string user;

  std::ifstream stream(kPasswordPath);
  if (!stream.is_open()) return string();

  while (!stream.eof()) {
    if (std::string line; std::getline(stream, line)) {
      std::istringstream lineStream(line);
      std::string token;

      std::getline(lineStream, user, ':');
      std::getline(lineStream, token, ':');
      std::getline(lineStream, token, ':');
      if (token.compare(uid) != 0) continue;
      break;
    }
  }
  stream.close();

  return user;
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::vector<std::string> stat = ProcessStat(pid);
  ulong sec = UpTime() - std::stoul(stat[starttime_]) / sysconf(_SC_CLK_TCK);

  return sec;
}
