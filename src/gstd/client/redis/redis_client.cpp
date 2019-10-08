#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <unistd.h>
#include "gstd/util/strtool.h"
#include "gstd/check/numeric.h"
#include "gstd/client/redis/redis_client.h"

namespace gstd{
namespace client{
namespace redis{

Client::Client()
{

}

Client::Client(const char* host, int port, int dbnum, int timeout, int retries)
:host_(host), port_number_(port), db_index_(dbnum), timeout_(timeout*1000), retries_(retries)
{
  
}

Client::~Client()
{
  Close();
}

//  redis를 위해 생성된 동적메모리를 전부 초기화한다.
void Client::Close()
{
  if(redis_reply_){
    freeReplyObject(redis_reply_);
    redis_reply_ = nullptr;
  }

  if(redis_context_){
    redisFree(redis_context_);
    redis_context_ = nullptr;
    __CPRINTF_SUCCESS(is_debug_, "redis::Client::Close()");
  }
}


//  initialize redis result buffer
void Client::FreeResult()
{
  if(redis_reply_){
    freeReplyObject(redis_reply_);
    redis_reply_ = nullptr;
  }
}

//  redis reply check function
//  type = 0
inline int parsing_get_replyObject(redisReply* re, std::string& msg)
{
  int ret=-1;
  if (!re) {
    msg.assign("Get error: can't allocate redis replyObject");
  } else if (re->type == REDIS_REPLY_NIL) {
    msg.assign("null");
    ret=1;
   }else if (re->type == REDIS_REPLY_ERROR) {
    msg.assign("Error : ");
    msg += std::string(re->str);
  } else if (re->type == REDIS_REPLY_STATUS) {
    msg.assign("Status : ");
    msg += std::string(re->str);
    ret=1;
  } else {
    ret=0;
  }
  return ret;
}


//  redis command 의 결과값인 redisReply 결과를 체크한다.
//  결과에맞는 에러메시지를 버퍼에 추가하며 결과값을 리턴한다.
inline int parsing_set_replyObject(redisReply* re, std::string& msg)
{
  int ret=-1;
  if (!re) {
    msg.assign("Set error: can't allocate redis replyObject");
  } else if (re->type == REDIS_REPLY_NIL) {
    msg.assign("null");
  } else if (re->type == REDIS_REPLY_ERROR) {
    msg.assign("Error : ");
    msg += std::string(re->str);
  } else if (re->type == REDIS_REPLY_STATUS) {
    if (!strcmp(re->str, "OK")) {
      ret=0;
    } else {
      msg.assign("Status : ");
      msg += std::string(re->str);
    }
  } else {
    msg.assign("Unknown type : ");
    msg += std::string(std::to_string(re->type));
  }
  return ret;
}

//  redis hash store 에 key 에 해당하는 field 와 value 를 삭제한다.
//  삭제할 field 명을 HashStoreFields 에 추가하여 전달한다.
int Client::DeleteHash(const std::string& key, const HashStoreFields& hdata, uint32_t& delete_count)
{
  int result = 0;
  delete_count = 0;
  std::string hdel_string = util::StringTool::StringPrintf("HDEL %s", key.c_str());
  for (auto it = hdata.begin(); it != hdata.end(); it++) {
    if (!it->empty()) {
      hdel_string += " " + *it;
    }
  }
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, hdel_string.c_str()));
  if (!(result = parsing_get_replyObject(redis_reply_, error_message_))) {
    if (REDIS_REPLY_INTEGER == redis_reply_->type) {
      delete_count = redis_reply_->integer;
    } else {
      error_message_ = "redis error type > " + std::to_string(redis_reply_->type);
      result = -1;
    }
  }
  FreeResult();
  return result;
}

//  redis hash store 의 key 에 field value 들을 추가한다.
//  value 값이 비어있을 경우 저장하지 않는다.
int Client::SetHash(const std::string& key, const HashStoreMap& hdata)
{
  int result = 0;
  std::string hset_string = util::StringTool::StringPrintf("HMSET %s", key.c_str());
  for (auto it = hdata.begin(); it != hdata.end(); it++) {
    if (!it->first.empty() && !it->second.empty()) {
      hset_string += " " + it->first + " " + it->second;
    }
  }
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, hset_string.c_str()));
  result = parsing_set_replyObject(redis_reply_, error_message_);
  FreeResult();
  return result;
}

//  redis hash store 의 key 의 field 들의 값을 가져온다.
//  여러개의 field 값을 가져오며 field 값이 비어있는 경우 hmget 에서 제외한다.
//  가져온 field 의 string 값이 redis null string 결과 인 경우 없는 결과로 처리한다.
//  가져온 field value 개수는 get_count 변수에 저장하여 리턴한다.
int Client::GetHash(const std::string& key, HashStoreMap& hdata, uint32_t& get_count)
{
  int result = 0;
  get_count = 0;
  std::string hget_string = util::StringTool::StringPrintf("HMGET %s", key.c_str());
  for (auto it = hdata.begin(); it != hdata.end(); it++) {
    if (!it->first.empty()) hget_string += " " + it->first;
  }
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, hget_string.c_str()));
  if (!(result = parsing_get_replyObject(redis_reply_, error_message_))) {
    int hget_index = 0;
    for (auto it = hdata.begin(); it != hdata.end() && redis_reply_->element; it++, hget_index++) {
      if (!it->first.empty()) {
        if (redis_reply_->element[hget_index]->type == REDIS_REPLY_INTEGER) {
          it->second = std::to_string(redis_reply_->element[hget_index]->integer);
          ++get_count;
        } else if (redis_reply_->element[hget_index]->type == REDIS_REPLY_STRING) {
          it->second = redis_reply_->element[hget_index]->str;
          ++get_count;
        } else if (redis_reply_->element[hget_index]->type == REDIS_REPLY_NIL) {
          it->second = "";
        } 
      }
    }
    // redis hmget 결과의 경우 field 의 값이 없어도 redisCommand 실행 결과에서 각 field 에 해당하는 element 에
    // nil(결과없음) 이 넘어오므로 실제 가져온 결과는 없음 처리
    // 함수 리턴은 값이 없으므로 1 리턴
    if (!get_count) result = 1;  
  }
  FreeResult();
  return result;
}

//  redis hash store 에 key 에 해당하는 field 의 값을 add_value 값 만큼 증가 시킨다 
//  변경된 값에 대해서는 increased_value 를 리턴한다.
//  변경하고자 하는 field 의 값이 integer 가 아닐 경우 함수의 리턴은 -1 이며
//  increased_value 의 값은 0 이다.
int Client::IncreaseHashField(const std::string& key, const std::string& field, 
                              const uint32_t& add_value, int64_t& increased_value)
{
  int result = 0;
  increased_value = 0;
  std::string hincr_string = util::StringTool::StringPrintf("HINCRBY %s %s %d", key.c_str(), 
                                                            field.c_str(), add_value);

  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, hincr_string.c_str()));
  if (!(result = parsing_get_replyObject(redis_reply_, error_message_))) {
    if (REDIS_REPLY_INTEGER == redis_reply_->type) {
      increased_value = redis_reply_->integer;
    } else {
      error_message_ = "redis error type > " + std::to_string(redis_reply_->type);
      result = -1;
    }
  }
  FreeResult();
  return result;
}

//  redis hash store 에 key 에 해당하는 field 의 값을 add_value 값 만큼 감소 시킨다
//  변경된 값에 대해서는 decreased_value 를 리턴한다.
//  변경하고자 하는 field 의 값이 integer 가 아닐 경우 함수의 리턴은 -1 이며
//  increased_value 의 값은 0 이다.
int Client::DecreaseHashField(const std::string& key, const std::string& field, 
                              const uint32_t& sub_value, int64_t& decreased_value)
{
  int64_t local_sub_value = static_cast<int64_t>(sub_value) * -1;
  int result = 0;
  decreased_value = 0;
  std::string hincr_string = util::StringTool::StringPrintf("HINCRBY %s %s %d", key.c_str(), 
                                                            field.c_str(), local_sub_value);

  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, hincr_string.c_str()));
  if (!(result = parsing_get_replyObject(redis_reply_, error_message_))) {
    if (REDIS_REPLY_INTEGER == redis_reply_->type) {
      decreased_value = redis_reply_->integer;
    } else {
      error_message_ = "redis error type > " + std::to_string(redis_reply_->type);
      result = -1;
    }
  }
  FreeResult();
  return result;
}

//  key에 해당하는 카운터값을 증가시킨다.
//  증가된 결과는 val 에 저장하여 리턴한다.

//  @return  int  성공시 0, 에러발생시 -1 또는 1 을 리턴한다.
//          주로 key의 값이 string 인 경우 에러발생.

int Client::Increase(const char* key, long long& val)
{
  int ret=0;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "INCR %s", key));
  if (!(ret = parsing_get_replyObject(redis_reply_, error_message_))) {
    if (REDIS_REPLY_INTEGER == redis_reply_->type) {
      val = redis_reply_->integer;
    }  
  }
  FreeResult();
  return ret;  
}


// key에 해당하는 카운터값을 감소시킨다.
// 감소된 결과는 val에 저장하여 리턴한다.

// @return int  성공시 0, 에러발생시 -1 또는 1 을 리턴한다.
//         주로 key의 값이 string 인 경우 에러발생.
int Client::Decrease(const char* key, long long& val)
{
  int ret=0;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "DECR %s", key));
  if (!(ret = parsing_get_replyObject(redis_reply_, error_message_))) {
    if (REDIS_REPLY_INTEGER == redis_reply_->type) {
      val = redis_reply_->integer;
    }  
  }
  FreeResult();
  return ret;  
}

int Client::Get(std::string key, std::string& val)
{
  int ret=0;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "GET %s", key.c_str()));
  if (!(ret = parsing_get_replyObject(redis_reply_, error_message_))) {
    if(REDIS_REPLY_STRING == redis_reply_->type){
      val.assign(redis_reply_->str);
    }  
  }
  FreeResult();
  return ret;
}

int Client::Get(std::string key, long long& val)
{
  int ret=0;
  std::string buf;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "GET %s", key.c_str()));
  if (!(ret = parsing_get_replyObject(redis_reply_, error_message_))) {
    if (REDIS_REPLY_STRING == redis_reply_->type) {
      buf.assign(redis_reply_->str);
      __CPRINTF_SUCCESS(is_debug_, "Client::Get(long long)");
      __CPRINTF_SUCCESS(is_debug_, buf.c_str());
      if (std::count_if(buf.begin(), buf.end(), [](unsigned char ch) {
        return (std::isdigit(ch) == 0) ? true : false;})){
        error_message_ = std::string("Error : non numeric => ") + buf;
        ret = -1;
      } else {
        val = strtoll(buf.c_str(), nullptr, 10);
      }
    } else if (REDIS_REPLY_INTEGER == redis_reply_->type) {
      val = redis_reply_->integer;
    }
  }
  FreeResult();  
  return ret;
}

int Client::Get(std::string key, double& val)
{
  int ret=0;
  std::string buf;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "GET %s", key.c_str()));
  if (!(ret = parsing_get_replyObject(redis_reply_, error_message_))) {
    if (REDIS_REPLY_STRING == redis_reply_->type) {
      buf.assign(redis_reply_->str);
      std::string chkBuf = buf;
      //! 실수형 체크. 소수점을 제외한 정수로 만들어 numeric 체크.
      util::StringTool::Replace(chkBuf, ".", "");
      __CPRINTF_SUCCESS(is_debug_, "Client::Get(double)");
      __CPRINTF_SUCCESS(is_debug_, buf.c_str());
      if (std::count_if(chkBuf.begin(), chkBuf.end(), [](unsigned char ch) {
        return (std::isdigit(ch) == 0) ? true : false;})){
        error_message_ = std::string("Error : non numeric => ") + buf;
        ret = -1;
      } else {
        val = strtod(buf.c_str(), nullptr);
      }
    }
  }
  FreeResult();  
  return ret;
}

int Client::Get(std::string key, int& val)
{
  int ret=0;
  std::string buf;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "GET %s", key.c_str()));
  if (!(ret = parsing_get_replyObject(redis_reply_, error_message_))) {
    if (REDIS_REPLY_STRING == redis_reply_->type) {
      buf.assign(redis_reply_->str);
      __CPRINTF_SUCCESS(is_debug_, "Client::Get(int)");
      __CPRINTF_SUCCESS(is_debug_, buf.c_str());
      if (std::count_if(buf.begin(), buf.end(), [](unsigned char ch) {
        return (std::isdigit(ch) == 0) ? true : false;})){
        error_message_ = std::string("Error : non numeric => ") + buf;
        ret = -1;
      } else {
      val = static_cast<int>(strtol(buf.c_str(), nullptr, 10));
      }
    } else if (REDIS_REPLY_INTEGER == redis_reply_->type) {
      val = redis_reply_->integer;
    }
  }
  FreeResult();  
  return ret;
}

int Client::Get(std::string key, unsigned int& val)
{
  int ret=0;
  std::string buf;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "GET %s", key.c_str()));
  if (!(ret = parsing_get_replyObject(redis_reply_, error_message_))) {
    if (REDIS_REPLY_STRING == redis_reply_->type) {
      buf.assign(redis_reply_->str);
      __CPRINTF_SUCCESS(is_debug_, "Client::Get(unsigned int)");
      __CPRINTF_SUCCESS(is_debug_, buf.c_str());
      if (std::count_if(buf.begin(), buf.end(), [](unsigned char ch) {
        return (std::isdigit(ch) == 0) ? true : false;})){
        error_message_ = std::string("Error : non numeric => ") + buf;
        ret = -1;
      } else {
        val = static_cast<unsigned int>(strtoul(buf.c_str(), nullptr, 10));
      }
    } else if (REDIS_REPLY_INTEGER == redis_reply_->type) {
      val = redis_reply_->integer;
    }
  }
  FreeResult();  
  return ret;
}


// 이하 Set 동일.
// key 에 해당하는 val argument 를 Redis 에 저장한다.
// parsing_set_replyObject 함수로 결과를 체크 한 후 리턴한다.
// 에러가 발생시 에러버퍼에 저장되어있다.
int Client::Set(std::string key, std::string val)
{
  return Set(key.c_str(), val.c_str());
}

int Client::Set(const char* key, const char* val)
{
  int ret = -1;
  if(key != nullptr && val != nullptr){
    redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "SET %s %s", key, val));
    //! redis set reply 결과값을 parsing
    ret = parsing_set_replyObject(redis_reply_, error_message_);
  }
  FreeResult();
  return ret;
}

int Client::Set(std::string key, long long val)
{
  int ret=0;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "SET %s %lld", key.c_str(), val));
  //! redis set reply 결과값을 parsing
  ret = parsing_set_replyObject(redis_reply_, error_message_);
  FreeResult();
  return ret;
}

int Client::Set(std::string key, double val)
{
  int ret=0;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "SET %s %.4f", key.c_str(), val));
  ret = parsing_set_replyObject(redis_reply_, error_message_);
  FreeResult();
  return ret;
}

int Client::Set(std::string key, int val)
{
  int ret=0;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "SET %s %d", key.c_str(), val));
  ret = parsing_set_replyObject(redis_reply_, error_message_);
  FreeResult();
  return ret;
}

int Client::Set(std::string key, unsigned int val)
{
  int ret=0;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "SET %s %u", key.c_str(), val));
  ret = parsing_set_replyObject(redis_reply_, error_message_);
  FreeResult();
  return ret;
}

int Client::Delete(std::string key)
{
  int result = -1;
  std::string keys_string = "DEL " + key;
  redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, keys_string.c_str()));
  if (!(result = parsing_get_replyObject(redis_reply_, error_message_))) {
    if (REDIS_REPLY_INTEGER == redis_reply_->type) {
      result = redis_reply_->integer;
    } else {
      error_message_ = "redis error type > " + std::to_string(redis_reply_->type);
      result = -1;
    }
  }
  FreeResult();
  return result;
}

int Client::Delete(std::vector<std::string> keys)
{
  std::string keys_string = "";
  int count = 0;
  for (auto const& value: keys) {
    if (count++) keys_string += " ";
    keys_string += value;
  }
  if (keys_string.empty()) return 0;
  return Delete(keys_string);
}


//  @brief  redis-server 로 접속한다.
//          설정된 접속정보를 사용.
bool Client::Connect()
{
  bool auth = false;
  bool ret = false;
  this->Close();
  int lsec = 0, usec = 0, lretries = 0;
  if( timeout_ >= 1000000) {
    lsec = timeout_ / 1000000;
    usec = timeout_ % 1000000;
  } else {
    usec = timeout_;
  }
  struct timeval ltimeout = { lsec, usec };
  while (++lretries <= retries_) {
    redis_context_ = redisConnectWithTimeout(host_.c_str(), port_number_, ltimeout);
    if (redis_context_ == nullptr){
      error_message_.assign("Connection error: can't allocate redis context");
    } else if (redis_context_->err) {
      error_message_.assign("Connection error: ");
      error_message_ += std::string(redis_context_->errstr) + std::string(" retries : ");
      error_message_ += std::to_string(lretries);
      __CPRINTF_ERROR(is_debug_, error_message_.c_str());
      Close();
      sleep(1);
    } else {
      //! 인증정보가 설정된 경우 인증절차 진행.
      if (is_auth_) {
        redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "AUTH %s", password_.c_str()));
        if (redis_reply_) {
          if (redis_reply_->type == REDIS_REPLY_ERROR) {
            error_message_.assign("Auth error: ");
            error_message_ += std::string(redis_reply_->str);
            __CPRINTF_ERROR(is_debug_, error_message_.c_str());
          } else if (redis_reply_->type == REDIS_REPLY_STATUS) {
            if (!strcmp(redis_reply_->str, "OK")) {
              auth = true;
              __CPRINTF_SUCCESS(is_debug_, "Redis Auth");
            } else {
              error_message_.assign("Auth Status : ");
              error_message_ += std::string(redis_reply_->str);
              __CPRINTF_ERROR(is_debug_, error_message_.c_str());
            }
          }
          FreeResult();
        } else {
          error_message_.assign("DB select error: can't allocate redis replyObject");
        }
      } else {
        auth = true;
      }
      if (auth) {
        if (db_index_ > 0) {
          redis_reply_ = static_cast<redisReply*>(redisCommand(redis_context_, "SELECT %d", db_index_));
          if (redis_reply_) {
          //! db index 선택 오류는 재접속 시도 하지않음.
            if (redis_reply_->type == REDIS_REPLY_ERROR) {
              error_message_.assign("DB select error:");
              error_message_ += std::string(redis_reply_->str) + std::string("dbnum : ") 
                + std::to_string(db_index_);
              __CPRINTF_ERROR(is_debug_, error_message_.c_str());
            } else {
              ret = true;
            }
            FreeResult();
          } else {
            error_message_.assign("DB select error: can't allocate redis replyObject");
          }
        } else {
          ret = true;
        }
      }
      break;
    }
  }
  return  ret;
}

} // namespace redis
} // namespace client
} // namespace gstd

// 기존의 main 함수로 작성한 테스트코드는 삭제한다.
// 테스트코드는 gtest 코드로 변경하며 test/redis_client/test_redis_client.cpp 에 추가
