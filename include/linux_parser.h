#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <regex>
#include <string>

namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc/"};
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};

// System
float MemoryUtilization();
ulong UpTime();
std::vector<int> Pids();
int TotalProcesses();
int RunningProcesses();
std::string OperatingSystem();
std::string Kernel();

// CPU
enum CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_,
  kSize_
};

// Process
enum ProcessStat {
  pid_ = 0,
  comm_ = 1,
  state_ = 2,
  utime_ = 13,
  stime_ = 14,
  cutime_ = 15,
  cstime_ = 16,
  starttime_ = 21,
  size_
};

std::vector<unsigned long> CpuUtilization();
unsigned long Jiffies();
unsigned long ActiveJiffies();
unsigned long ActiveJiffies(int pid);
unsigned long IdleJiffies();
std::string ProcessStatus(int pid, std::string param);
std::vector<std::string> ProcessStat(int pid);
bool IsNumber(const std::string& s);

// Processes
std::string Command(int pid);
std::string Ram(int pid);
std::string Uid(int pid);
std::string User(int pid);
long int UpTime(int pid);
};  // namespace LinuxParser

#endif