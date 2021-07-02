#include "processor.h"

#include <unistd.h>

#include <thread>
#include <vector>

#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
// cpu  706064 1736 223081 85385909 3884 0 46436 0 0 0
// cpu  706064 1736 223083 85387059 3884 0 46436 0 0 0
float Processor::Utilization() {
  unsigned long active_start = LinuxParser::ActiveJiffies();
  unsigned long total_start = LinuxParser::Jiffies();
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  unsigned long active_end = LinuxParser::ActiveJiffies();
  unsigned long total_end = LinuxParser::Jiffies();

  float usage =
      (float)(active_end - active_start) / (float)(total_end - total_start);
  usage = (usage >= 1.0) ? 0.0 : usage;

  return usage;
}