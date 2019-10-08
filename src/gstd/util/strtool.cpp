#if defined(_WIN32)
#include "stdafx.h"
#endif
#include "gstd/util/strtool.h"

namespace gstd {
namespace util {

int StringTool::Split(std::string str, const char delimiter, std::list<std::string>* plist)
{
  int result = 0;
  if (plist) {
    plist->clear();
    std::stringstream ss(str); // Turn the string into a stream.
    std::string tok;
    while (getline(ss, tok, delimiter)) {
      if (tok.length() > 0) {
        plist->push_back(tok);
        ++result;
      }
    }
  }
  return result;
}

int StringTool::Split(std::string str, const char delimiter, std::vector<std::string>* pvec)
{
  int result = 0;
  if (pvec) {
    pvec->clear();
    std::stringstream ss(str); // Turn the string into a stream.
    std::string tok;
    while (getline(ss, tok, delimiter)) {
      if (tok.length() > 0) {
        pvec->push_back(tok);
        ++result;
      }
    }
  }
  return result;
}



int StringTool::Replace(char* result_value, int result_size, const char* change_value, const char* changed_value)
{
  int result = -1;
  if (result_value) {
    std::string rstr(result_value);
    while (true) {
      size_t findResource=0;
      size_t sizeResource=0;
      sizeResource = strlen(change_value);
      findResource = rstr.find(change_value);
      if (findResource != std::string::npos) {
        std::string buffer;
        buffer.assign(rstr, findResource+sizeResource, rstr.size());
        rstr.erase(findResource+sizeResource, rstr.size());
        rstr.replace(findResource, findResource+sizeResource, changed_value);
        rstr += buffer;
      } else {
        break;
      }
    }
    memset(result_value, 0x00, result_size);
#if _MSC_VER > 1900
    strncpy_s(result_value, result_size, rstr.c_str(), result_size - 1);
#else
    strncpy(result_value, rstr.c_str(), result_size-1);
#endif
    result = strlen(result_value);
  }
  return result;
}

int StringTool::Replace(std::string& result_string, const char* change_value, const char* changed_value)
{
  int result = -1;
  if (result_string.length()) {
    std::string rstr(result_string);
    while (true) {
      size_t findResource=0;
      size_t sizeResource=0;
      sizeResource = strlen(change_value);
      findResource = rstr.find(change_value);
      if (findResource != std::string::npos) {
        std::string buffer;
        buffer.assign(rstr, findResource+sizeResource, rstr.size());
        rstr.erase(findResource+sizeResource, rstr.size());
        rstr.replace(findResource, findResource+sizeResource, changed_value);
        rstr += buffer;
      } else {
        break;
      }
    }
    result_string = rstr;
    result = result_string.length();
  }
  return result;
}

void StringTool::ReplaceUsingDictionary(std::string& result_string, const ReplaceDictionary& replace_dictionary)
{
  for (const auto& value : replace_dictionary) {
    StringTool::Replace(result_string, value.first.c_str(), value.second.c_str());
  }
}

void StringAppend(std::string& destination, const char* format, va_list ap) 
{
  const int kDefaultBufferSize = 4096;
  char append_buffer[kDefaultBufferSize] = {0, };

  va_list backup_ap;
  va_copy(backup_ap, ap);
  int result = vsnprintf(append_buffer, kDefaultBufferSize, format, backup_ap);
  va_end(backup_ap);
  if (result < kDefaultBufferSize) {
    if (result >= 0) {
      destination.append(append_buffer, result);
      return;
    }
    if (result < 0) return;
  } else {  // 필요한 버퍼사이즈가 더 큰 경우.
    int length = result + 1;
    char* expanded_buffer = new char[length];
    if (expanded_buffer) {
      va_copy(backup_ap, ap);
      result = vsnprintf(expanded_buffer, length, format, backup_ap);
      va_end(backup_ap);
      if (result >= 0 && result < length) {
        destination.append(expanded_buffer, result);
      }
      delete[] expanded_buffer;
    }
  }
}

std::string StringTool::StringPrintf(const char* format, ...) 
{
  va_list ap;
  va_start(ap, format);
  std::string result;
  StringAppend(result, format, ap);
  va_end(ap);
  return result;
}


} // namespace util
} // namespace gstd