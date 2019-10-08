#include "gstd/container/dictionary.h"


namespace gstd {
namespace container {

std::unordered_map<std::string, std::string> Dictionary::string_dictionary_;
std::unordered_map<int32_t, std::string>  Dictionary::int_dictionary_;
std::unordered_map<std::string, uint32_t> Dictionary::string_uint_dictionary_;
std::mutex Dictionary::string_mutex_;
std::mutex Dictionary::int_mutex_;
std::mutex Dictionary::string_uint_mutex_;

Dictionary::Dictionary()
{
  //
}

Dictionary::~Dictionary()
{
  //
}

void Dictionary::Clear()
{
  std::lock_guard<std::mutex> lock_string(string_mutex_);
  std::lock_guard<std::mutex> lock_int(int_mutex_);
  std::lock_guard<std::mutex> lock_strint(string_uint_mutex_);

  string_dictionary_.clear();
  int_dictionary_.clear();
  string_uint_dictionary_.clear();
}

//  @brief    프로세스 공유데이터 key(string) - value 데이터를 저장
//  @params   std::string key   key 값
//  @params   std::string value value 값
//  @return   저장시 1, 업데이트시 0, 별도의 실패처리 없음.
int32_t Dictionary::Push(std::string key, std::string value)
{
  int32_t result = 0;
  std::lock_guard<std::mutex> lock(string_mutex_);

  auto it = string_dictionary_.find(key);
  if (it != string_dictionary_.end()) {
    it->second = value;
  } else {
    string_dictionary_.insert(std::pair<std::string, std::string>(key, value));
    ++result;
  }
  return result;
}

//  @brief    프로세스 공유데이터 key(int) - value 데이터를 저장
//  @params   int key             key 값
//  @params   std::string value   value 값
//  @return   저장시 1, 업데이트시 0, 별도의 실패처리 없음.
int32_t Dictionary::Push(int32_t key, std::string value)
{
  int32_t result = 0;
  std::lock_guard<std::mutex> lock(int_mutex_);

  auto it = int_dictionary_.find(key);
  if (it != int_dictionary_.end()) {
    it->second = value;
  } else {
    int_dictionary_.insert(std::pair<int32_t, std::string>(key, value));
    ++result;
  }
  return result;
}

//  @brief    프로세스 공유데이터 key(string) - value(unsigned int) 데이터를 저장
//  @params   std::string   key      key 값
//  @params   uint32_t      value    value 값
//  @return   저장시 1, 업데이트시 0, 별도의 실패처리 없음.
int32_t Dictionary::Push(std::string key, uint32_t value)
{
  int32_t result = 0;
  std::lock_guard<std::mutex> lock(string_uint_mutex_);

  auto it = string_uint_dictionary_.find(key);
  if (it != string_uint_dictionary_.end()) {
    it->second = value;
  } else {
    string_uint_dictionary_.insert(std::pair<std::string, uint32_t>(key, value));
    ++result;
  }
  return result;
}

//  @brief    xms 공통데이터 key(string) 에 해당하는 value 값을 찾아 결과로 넘긴다.
//  @params   std::string   key    검색할 key
//  @params   std::string&  value  결과를 넘겨받을 string class
//  @return   있을경우  1, 없을경우  0
int32_t Dictionary::Get(std::string key, std::string& value)
{
  int32_t result = 0;
  std::lock_guard<std::mutex> lock(string_mutex_);
  auto it = string_dictionary_.find(key);
  if (it != string_dictionary_.end()) {
    value = it->second;
    ++result;
  }
  return result;
}

//  @brief    xms 공통데이터 key(int) 에 해당하는 value 값을 찾아 결과로 넘긴다.
//  @params   int           key    검색할 key
//  @params   std::string&  value  결과를 넘겨받을 string class
//  @return   있을경우  1, 없을경우  0
int32_t Dictionary::Get(int32_t key, std::string& value)
{
  int32_t result = 0;
  std::lock_guard<std::mutex> lock(int_mutex_);
  auto it = int_dictionary_.find(key);
  if (it != int_dictionary_.end()) {
    value = it->second;
    ++result;
  }
  return result;
}

//  @brief    xms 공통데이터 key(string) 에 해당하는 value(unsigned) 값을 찾아 결과로 넘긴다.
//  @params   std::string   key      검색할 key string
//  @params   uint32_t&     value    결과를 넘겨받을 unsigned int
//  @return   있을경우  1, 없을경우  0
int32_t Dictionary::Get(std::string key, uint32_t& value)
{
  int32_t result = 0;
  std::lock_guard<std::mutex> lock(string_uint_mutex_);
  auto it = string_uint_dictionary_.find(key);
  if (it != string_uint_dictionary_.end()) {
    value = it->second;
    ++result;
  }
  return result;
}

//  @brief    xms 공통데이터 key(string) 에 해당하는 value(unsigned) 값을 찾아 삭제.
//  @params   std::string   key      검색할 key string
//  @return   삭제된 경우 1, 미삭제시 0
int32_t Dictionary::Delete(std::string key, uint32_t& value)
{
  int32_t result = 0;
  std::lock_guard<std::mutex> lock(string_uint_mutex_);
  auto it = string_uint_dictionary_.find(key);
  if (it != string_uint_dictionary_.end()) {
    string_uint_dictionary_.erase(it);
    ++result;
  }
  return result;
}

//  @brief    xms 공통데이터 key(string) 에 해당하는 value(string) 값을 찾아 삭제.
//  @params   std::string   key      검색할 key string
//  @return   삭제된 경우 1, 미삭제시 0
int32_t Dictionary::Delete(std::string key, std::string& value)
{
  int32_t result = 0;
  std::lock_guard<std::mutex> lock(string_uint_mutex_);
  auto it = string_dictionary_.find(key);
  if (it != string_dictionary_.end()) {
    string_dictionary_.erase(it);
    ++result;
  }
  return result;
}

//  @brief    xms 공통데이터 key(int32_t) 에 해당하는 value(string) 값을 찾아 삭제.
//  @params   int32_t key   검색할 key 정수값
//  @return   삭제된 경우 1, 미삭제시 0
int32_t Dictionary::Delete(int32_t key, std::string& value)
{
  int32_t result = 0;
  std::lock_guard<std::mutex> lock(string_uint_mutex_);
  auto it = int_dictionary_.find(key);
  if (it != int_dictionary_.end()) {
    int_dictionary_.erase(it);
    ++result;
  }
  return result;
}

} // namespace container
} // namespace gstd
