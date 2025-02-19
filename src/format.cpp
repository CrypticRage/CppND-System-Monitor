#include "format.h"

#include <string>

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
  int days = rem / 24;

  char buffer[64];
  if (days == 0)
    sprintf(buffer, "%i:%02i:%02i", hours, mins, secs);
  else
    sprintf(buffer, "%i days %i:%02i:%02i", days, hours, mins, secs);
  return string(buffer);
}