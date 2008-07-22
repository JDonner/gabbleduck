#include "util.h"
#include <iomanip>

using namespace std;

void set_nice_numeric_format(ostream& os)
{
   os << fixed << setprecision(3);
}
