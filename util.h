#ifndef UTIL_H
#define UTIL_H

#include <iosfwd>
#include <fstream>

extern std::ofstream g_log;

extern void set_nice_numeric_format(std::ostream& oss);
struct LongEnoughException {};

#endif // UTIL_H
