#ifndef GSTD_CONTAINER_DICTIONARY_H
#define GSTD_CONTAINER_DICTIONARY_H

#include <stdint.h>
#include <unordered_map>
#include <mutex>

namespace gstd {
namespace container {

//  @class  Dictionary
//  @brief  
class Dictionary {
public:
  Dictionary();
  ~Dictionary();
  //* 데이터 저장
  static int32_t  Push(std::string key, std::string value);
  static int32_t  Push(int32_t key, std::string value);
  static int32_t  Push(std::string key, uint32_t value);
  //* 데이터 가져오기
  static int32_t  Get(std::string key, std::string& value);
  static int32_t  Get(int32_t key, std::string& value);
  static int32_t  Get(std::string key, uint32_t& value);
  //* 데이터 지우기
  static int32_t  Delete(std::string key, std::string& value);
  static int32_t  Delete(int32_t key, std::string& value);
  static int32_t  Delete(std::string key, uint32_t& value);

  //* 내부 메모리 초기화
  static void     Clear();
private:
  static std::unordered_map<std::string, std::string> string_dictionary_;
  static std::unordered_map<int32_t, std::string>	int_dictionary_;
  static std::unordered_map<std::string, uint32_t>  string_uint_dictionary_;
  static std::mutex string_mutex_;
  static std::mutex int_mutex_;
  static std::mutex string_uint_mutex_;
};

} // namespace container
} // namespace gstd

#endif // GSTD_CONTAINER_DICTIONARY_H
