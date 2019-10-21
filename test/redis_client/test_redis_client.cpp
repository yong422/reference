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

class TestDerivedRedisClient : public gstd::client::redis::Client, public ::testing::Test {
 protected:
  virtual void SetUp() {
    redis_client
      = new gstd::client::redis::Client("127.0.0.1", 6379, 0);
    redis_client2
      = new gstd::client::redis::Client("127.0.0.1", 6379, 0);
    ASSERT_TRUE(redis_client->Connect());
    ASSERT_TRUE(redis_client2->Connect());
  }

  virtual void TearDown() {
    redis_client->Close();
    redis_client2->Close();
    delete redis_client;
    delete redis_client2;
  }
  gstd::client::redis::Client* redis_client;
  gstd::client::redis::Client* redis_client2;
};

TEST_F(TestDerivedRedisClient, LockUtilTest)
{
  // 요청한 key 값이 최초 호출일 경우 unique id 를 생성하여 리턴하며 내부 캐싱.
  std::string test_unique_id = GetUniqueId_("get-test-key");
  EXPECT_FALSE(test_unique_id.empty());
  // 동일한 key 요청시 내부 캐싱된 값을 리턴.
  EXPECT_STREQ(test_unique_id.c_str(), GetUniqueId_("get-test-key").c_str());
  std::string test_unique_id2 = GetUniqueId_("get-test-key");
  EXPECT_STREQ(test_unique_id.c_str(), test_unique_id2.c_str());

  test_unique_id2 = GetUniqueId_("get-test-key2");
  EXPECT_STRNE(test_unique_id.c_str(), test_unique_id2.c_str());
  test_unique_id = GetUniqueId_("get-test-key2");;
  EXPECT_STREQ(test_unique_id.c_str(), test_unique_id2.c_str());
}

TEST_F(TestDerivedRedisClient, LockUnlockTest)
{
  // my-get-key
  EXPECT_EQ(0, redis_client->Lock("my-get-key", 10000));
  usleep(10000);
  // second connection 에서 동일한 키에 대한 lock instance 생성. 실패. 및 대기처리
  EXPECT_EQ(1, redis_client2->Lock("my-get-key", 1000));
  // 사용완료 후 unlock
  EXPECT_EQ(0, redis_client->Unlock("my-get-key"));
  usleep(1000);
  // second connection 에서 해당 키에 대한 lock 재시도 성공
  EXPECT_EQ(0, redis_client2->Lock("my-get-key", 1000));
  usleep(1000);
  // first connection 에서 이미 생성된 lock instance key 에 대한 lock 시도 실패.
  EXPECT_EQ(1, redis_client->Lock("my-get-key", 10000));
  // unique id 가 해당 connection 의 값이 아니므로 Unlock 불가.
  EXPECT_EQ(1, redis_client->Unlock("my-get-key"));
  // second session 의 lock instance 해제.
  EXPECT_EQ(0, redis_client2->Unlock("my-get-key"));
  // first session 의 empty lock instance 에 대한 해제는 성공.
  EXPECT_EQ(0, redis_client->Unlock("my-get-key"));

}

TEST_F(TestDerivedRedisClient, LockTimeoutTest)
{
  // Lock timeout 에 의힌 만료 테스트 100 ms
  EXPECT_EQ(0, redis_client->Lock("my-get-key", 1000));
  // Lock 불가.
  EXPECT_EQ(1, redis_client2->Lock("my-get-key", 1000));
  usleep(100000);
  EXPECT_EQ(1, redis_client2->Lock("my-get-key", 1000));
  usleep(100000);
  EXPECT_EQ(1, redis_client2->Lock("my-get-key", 1000));
  usleep(800000);
  EXPECT_EQ(0, redis_client2->Lock("my-get-key", 1000));
  EXPECT_EQ(0, redis_client2->Unlock("my-get-key"));
}