#include "gstd/check/version.h"


namespace gstd {
namespace check {

uint32_t Version::CheckChanged(version::Type type,
                               std::string current_version,
                               std::string new_version)
{
  uint32_t result = version::kNotSupportedType;

  switch (type) {
    case version::kSemantic:
      result = CheckChangedSemantic_(current_version, new_version);
      break;
    case version::kGabiaMon:

      break;
  }
  return result;
}

uint32_t Version::CheckChangedSemantic_(std::string current_version, std::string new_version)
{
  uint32_t result = version::kSame;
  int32_t sscanf_result = 0, dot_count = 0;
  int32_t curver[_GSTD_MAXCOUNT_SEMANTIC_VERSION] = { 0, };
  int32_t newver[_GSTD_MAXCOUNT_SEMANTIC_VERSION] = { 0, };
  for (uint32_t i = 0; i < current_version.length();) {
    if (current_version[i++] == '.') ++dot_count;
  }
  if (dot_count != _GSTD_MAXCOUNT_SEMANTIC_VERSION - 1)
    return version::kFailedParsing;

  dot_count = 0;
  for (uint32_t i = 0; i < new_version.length();) {
    if (new_version[i++] == '.') ++dot_count;
  }
  if (dot_count != _GSTD_MAXCOUNT_SEMANTIC_VERSION - 1)
    return version::kFailedParsing;

#if (_MSC_VER >= 1700) || (__cplusplus > 201103L)
  if ((sscanf_result = sscanf_s(current_version.c_str(), "%d.%d.%d",
#else
  if ((sscanf_result = sscanf(current_version.c_str(), "%d.%d.%d",
#endif
    &curver[0],
    &curver[1],
    &curver[2])) != _GSTD_MAXCOUNT_SEMANTIC_VERSION) {
    return version::kFailedParsing;
  }
#if (_MSC_VER >= 1700) || (__cplusplus > 201103L)
  if ((sscanf_result = sscanf_s(new_version.c_str(), "%d.%d.%d",
#else
  if ((sscanf_result = sscanf(new_version.c_str(), "%d.%d.%d",
#endif
    &newver[0],
    &newver[1],
    &newver[2])) != _GSTD_MAXCOUNT_SEMANTIC_VERSION) {
    return version::kFailedParsing;
  }

  for (int32_t i = 0; i < _GSTD_MAXCOUNT_SEMANTIC_VERSION;) {
    if (newver[i] > curver[i]) {
      result = version::kHigher;
      break;
    } else if (newver[i] == curver[i]) {
      ++i;
    } else {
      result = version::kLower;
      break;
    }
  }
  return result;
}

} // namespace check
} // namespace gstd
