#ifndef GSTD_CLIENT_REDIS_CLIENT_H
#define GSTD_CLIENT_REDIS_CLIENT_H
#include <string>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <functional>
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

  //  @brief  WATCH redis transaction 처리
  //          전달된 key 에 WATCH 를 실행한다.
  //          WATCH 가 실행된 상태에선 CheckAndSet 을 실행할 수 있다.
  //  @return 0: 성공, -1 : 에러.
  int Watch(const std::string& key);
  //  @brief  multiple WATCH 에 대한 처리 함수.
  //          Watch({"key1", "key2", "key3"});
  int Watch(const std::initializer_list<std::string>& keys);

  //  @brief  Watch 실행중인 key 에 대하여 CheckAndSet 방식으로 동작한다.
  //          Watch 가 실행중인 상태에서 해당 key 에 대해 다른 connection 에서 값을 변경한 경우
  //          CheckAndSet 실행시 null (1) 을 리턴한다.
  //          Watch 를 실행하지 않은 상태에서는 일반적인 Set 과 동일하게 동작한다.
  //  @return 0 : success, 1 : null, -1 : error
  int CheckAndSet(const std::string& key, const std::string& val);
  int CheckAndSet(const std::string& key, const uint64_t&    val);

  //  @brief  WATCH 실행중인 모든 key 에 대한 해제 처리
  //  @return 0: 성공, -1 : 에러.
  int UnWatch();

  //  @brief  redis MULTI 실행
  int Multi();

  //  @brief  redis EXEC 실행
  int Exec();

protected:

  //  @brief  redis set 관련 결과값의 상세 처리를 위한 callback function object
  //  @params int           내부 redis command 실행 결과의  parsing_set_replyObject 함수 리턴 값.
  //  @params redisReply*   내부 redis command 실행 결과 object pointer
  //  @return int
  using set_reply_callback = std::function<int(int, redisReply*)>;

  //  @brief  SET, MULTI, WATCH 등의 Set 관련 처리를 위한 공통 함수
  //  @params const std::string& command  실행하기 위한 커맨드 (SET key value, MULTI 등)
  //  @params const std::function<int(int, redisReply*)>& process_result  
  //                  실행결과의 처리를 위한 callback function
  //                  각 argument 는 다음과 같다.
  //                  int : redis command 의 실행 결과 (0: 성공, 1:null, -1:실패)
  //                  redisReply* : redis command 의 실행 결과가 저장된 object pointer
  //                                multi exec 등의 실행결과가 array 등으로 전달되어 별도 처리가 필요한경우
  //                                callback function 에 처리하여 전달.
  //                                                    
  //  @return int   callback function 에 정의된 리턴값.
  //                별도의 처리가 필요 없을 경우 callback function 의 argument 로 전달되는 int 를 그대로 리턴.
  //
  //  @example  Set_("SET key1 val1", [](int result, redisReply* r) {return result;});

  int Set_(const std::string& command, const set_reply_callback& process_result) noexcept;

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
};
} // namespace redis
} // namespace client
} // namespace gstd

#endif // GSTD_CLIENT_REDIS_CLIENT_H
