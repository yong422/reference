#ifndef GSTD_CHECK_NUMERIC_H
#define GSTD_CHECK_NUMERIC_H

#include <string>
#include <cctype>     // isdigit
#include <algorithm>  // find_if

namespace gstd {
namespace check {

bool IsNumeric(const std::string& value)
{
  return !value.empty() && 
    std::find_if(value.begin(), value.end(), 
                  [](unsigned char c) { return !std::isdigit(c); }
                ) == value.end();
}

} // namespace check
} // namespace gstd
#endif // GSTD_CHECK_NUMERIC_H
