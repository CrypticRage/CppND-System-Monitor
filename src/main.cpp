#include "ncurses_display.h"
#include "system.h"

#include "format.h"
#include "linux_parser.h"

int main() {
  System system;

  float util = LinuxParser::MemoryUtilization();
  std::string upTime = Format::ElapsedTime(LinuxParser::UpTime());

  NCursesDisplay::Display(system);
}