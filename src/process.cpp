#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// TODO: Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() const {
  vector<string> stat = LinuxParser::ProcessStat(pid_);

  if (stat.empty()) return 0.0;

  ulong utime = stoul(stat[LinuxParser::utime_]);
  ulong stime = stoul(stat[LinuxParser::stime_]);
  uint cutime = stoul(stat[LinuxParser::cutime_]);
  uint cstime = stoul(stat[LinuxParser::cstime_]);
  unsigned long long starttime = stoul(stat[LinuxParser::starttime_]);

  ulong uptime = LinuxParser::UpTime();
  long hertz = sysconf(_SC_CLK_TCK);

  ulong total_time = utime + stime;
  total_time += cutime + cstime;

  long seconds = uptime - (starttime / hertz);
  float usage =(float)(total_time / hertz) / (float)seconds;
  return usage;
}

// TODO: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// TODO: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// TODO: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// TODO: Overload the "less than" comparison operator for Process objects
bool Process::operator < (Process const& a) const {
  return a.CpuUtilization() < CpuUtilization();
}