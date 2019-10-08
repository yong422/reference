#include <gtest/gtest.h>
#include "gstd/client/redis/redis_client.h"

class TestRedisClient : public ::testing::Test {
 protected:
  // 테스트에 필요한 설정을 설정
  // 각 테스트 케이스 시작전 호출
  virtual void SetUp() {
    //
    redis_client = NULL;
  }

  // 테스트 자원 해제
  // 각 테스트 종료시 호출출
  virtual void TearDown() {
    //
    if (redis_client) delete redis_client;
    redis_client = NULL;
  }
  gstd::client::redis::Client* redis_client;
  std::string strRet;
  unsigned int uRet;
  long long llRet;
  double fRet;
  gstd::client::redis::HashStoreMap hstore_data;
  gstd::client::redis::HashStoreMap hstore_data_result;
};


// 로컬 redis-server 설치후 정상 접속 기능과 auth, data get set, increase decrease 등을 테스트 하기 위한
// 테스트 코드
// 빌드 및 테스트가 진행되는 로컬 서버에 redis-server 가 설치되어 실행된 상태에서 테스트 코드 진행

TEST_F(TestRedisClient, ConnectionTest)
{
  redis_client = new gstd::client::redis::Client("127.0.0.1", 6379, 0);
  redis_client->set_is_debug(true);
  EXPECT_TRUE(redis_client->Connect());
  delete redis_client;
  redis_client = new gstd::client::redis::Client("127.0.0.1", 6378, 0);
  EXPECT_FALSE(redis_client->Connect());
}

TEST_F(TestRedisClient, SetTest)
{
  redis_client = new gstd::client::redis::Client("127.0.0.1", 6379, 0);
  redis_client->set_is_debug(true);
  EXPECT_TRUE(redis_client->Connect());
  EXPECT_EQ(0, redis_client->Set("key1", "string_val"));
  EXPECT_EQ(0, redis_client->Set("key2", (unsigned int)time(NULL)));
  EXPECT_EQ(0, redis_client->Set("key3", (double)3.1456));
  EXPECT_EQ(0, redis_client->Set("key4", "delete test"));
  redis_client->Close();
}

TEST_F(TestRedisClient, GetTest)
{
  redis_client = new gstd::client::redis::Client("127.0.0.1", 6379, 0);
  redis_client->set_is_debug(true);
  EXPECT_TRUE(redis_client->Connect());
  EXPECT_EQ(0, redis_client->Get("key1", strRet));
  EXPECT_STREQ("string_val", strRet.c_str());
  EXPECT_EQ(0, redis_client->Get("key2", uRet));
  EXPECT_NE((unsigned int)time(NULL) - 10, uRet);
  EXPECT_EQ(0,redis_client->Get("key3", fRet));
  EXPECT_EQ(3.1456, fRet);
  EXPECT_EQ(1, redis_client->Get("key5", strRet));
}

TEST_F(TestRedisClient, DeleteTest)
{
  redis_client = new gstd::client::redis::Client("127.0.0.1", 6379, 0);
  redis_client->set_is_debug(true);
  EXPECT_TRUE(redis_client->Connect());
  std::vector<std::string> delvec = {"key1", "key2", "key3"};
  //std::vector<std::string> delvec = {"key1  key2  key3"};
  EXPECT_EQ(3, redis_client->Delete(delvec));
  EXPECT_EQ(1, redis_client->Delete("key4"));
  EXPECT_EQ(0, redis_client->Delete("key5"));
}

TEST_F(TestRedisClient, HSetGetTest)
{
  uint32_t get_count = 0, delete_count = 0;
  redis_client = new gstd::client::redis::Client("127.0.0.1", 6379, 0);
  redis_client->set_is_debug(true);
  hstore_data.insert(std::make_pair("key1", "value1"));
  hstore_data.insert(std::make_pair("key2", "value2"));
  hstore_data.insert(std::make_pair("key4", "value4"));
  hstore_data.insert(std::make_pair("key5", "50552"));
  hstore_data.insert(std::make_pair("key30", "25"));
  EXPECT_TRUE(redis_client->Connect());

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
  redis_client = new gstd::client::redis::Client("127.0.0.1", 6379, 0);
  redis_client->set_is_debug(true);
  ASSERT_TRUE(redis_client->Connect());
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