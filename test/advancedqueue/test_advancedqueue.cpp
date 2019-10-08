#include <pthread.h>
#include <memory>
#include <string>
#include <mutex>
#include <gtest/gtest.h>
#include "gstd/container/advancedqueue.h"
#include "gstd/container/ladvancedqueue.h"
#include "gstd/container/blocking_advancedqueue.h"
#include "gstd/util/timer.h"

template<> gstd::container::AdvancedQueue<int>* gstd::container::AdvancedQueue<int>::this_ptr_ = NULL;
template<> gstd::container::BlockingAdvancedQueue<uint32_t>* 
  gstd::container::BlockingAdvancedQueue<uint32_t>::this_ptr_ = NULL;

typedef gstd::container::BlockingAdvancedQueue<uint32_t> __CTQ;
typedef gstd::container::AdvancedQueue<int> __TQ;


// Fixture class
class TestAdvancedQueue : public ::testing::Test {

protected:
  // 테스트에 필요한 설정을 설정
  // 각 테스트 케이스 시작전 호출
  virtual void SetUp() {
    // 테스트를 위한 3개의 큐생성
    gstd::container::AdvancedQueue<int>::InitInstance(3);
    gstd::container::BlockingAdvancedQueue<uint32_t>::InitInstance(3);
    __LTQ = new gstd::container::LAdvancedQueue<int>(1, 5);
  }

  // 테스트 자원 해제
  // 각 테스트 종료시 호출출
  virtual void TearDown() {
    //
    gstd::container::AdvancedQueue<int>::ExitInstance();
    gstd::container::BlockingAdvancedQueue<uint32_t>::ExitInstance();
    delete __LTQ;
  }
  gstd::container::LAdvancedQueue<int>* __LTQ;
};

// test fixture
TEST_F(TestAdvancedQueue, Push)
{
  // 1번큐
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(1, 1));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(2, 1));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(3, 1));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(2, 2));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(3, 2));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(4, 2));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(5, 2));
  // 생성되지않은 큐 번호 푸시
  EXPECT_EQ(0, __TQ::Get()->Push(5, 4));
  EXPECT_EQ(0, __TQ::Get()->Push(5, 6));

  // 큐 사이즈 체크
  EXPECT_EQ(3, __TQ::Get()->Size(1));
  EXPECT_EQ(4, __TQ::Get()->Size(2));
}

// Pop, Clear test
TEST_F(TestAdvancedQueue, PopOrClear)
{
  // 1번큐
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(1, 1));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(2, 1));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(3, 1));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(2, 2));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(3, 2));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(4, 2));
  EXPECT_EQ(1, gstd::container::AdvancedQueue<int>::Get()->Push(5, 2));

  int val = 0;
  EXPECT_EQ(1, __TQ::Get()->Pop(val, 1));
  EXPECT_EQ(1, val);
  EXPECT_EQ(1, __TQ::Get()->Pop(val, 1));
  EXPECT_EQ(2, val);
  EXPECT_EQ(1, __TQ::Get()->Pop(val, 1));
  EXPECT_EQ(3, val);
  EXPECT_EQ(0, __TQ::Get()->Pop(val, 1));
  EXPECT_EQ(1, __TQ::Get()->Pop(val, 2));
  EXPECT_EQ(2, val);
  EXPECT_EQ(1, __TQ::Get()->Pop(val, 2));
  EXPECT_EQ(3, val);
  EXPECT_EQ(2, __TQ::Get()->Clear(2));
  EXPECT_EQ(0, __TQ::Get()->Clear(2));
  EXPECT_EQ(0, __TQ::Get()->Clear(3));
  EXPECT_EQ(-1, __TQ::Get()->Clear(4));
}

TEST_F(TestAdvancedQueue, PushLocalQueue)
{
  EXPECT_EQ(1, __LTQ->Push(1));
  EXPECT_EQ(1, __LTQ->Push(2));
  EXPECT_EQ(1, __LTQ->Push(3));
  EXPECT_EQ(1, __LTQ->Push(4));
  EXPECT_EQ(1, __LTQ->Push(5));
  EXPECT_EQ(0, __LTQ->Push(6));
  EXPECT_EQ(0, __LTQ->Push(7));
  __LTQ->set_is_delete_old(true);
  EXPECT_EQ(1, __LTQ->Push(6));
  EXPECT_EQ(1, __LTQ->Push(7));
  EXPECT_EQ(1, __LTQ->Push(8));
  EXPECT_EQ(1, __LTQ->Push(9));
  EXPECT_EQ(5, __LTQ->Size());
  __LTQ->set_is_delete_old(false);
  EXPECT_EQ(0, __LTQ->Push(5));
  // 큐 최대사이즈 10 으로 변경
  __LTQ->SetLimit(10);
  EXPECT_EQ(1, __LTQ->Push(6));
  EXPECT_EQ(1, __LTQ->Push(7));
  EXPECT_EQ(1, __LTQ->Push(8));
  EXPECT_EQ(1, __LTQ->Push(9));
  EXPECT_EQ(1, __LTQ->Push(6));
  EXPECT_EQ(0, __LTQ->Push(7));
  EXPECT_EQ(0, __LTQ->Push(8));
  EXPECT_EQ(10, __LTQ->Size());
  __LTQ->set_is_delete_old(true);
  EXPECT_EQ(1, __LTQ->Push(6));
  EXPECT_EQ(1, __LTQ->Push(7));
  EXPECT_EQ(1, __LTQ->Push(8));
  EXPECT_EQ(1, __LTQ->Push(9));
  EXPECT_EQ(1, __LTQ->Push(6));
  EXPECT_EQ(10, __LTQ->Size());
}


//  BlockingAdvancedQueue 테스트
//  구현 의도에 맞는 테스트 방식으로 수정
//  Pop 의 경우 timeout 또는 signal 이 호출되었으나 이미 다른스레드에서 가져간 경우 0 을 리턴할 수 있다.
//  기존 numeric 테스트의 경우 각 thread 에서 데이터를 가져갈 경우 mutex 를 사용하면
//  Blocking Queue 의 사용의도와 맞지 않으므로 제거.
//  TODO: 추후 좀더 테스트 케이스를 다양화해서 추가할 예정.


using TESTBQ = gstd::container::BlockingAdvancedQueue< std::shared_ptr<std::string> >;
template<> gstd::container::BlockingAdvancedQueue< std::shared_ptr<std::string> >*
  gstd::container::BlockingAdvancedQueue< std::shared_ptr<std::string> >::this_ptr_ = nullptr;

// 테스트용 스레드 카운트
// pop thread = push thread * 2
#define UNITTEST_BQ_PUSH_THREAD_COUNT 10
#define UNITTEST_BQ_POP_THREAD_COUNT  UNITTEST_BQ_PUSH_THREAD_COUNT * 2

void* TestPopThreadFunction(void* params)
{
  int thread_num = *(reinterpret_cast<int*>(params));
  std::shared_ptr<std::string> get_value;
  int return_value = 0;
  for(uint32_t i=0; i<2500;) {
    return_value = TESTBQ::Get()->Pop(get_value, 1);

    EXPECT_NE(TESTBQ::ExecResult::kError, return_value);
    if (return_value == TESTBQ::ExecResult::kSuccess) {
      EXPECT_STREQ("test", get_value->c_str());
      ++i;
      //usleep(5000);
    } else if (return_value == TESTBQ::ExecResult::kClosed) {
      break;
    } else if (return_value == TESTBQ::ExecResult::kEmpty) {
      // or timeout
      usleep(10000);
    }
  }
}

void* TestPushThreadFunction(void* params)
{
  int thread_num = *(reinterpret_cast<int*>(params));
  uint32_t get_value;
  for(uint32_t i=0; i<5000; i++) {
    usleep(100);
    std::shared_ptr<std::string> push_value = std::shared_ptr<std::string>(new std::string("test"));
    EXPECT_EQ(TESTBQ::ExecResult::kSuccess, TESTBQ::Get()->Push(push_value, 1));
  }
}

// push, pop threads 로 분리되어 blocking queue 에 데이터를 push, pop 처리에 문제없는지에 대한 테스트
// pop threads 는 정해진 데이터의 개수만큼 가져오도록 하며, push threads 에서 push 하는 전체 개수만큼
// pop threads 에서 pop 을 호출한다.
// 각 개별 pop threads 에서 각 정해진 개수만큼 pop 하여 데이터를 가져온 경우 스레드는 종료된다.
// 생성한 전체 pop threads 가 정상 종료되는 경우 테스트가 문제없다고 판단한다.
// 전체 테스트 소요시간은 push 소요되는 시간 + 스레드 생성 및 종료 시간을 고려하여 설정한다.
//
// 2core vm 기준으로 현재 테스트코드에서 1 초 이내 해당 테스트가 완료되어야 정상.
//  함수내부에서 잘못된 락이 걸리거나 대기상태에 걸릴 경우 테스트시간이 길어지며 테스트 실패로 판단.
//
TEST_F(TestAdvancedQueue, TestBlockingAdvancedQueueSharedPtr)
{
  pthread_t pop_thread[UNITTEST_BQ_POP_THREAD_COUNT];
  pthread_t push_thread[UNITTEST_BQ_PUSH_THREAD_COUNT];
  uint32_t value = 1;
  TESTBQ::InitInstance();
  TESTBQ::Get()->set_timeout(5000 /*1 secs*/);
  gstd::util::Timer timer;
  for (int i=0; i < UNITTEST_BQ_POP_THREAD_COUNT; i++) {
    ASSERT_EQ(0, pthread_create(&pop_thread[i], NULL, TestPopThreadFunction, reinterpret_cast<void*>(&i)));
  }
  for (int i=0; i < UNITTEST_BQ_PUSH_THREAD_COUNT; i++) {
    ASSERT_EQ(0, pthread_create(&push_thread[i], NULL, TestPushThreadFunction, reinterpret_cast<void*>(&i)));
  }

  for (int i=0; i < UNITTEST_BQ_POP_THREAD_COUNT; i++) {
    pthread_join(pop_thread[i], NULL);
  }
  for (int i=0; i < UNITTEST_BQ_PUSH_THREAD_COUNT; i++) {
    pthread_join(push_thread[i], NULL);
  }
  EXPECT_GT(1 /*secs*/, timer.Stop());
  EXPECT_EQ(0, TESTBQ::Get()->Size());
  EXPECT_TRUE(TESTBQ::Get()->IsEmpty());


  TESTBQ::ExitInstance();
}

// BlockingAdvancedQueue close test
// 외부에서 BlockingAdvancedQueue 를 close 할 경우 대기 중
// pop threads 에서 정해진 데이터를 가져가기 위해 wait 되어있는 상태.
// 타임아웃값이 없는 무한대기 모드
// 메인 스레드에서 Close 호출시 대기중인 스레드들이 해제되며 큐 접근에 대해 블록.
// Pop, Push 함수 호출시 Closed 리턴하도록 되어있으며 스레드들이 종료되도록 테스트.
//
TEST(TestBlockingAdvancedQueue, TestCloseBlockingQueue)
{
  pthread_t pop_thread[UNITTEST_BQ_POP_THREAD_COUNT];
  TESTBQ::InitInstance();
  // 타임아웃 없이 무한대기 blocking queue
  TESTBQ::Get()->set_timeout(0);
  for (int i=0; i < UNITTEST_BQ_POP_THREAD_COUNT; i++) {
    ASSERT_EQ(0, pthread_create(&pop_thread[i], NULL, TestPopThreadFunction, reinterpret_cast<void*>(&i)));
  }
  // pop thread 별로 10개의 데이터가 pop 될 경우 종료 처리됨.
  // pop thread 전체가 처리해야하는 데이터의 절반만 Push.
  for(uint32_t i=0; i<UNITTEST_BQ_POP_THREAD_COUNT * 5; i++) {
    std::shared_ptr<std::string> push_value = std::shared_ptr<std::string>(new std::string("test"));
    EXPECT_EQ(1, TESTBQ::Get()->Push(push_value, 1));
  }
  // 일부 스레드 또는 전체 스레드 wait 상태.
  // 큐 전체 종료 처리.
  sleep(1);
  // Push 된 데이터에 대해서 전부 Pop 된 상태.
  EXPECT_TRUE(TESTBQ::Get()->IsEmpty());
  EXPECT_EQ(0, TESTBQ::Get()->Size());
  // BlockingAdvancedQueue close 처리하여 wait 중인 스레드 전부 해제 및 접근에 대한 블록 처리.
  TESTBQ::Get()->Close();
  usleep(100);
  // 블록처리된 queue 에 데이터 추가.
  // closed 상태이며 Push 에 대해 전부 실패.
  for(uint32_t i=0; i<10; i++) {
    std::shared_ptr<std::string> push_value = std::shared_ptr<std::string>(new std::string("test"));
    EXPECT_EQ(TESTBQ::ExecResult::kClosed, TESTBQ::Get()->Push(push_value, 1));
  }
  EXPECT_TRUE(TESTBQ::Get()->IsEmpty());
  EXPECT_EQ(0, TESTBQ::Get()->Size());
  
  // Closed 호출시 이미 wait 중이던 pop thread 전부 해제 후 스레드 종료
  for (int i=0; i < UNITTEST_BQ_POP_THREAD_COUNT; i++) {
    pthread_join(pop_thread[i], NULL);
  }
  TESTBQ::ExitInstance();
}