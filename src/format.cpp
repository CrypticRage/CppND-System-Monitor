#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  int secs = seconds % 60;
  int rem = seconds / 60;
  int mins = rem % 60;
  rem /= 60;
  int hours = rem % 60;

  char buffer[64];
  int size = sprintf(buffer, "%i:%02i:%02i", hours, mins, secs);
  return string(buffer);
}