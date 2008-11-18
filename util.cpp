#include "util.h"
#include <iomanip>
#include <fstream>
#include <sys/stat.h>

using namespace std;

ofstream g_log;

void set_nice_numeric_format(ostream& os)
{
   os << fixed << setprecision(3);
}

// Make a file readable / writable for shared user installation at NMSU
void give_wide_permissions(const char* fpath)
{
   ::chmod(fpath,
           // everyone can read and write (not execute)
           // user (creator, owner)
           S_IRUSR | S_IWUSR |
           // group
           S_IRGRP | S_IWGRP |
           // others (everyone else)
           S_IROTH | S_IWOTH);
}
