#include <memory>
#include <gtest/gtest.h>
#include "gstd/client/redis/redis_client.h"

class TestRedisClient : public ::testing::Test {
 protected:
  // 테스트에 필요한 설정을 설정
  // 각 테스트 케이스 시작전 호출
  virtual void SetUp() {
    //
    redis_client = new gstd::client::redis::Client("127.0.0.1", 6379, 0);
    ASSERT_TRUE(redis_client->Connect());
  }

  // 테스트 자원 해제
  // 각 테스트 종료시 호출출
  virtual void TearDown() {
    //
    if (redis_client) delete redis_client;
    redis_client = NULL;
  }
  gstd::client::redis::Client* redis_client;
  std::string   strRet;
  unsigned int  uRet;
  long long     llRet;
  double        fRet;
  uint64_t      ullRet;
  gstd::client::redis::HashStoreMap hstore_data;
  gstd::client::redis::HashStoreMap hstore_data_result;
};


// 로컬 redis-server 설치후 정상 접속 기능과 auth, data get set, increase decrease 등을 테스트 하기 위한
// 테스트 코드
// 빌드 및 테스트가 진행되는 로컬 서버에 redis-server 가 설치되어 실행된 상태에서 테스트 코드 진행

TEST_F(TestRedisClient, ConnectionTest)
{
  delete redis_client;
  redis_client = new gstd::client::redis::Client("127.0.0.1", 6378, 0);
  EXPECT_FALSE(redis_client->Connect());
}

TEST_F(TestRedisClient, SetTest)
{
  EXPECT_EQ(0, redis_client->Set("key1", "string_val"));
  EXPECT_EQ(0, redis_client->Set("key2", (unsigned int)time(NULL)));
  EXPECT_EQ(0, redis_client->Set("key3", (double)3.1456));
  EXPECT_EQ(0, redis_client->Set("key4", "deletetest"));
  EXPECT_EQ(0, redis_client->Set("key10", static_cast<uint64_t>(std::numeric_limits<uint64_t>::max())));
  redis_client->Close();
}

TEST_F(TestRedisClient, GetTest)
{
  EXPECT_EQ(0, redis_client->Get("key1", strRet));
  EXPECT_STREQ("string_val", strRet.c_str());
  EXPECT_EQ(0, redis_client->Get("key2", uRet));
  EXPECT_NE((unsigned int)time(NULL) - 10, uRet);
  EXPECT_EQ(0,redis_client->Get("key3", fRet));
  EXPECT_EQ(3.1456, fRet);
  // 없는 데이터 가져오는 테스트
  EXPECT_EQ(1, redis_client->Get("key5", strRet));
  // 정상 데이터 타입으로의 요청 테스트
  EXPECT_EQ(0, redis_client->Get("key10", ullRet));
  EXPECT_EQ(static_cast<uint64_t>(std::numeric_limits<uint64_t>::max()), ullRet);
  // uint64_t 값으로 저장된 데이터에 대한 uint32_t 요청. 실패테스트.
  EXPECT_EQ(-1, redis_client->Get("key10", uRet));

  // 없는 키에 대한 uint64_t 요청 실패테스트.
  EXPECT_EQ(1, redis_client->Get("key55", uRet));
}

TEST_F(TestRedisClient, DeleteTest)
{
  std::vector<std::string> delvec = {"key1", "key2", "key3"};
  //std::vector<std::string> delvec = {"key1  key2  key3"};
  EXPECT_EQ(3, redis_client->Delete(delvec));
  EXPECT_EQ(1, redis_client->Delete("key4"));
  EXPECT_EQ(0, redis_client->Delete("key5"));
}

TEST_F(TestRedisClient, HSetGetTest)
{
  uint32_t get_count = 0, delete_count = 0;
  hstore_data.insert(std::make_pair("key1", "value1"));
  hstore_data.insert(std::make_pair("key2", "value2"));
  hstore_data.insert(std::make_pair("key4", "value4"));
  hstore_data.insert(std::make_pair("key5", "50552"));
  hstore_data.insert(std::make_pair("key30", "25"));

  // HGET 가져오기 위한 key 리스트 추가
  hstore_data_result.insert(std::make_pair("key1", ""));
  hstore_data_result.insert(std::make_pair("key2", ""));
  hstore_data_result.insert(std::make_pair("key4", ""));
  hstore_data_result.insert(std::make_pair("key5", ""));

  // Hash store 에 myhashkey 에 대해 key value 저장
  EXPECT_EQ(0, redis_client->SetHash("myhashkey", hstore_data));

  // hash store 에서 가져올 myhashkey 의 key 전달하여 hmget
  EXPECT_EQ(0, redis_client->GetHash("myhashkey", hstore_data_result, get_count));
  EXPECT_EQ(4, get_count);
  // 정상 가져온 값 비교.
  EXPECT_STREQ(hstore_data["key1"].c_str(), hstore_data_result["key1"].c_str());
  EXPECT_EQ(0, hstore_data["key4"].compare(hstore_data_result["key4"]));
  EXPECT_EQ(50552, stoull(hstore_data_result["key5"]));

  // 없는 sub key 값에 대한 hmget
  gstd::client::redis::HashStoreMap empty_get_test;
  empty_get_test.insert(std::make_pair("key6", ""));
  EXPECT_EQ(1, redis_client->GetHash("myhashkey", empty_get_test, get_count));
  EXPECT_EQ(0, get_count);
  //EXPECT_EQ(1, redis_client->HashGet("emptyhashkey", hstore_data_result));

  gstd::client::redis::HashStoreFields delete_hkeys = {"key1", "key2", "key4", "key5", "key30"};
  EXPECT_EQ(0, redis_client->DeleteHash("myhashkey", delete_hkeys, delete_count));
  EXPECT_EQ(5, delete_count);
}

TEST_F(TestRedisClient, HIncrbyTest)
{
  int64_t get_result = 0;
  uint32_t get_count = 0;
  hstore_data.insert(std::make_pair("key1", "50552"));
  hstore_data.insert(std::make_pair("key2", "-30"));
  hstore_data.insert(std::make_pair("key3", "string_value"));
  EXPECT_EQ(0, redis_client->SetHash("myincrhashkey", hstore_data));

  hstore_data_result.insert(std::make_pair("key1", ""));
  hstore_data_result.insert(std::make_pair("key2", ""));

  EXPECT_EQ(0, redis_client->IncreaseHashField("myincrhashkey", "key1", 30, get_result));
  EXPECT_EQ(50582, get_result);
  EXPECT_EQ(0, redis_client->DecreaseHashField("myincrhashkey", "key2", 2, get_result));
  EXPECT_EQ(-32, get_result);
  EXPECT_EQ(0, redis_client->GetHash("myincrhashkey", hstore_data_result, get_count));
  EXPECT_EQ(2, get_count);

  EXPECT_EQ(-1, redis_client->DecreaseHashField("myincrhashkey", "key3", 2, get_result));
  EXPECT_EQ(-1, redis_client->IncreaseHashField("myincrhashkey", "key3", 5, get_result));
  // 증가 또는 감소시킨 값에 대한 비교.

  EXPECT_EQ(50582, stoll(hstore_data_result["key1"]));
  EXPECT_EQ(-32, stoll(hstore_data_result["key2"]));
  gstd::client::redis::HashStoreFields delete_hkeys = {"key1", "key2", "key2"};
  EXPECT_EQ(0, redis_client->DeleteHash("myhashkey", delete_hkeys, get_count));
}

TEST_F(TestRedisClient, GetMultipleTest)
{
  EXPECT_EQ(0, redis_client->Set("mkey1", "value1"));
  EXPECT_EQ(0, redis_client->Set("mkey2", "value2"));

  gstd::client::redis::MultipleData get_multiple_data = {{"mkey1", ""}, {"mkey2", ""}};
  gstd::client::redis::MultipleData set_exists_multiple_data = {{"mkey3", "val"}, {"mkey2", "val2"}};
  uint32_t get_count = 0;
  // multiple key 에 대한 데이터 가져오기 테스트
  EXPECT_EQ(0, redis_client->GetMultiple(get_multiple_data, get_count));
  // 정상적인 2개의 데이터 처리.
  EXPECT_EQ(2, get_count);
  // 데이터 검증.
  EXPECT_STREQ("value1", get_multiple_data["mkey1"].c_str());
  EXPECT_STREQ("value2", get_multiple_data["mkey2"].c_str());

  // MSETNX 테스트. 중복된 키에 대해 SET 을 실행시 전체 실패처리.
  EXPECT_EQ(1, redis_client->SetMultiple(set_exists_multiple_data, true /*if exsits fail*/));
  // 중복된 키 삭제후 새로운 키 등록.
  set_exists_multiple_data.erase(set_exists_multiple_data.find("mkey2"));
  set_exists_multiple_data.insert(std::make_pair("mkey4", "val4"));
  // 정상 MSETNX 처리
  EXPECT_EQ(0, redis_client->SetMultiple(set_exists_multiple_data, true /*if exsits fail*/));

  // 없는 key list 에 대한 GetMultiple 테스트.
  gstd::client::redis::MultipleData get_empty_multiple_data = {{"mkey5", ""}, {"mkey6", ""}};
  EXPECT_EQ(1, redis_client->GetMultiple(get_empty_multiple_data, get_count));
  // 가져온 데이터 없으므로 0 처리.
  EXPECT_EQ(0, get_count);
  // 테스트에 사용한 모든데이터 삭제.
  EXPECT_EQ(4, redis_client->Delete({"mkey1", "mkey2", "mkey3", "mkey4"}));
}

TEST_F(TestRedisClient, GetSetTest)
{
  EXPECT_EQ(0, redis_client->Set("key11", "val11"));

  std::string get_string = "";
  // 없는 key
  EXPECT_EQ(1, redis_client->Get("key22", get_string));
  // value 설정된 key
  EXPECT_EQ(0, redis_client->Get("key11", get_string));
  EXPECT_STREQ("val11", get_string.c_str());
  get_string = "change_val11";
  EXPECT_EQ(0, redis_client->GetSet("key11", get_string));
  EXPECT_STREQ("val11", get_string.c_str());
  EXPECT_EQ(0, redis_client->Get("key11", get_string));
  EXPECT_STREQ("change_val11", get_string.c_str());

  EXPECT_EQ(1, redis_client->Delete({"key11"}));

  // uint64 GetSet test
  EXPECT_EQ(0, redis_client->Set("numkey11", 12345678));
  uint64_t test_value  = 0;
  EXPECT_EQ(0, redis_client->Get("numkey11", test_value));
  EXPECT_EQ(12345678, test_value);
  test_value = std::numeric_limits<uint64_t>::max();
  EXPECT_EQ(0, redis_client->GetSet("numkey11", test_value));
  EXPECT_EQ(12345678, test_value);
  // uint64 data 에 대한 uint32 변수로 접근 테스트.
  uint32_t get_uint32_value = 0;
  EXPECT_EQ(-1, redis_client->Get("numkey11", get_uint32_value));
  EXPECT_EQ(0, get_uint32_value);

  EXPECT_EQ(0, redis_client->Get("numkey11", test_value));
  EXPECT_EQ(std::numeric_limits<uint64_t>::max(), test_value);

  EXPECT_EQ(1, redis_client->Delete({"numkey11"}));
}

//  multiple redis client 의 테스트를 위한 Fixture class
class TestMultipleRedisClient : public ::testing::Test {
protected:
  virtual void SetUp() {
    //
    first_redis_client = std::shared_ptr<gstd::client::redis::Client> 
                          (new gstd::client::redis::Client("127.0.0.1", 6379, 0));
    ASSERT_TRUE(first_redis_client->Connect());
    second_redis_client = std::shared_ptr<gstd::client::redis::Client> 
                            (new gstd::client::redis::Client("127.0.0.1", 6379, 0));
    ASSERT_TRUE(second_redis_client->Connect());
  }

  virtual void TearDown() {
    //
    first_redis_client->Close();
    second_redis_client->Close();
  }
  std::shared_ptr<gstd::client::redis::Client>  first_redis_client;
  std::shared_ptr<gstd::client::redis::Client>  second_redis_client;
};

//  WATCH 가 미실행 되었을때의 CheckAndSet 테스트.
//  WATCH 미실행 상태일 경우 CheckAndSet 은 일반적인 Set 과 동일하게 동작한다.
TEST_F(TestMultipleRedisClient, CheckAndSetWithoutWatchTest)
{
  //  Watch 호출 없이 CheckAndSet test
  //  일반적인 Set 함수호출과 동일하게 동작
  std::string get_value = "";
  EXPECT_EQ(0, first_redis_client->Set("cns_key1", "cns_value1"));
  EXPECT_EQ(0, first_redis_client->Get("cns_key1", get_value));
  EXPECT_STREQ("cns_value1", get_value.c_str());
  get_value.clear();
  EXPECT_EQ(0, second_redis_client->Get("cns_key1", get_value));
  EXPECT_STREQ("cns_value1", get_value.c_str());

  EXPECT_EQ(0, second_redis_client->CheckAndSet("cns_key1", "cns_value2"));
  EXPECT_EQ(0, first_redis_client->CheckAndSet("cns_key1", "cns_value3"));
  EXPECT_EQ(0, first_redis_client->Get("cns_key1", get_value));
  EXPECT_STREQ("cns_value3", get_value.c_str());
  first_redis_client->Delete("cns_key1");
}


//  WATCH CheckAndSet(MULTI SET EXEC) 의 동작에 대한 테스트
//  WATCH 실행중인 key 에 대하여 다른 커넥션에서 값을 변경할 경우의 실패 처리 테스트.
TEST_F(TestMultipleRedisClient, CheckAndSetWithWatchTest)
{
  //  Watch 를 이용한 CheckAndSet 동작 테스트
  std::string get_value = "";
  EXPECT_EQ(0, first_redis_client->Set("cns_key1", "cns_value1"));
  //  first client 에서 테스트키 WATCH
  EXPECT_EQ(0, first_redis_client->Watch("cns_key1"));
  //  second client 의 Get 에는 문제 없음.
  EXPECT_EQ(0, second_redis_client->Get("cns_key1", get_value));
  EXPECT_STREQ("cns_value1", get_value.c_str());

  //  second clinet 에서 first client 가 WATCH 실행중인 키에 대한 값 변경
  EXPECT_EQ(0, second_redis_client->Set("cns_key1", "cns_value2"));

  //  first client 에서 CheckAndSet 실행시 실패처리.
  EXPECT_EQ(1, first_redis_client->CheckAndSet("cns_key1", "cns_value3"));
  //  fitst client 에서 변경 시도한 값이 아닌 second client 에서 변경한 값이 설정 된 상태.
  EXPECT_EQ(0, first_redis_client->Get("cns_key1", get_value));
  EXPECT_STREQ("cns_value2", get_value.c_str());

  //  해당 key 에 대한 WATCH 재실행.
  EXPECT_EQ(0, first_redis_client->Watch("cns_key1"));
  //  정상적인 CheckAndSet 실행.
  EXPECT_EQ(0, first_redis_client->CheckAndSet("cns_key1", "cns_value5"));
  //  결과 확인.
  EXPECT_EQ(0, first_redis_client->Get("cns_key1", get_value));
  EXPECT_STREQ("cns_value5", get_value.c_str());
  first_redis_client->Delete("cns_key1");
}

TEST_F(TestMultipleRedisClient, CheckAndSetWithUnWatchTest)
{
  std::string get_value = "";
  EXPECT_EQ(0, first_redis_client->Set("cns_key1", "cns_value1"));
  //  first client 에서 테스트키 WATCH
  EXPECT_EQ(0, first_redis_client->Watch("cns_key1"));

  //  second clinet 에서 first client 가 WATCH 실행중인 키에 대한 값 변경
  EXPECT_EQ(0, second_redis_client->Set("cns_key1", "cns_value2"));
  //  first client 에서 실행된 WATCH 에 대하여 해제.
  //  WATCH 가 해제될 경우 다른 커넥션에서 해당 키를 변경하여도 CheckAndSet 실행시 정상처리.
  EXPECT_EQ(0, first_redis_client->UnWatch());

  //  first client 에서 CheckAndSet 실행시 WATCH 가 해제되었으므로 성공.
  EXPECT_EQ(0, first_redis_client->CheckAndSet("cns_key1", "cns_value3"));
  //  fitst client 에서 변경 시도한 값이 설정 된 상태.
  EXPECT_EQ(0, first_redis_client->Get("cns_key1", get_value));
  EXPECT_STREQ("cns_value3", get_value.c_str());

  first_redis_client->Delete("cns_key1");
}


TEST_F(TestMultipleRedisClient, MultiTest)
{
  gstd::client::redis::MultipleData set_multiple_data = {{"mkey1", "val1"}, {"mkey2", "val2"}};

  EXPECT_EQ(0, first_redis_client->Multi());
  EXPECT_EQ(0, first_redis_client->SetMultiple(set_multiple_data, false));

  //  MULTI 실행중 EXEC 를 실행하기 전의 데이터는 redis 의 큐에만 저장되는 상태.
  std::string get_string = "";
  EXPECT_EQ(1, second_redis_client->Get("mkey1", get_string));
  EXPECT_EQ(1, second_redis_client->Get("mkey2", get_string));
  //  EXEC 실행시 redis 에 반영 된다.
  EXPECT_EQ(0, first_redis_client->Exec());

  EXPECT_EQ(0, second_redis_client->Get("mkey1", get_string));
  EXPECT_STREQ("val1", get_string.c_str());
  EXPECT_EQ(0, second_redis_client->Get("mkey2", get_string));
  EXPECT_STREQ("val2", get_string.c_str());

  EXPECT_EQ(2, first_redis_client->Delete({"mkey1", "mkey2"}));
}

//  multi command 와 multiple watch 를 이용한 테스트

TEST_F(TestMultipleRedisClient, MultiWithWatchTest)
{
  gstd::client::redis::MultipleData set_multiple_data
    = {{"mkey1", "val1"}, {"mkey2", "val2"}, {"mkey3", "val3"}};
  gstd::client::redis::MultipleData set_multiple_second_data
    = {{"mkey1", "val1-1"}, {"mkey2", "val2-1"}, {"mkey3", "val2-1"}};
  gstd::client::redis::MultipleData get_multiple_data
    = {{"mkey1", ""}, {"mkey2", ""}, {"mkey3", ""}};

  EXPECT_EQ(0, first_redis_client->SetMultiple(set_multiple_data, false));
  //  여러 키에 대한 WATCH 실행
  EXPECT_EQ(0, first_redis_client->Watch({"mkey1", "mkey2", "mkey3"}));
  //  Check And Set 을 위한 MULTI 실행
  EXPECT_EQ(0, first_redis_client->Multi());
  
  //  다른 커넥션에서의 값 변경. 실패 유도를 위한 설정.
  EXPECT_EQ(0, second_redis_client->Set("mkey1", "val1-2"));

  //  MULTI 실행중 SetMultiple 실행시 queue 에 추가되며 정상 값을 리턴.
  EXPECT_EQ(0, first_redis_client->SetMultiple(set_multiple_second_data, false));

  //  multiple WATCH 실행중인 key 값이 변경 되었으므로 exec 실행시 실패
  EXPECT_EQ(1, first_redis_client->Exec());
  uint32_t get_count = 0;
  EXPECT_EQ(0, first_redis_client->GetMultiple(get_multiple_data, get_count));
  EXPECT_EQ(3, get_count);

  //  mkey1 의 값은 second client 에서 변경한 값으로 변경되어 있다.
  EXPECT_STREQ(get_multiple_data["mkey1"].c_str(), "val1-2");

  //  그외 multiple set 에서 실패한 다른 키의 경우는 기존의 값 그대로이다.
  EXPECT_STREQ(get_multiple_data["mkey2"].c_str(), "val2");
  EXPECT_STREQ(get_multiple_data["mkey3"].c_str(), "val3");

  EXPECT_EQ(3, first_redis_client->Delete({"mkey1", "mkey2", "mkey3"}));
}
