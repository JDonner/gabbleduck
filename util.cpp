#include "util.h"
#include <iomanip>
#include <fstream>

using namespace std;

ofstream g_log;

void set_nice_numeric_format(ostream& os)
{
   os << fixed << setprecision(3);
}
