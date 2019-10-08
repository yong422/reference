#include <string>
#ifndef GSTD_UTIL_BASE64_H
#define GSTD_UTIL_BASE64_H
std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);
std::string base64_encode_4tsdb(unsigned char const* , unsigned int len);
std::string base64_decode_4tsdb(std::string const& s);
#endif
