#ifndef GSTD_CLIENT_REDIS_CLIENT_H
#define GSTD_CLIENT_REDIS_CLIENT_H
#include <string>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include "hiredis.h"

namespace gstd {
namespace client {
namespace redis {

using MultipleData      = std::unordered_map<std::string /*key*/, std::string /*value*/>; 
using HashStoreMap      = std::unordered_map<std::string, std::string>;
using HashStoreFields   = std::unordered_set<std::string>;

const uint32_t kLockTimeout = 1000;  //  1 secs

//  @brief  hiredis wrapper class
//          Redis client
//          TODO: client function return 값에 대한 정의 추가.
class Client{
  public:
  //! timeout milli second
  Client();
  Client(const char* host, int port, int dbnum=0, int timeout = 1000, int retries = 3);
  ~Client();  

  void Close();
  void FreeResult();
  void SetDebug(){is_debug_=true;}
  void set_is_debug(bool is_debug) { is_debug_ = is_debug; }
  void SetAuth(std::string pwd){password_.assign(pwd);is_auth_=true;}
  bool Connect();
  bool IsConnected() { return (redis_context_ == nullptr ? false : true); }
  std::string error_message() const { return error_message_; }
  std::string GetErrorMsg() const { return error_message_; }

  //  @brief  Get function
  //          key 에 해당하는 value 값을 가져온다.
  //  @return 0 : 값이 있음, 1 : 값이 없음(NULL), -1: 그외 에러.

  int Get(const std::string& key, std::string&   val);
  int Get(const std::string& key, long long&     val);
  int Get(const std::string& key, double&        val);
  int Get(const std::string& key, int&           val);
  int Get(const std::string& key, uint32_t&      val);
  int Get(const std::string& key, uint64_t&      val);

  //  @brief  GetMultiple function
  //          multiple_data 에 저장된 key 해당하는 value 값들을 가져온다.
  //          is_exists_fail = true 일 경우
  //  @return 0 : 값이 있음, 1 : 값이 없음(NULL), -1: 그외 에러.
  int GetMultiple(MultipleData& multiple_data, uint32_t& get_count);

  //  @brief  GetSet function
  //          key 에 해당하는 value 값을 redis 에 저장한다.
  //          저장하면서 기존에 key 에 등록된 값을 value 에 가져온다.
  //  @return 0 : 값이 있음, 1 : 값이 없음(NULL), -1: 그외 에러.
  int GetSet(const std::string& key, std::string&  val);
  int GetSet(const std::string& key, uint64_t&     val);
  int GetSet(const std::string& key, uint32_t&     val);

  //  @brief  Set function
  //          key 에 해당하는 value 값을 Redis-server에 저장.
  //  @return 0 : success, 1 : null, -1 : error
  int Set(const char* key, const char*  val);
  int Set(const std::string& key, std::string  val);
  int Set(const std::string& key, long long    val);
  int Set(const std::string& key, uint32_t     val);
  int Set(const std::string& key, int          val);
  int Set(const std::string& key, double       val);
  int Set(const std::string& key, uint64_t     val);

  //  @brief  SetMultiple function
  //          multiple_data 에 저장된 key / value list 를 저장한다.
  //          is_exists_fail = true 일 경우, 이미 존재하는 key 일 경우 전체 실패처리.(MSETNX)
  //  @return 0 : success, 1 : exists key, -1: error
  int SetMultiple(const MultipleData& hdata, bool if_exists_fail=false);

  //  @brief  Hash store mset function
  //          redis Hash store 에 key value 데이터를 저장한다.
  //  @return 0 : success, 1 : null, -1 : error
  int SetHash(const std::string& key, const HashStoreMap& hdata);

  //  @brief  Hash store mget function
  //          redis Hash store 에 fields 기준으로 value 데이터를 가져온다 한다.
  //  @return 0 : success, 1 : null, -1 : error
  int GetHash(const std::string& key, HashStoreMap& hdata, uint32_t& get_count);

  //  @brief  Hash store del function
  //          redis Hash store 에 fields value 를 삭제한다.
  //  @return 0 : success, 1 : null, -1 : error
  int DeleteHash(const std::string& key, const HashStoreFields& hdata, uint32_t& delete_count);

  //  @brief  Hash store incr function
  //          redis Hash store 에 fields 기준으로 지정한 integer 값을 갖는 field 의 값을 증가시킨다.
  //          add_value (증가시킬 값) , increased_value
  //  @return 0 : success, 1 : null, -1 : error (필드가 정수가 아닌경우 등)
  int IncreaseHashField(const std::string& key, const std::string& field, 
                        const uint32_t& add_value, int64_t& increased_value);

  //  @brief  Hash store decr function
  //          redis Hash store 에 fields 기준으로 지정한 integer 값을 갖는 field 의 값을 감소시킨다.
  //          sub_value (감소시킬 값) , decreased_value
  int DecreaseHashField(const std::string& key, const std::string& field, 
                        const uint32_t& sub_value, int64_t& decreased_value);

  //  @brief  Incr, Decr function
  //          Incr : Key 에 해당하는 카운트값을 증가시킨다
  //          Decr : Key 에 해당하는 카운트값을 감소시킨다
  //  @params const char* key    값을 변화시킬 key string
  //          long long& val    변화된 값을 받아올 결과
  //  @return  0 : 증가 및 감소 성공, 1 : ?, -1 : 에러.
  int Increase(std::string key, long long& val) { return Increase(key.c_str(), val); }
  int Increase(const char* key, long long& val);
  int Decrease(std::string key, long long& val) { return Decrease(key.c_str(), val); }
  int Decrease(const char* key, long long& val);

  // @brief key 또는 key list 에 해당하는 key value 를 삭제하는 함수
  //        삭제된 key-value 의 개수를 리턴하며, 실패시 -1 을 리턴한다. 에러메시지 참조
  int Delete(const std::string&                         key);
  int Delete(const std::vector<std::string>&            keys);
  int Delete(const std::initializer_list<std::string>&  keys);

  // @brief 분산 잠금을 구현하기 위한 Lock 함수
  // @params  std::string key 접근을 제어하고자 하는 key, 
  //                          내부에서 해당 key에 대한 잠금키를 생성하는데 사용.
  //          uint32_t        잠금을 유지하는 타임아웃(ms)
  // @return  0 : Lock, 1 : lock was not acquired, -1 : error
  int32_t Lock(const std::string key, const uint32_t& timeout = kLockTimeout) noexcept;
  //int32_t LockContinue(const std::string key, const uint32_t& timeout = kLockTimeout) noexcept;
  int32_t Unlock(const std::string key) noexcept;

protected:
  int Set_(const std::string& command) noexcept; 
  inline std::string GetUniqueId_(const std::string& key) noexcept;
  inline std::string GetLockInstanceKey_(const std::string& key) noexcept;

private:
  // private variable
  std::string   host_           = "127.0.0.1";
  std::string   error_message_  = "";
  std::string   password_       = "";
  int           db_index_       = 0;
  int           port_number_    = 6379;
  int           timeout_        = 1000000;  //* default 1 sec (dataunit = micro second)
  int           retries_        = 3;
  bool          is_debug_       = false;
  bool          is_auth_        = false;
  redisContext* redis_context_  = nullptr;
  redisReply*   redis_reply_    = nullptr;

  //  variable for redlock
  //  현재 redis connection 에서 사용중인 잠금에 대한 캐시(lock key, value) 정보
  //  lock key -> (unique value, )
  std::unordered_map< std::string, std::pair<std::string, bool> >  lock_instance_cache_;
};
} // namespace redis
} // namespace client
} // namespace gstd

#endif // GSTD_CLIENT_REDIS_CLIENT_H
