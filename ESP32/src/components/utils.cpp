/////////////////////////////////////////////////////////////////

#include <sstream>
#include <string>

/////////////////////////////////////////////////////////////////

const char *toString(float f)
{
    std::ostringstream ss;
    ss << f;
    return ss.str().c_str();
}