#include "ncurses_display.h"
#include "system.h"

#include "format.h"
#include "linux_parser.h"

int main() {
  System system;

  float util = LinuxParser::MemoryUtilization();
  std::string upTime = Format::ElapsedTime(LinuxParser::UpTime());
  long totalJiffs = LinuxParser::Jiffies();
  totalJiffs = LinuxParser::ActiveJiffies(2058);
  totalJiffs = LinuxParser::ActiveJiffies();
  long idleJiffs = LinuxParser::IdleJiffies();

  std::vector<long> cpu = LinuxParser::CpuUtilization();

  // NCursesDisplay::Display(system);
}