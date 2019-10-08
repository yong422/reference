#ifndef GSTD_CONTAINER_BLOCKING_ADVANCED_QUEUE_H
#define GSTD_CONTAINER_BLOCKING_ADVANCED_QUEUE_H
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include "gstd/container/base_advancedqueue.h"

namespace gstd {
namespace container {


//  @brief pthread condition variable 을 이용한 blocking advanced queue
//          Linux 용으로 개발.
//          BaseAdvancedQueue 기본 클래스로 하여 pthread condition value 기능 추가
//          기존의 Pop 호출시 데이터가 있을 경우 1, 없을 경우 0 을 바로 리턴한다.
//          변경후 Pop 호출시 데이터가 없으면 timeout 설정값만큼 cond wait 으로 대기한다.
//          타임아웃 또는 데이터가 없을 경우 0,  데이터를 가져오면 1 을 리턴한다.
//          timeout 값이 0 일 경우 데이터가 없다면 Pop 은 무한대기에 들어가며
//           다른 스레드에서 Push 가 발생할 경우 wait 에서 해제된다.
//
//          Push 는 큰 기능 차이는 없으며 데이터 Push 이후 cond signal 을 보내
//           대기중인 다른 스레드의 Pop 내부의 Wait 을 해제시킨다.
//
//          현재 cond signal 은 broadcast 가 아닌 하나의 thread 만 깨우도록 단일 signal 을 사용한다.
//
//          TODO: 윈도우에서 사용에 필요성이 느껴질 경우 윈도우용 개발진행
//                https://docs.microsoft.com/en-us/windows/desktop/sync/condition-variables
//                또는 c++11 https://en.cppreference.com/w/cpp/thread/condition_variable
#if defined(__linux__)
template<class T>
class BlockingAdvancedQueue : public BaseAdvancedQueue<T> {
public:
  BlockingAdvancedQueue<T>(int queuecnt=1, int maxsize=0);
  ~BlockingAdvancedQueue<T>();

  static void ExitInstance();
  static BlockingAdvancedQueue<T>* Get(){ return this_ptr_; }
  static BlockingAdvancedQueue<T>* InitInstance(int queuecnt=1, int maxsize=0);

  // BlockingAdvancedQueue 함수 호출에 대한 result enum
  enum ExecResult {
    kClosed = -2,
    kError = -1,
    kEmpty = 0,
    kTimeout = 0,
    kSuccess = 1 
  };
  // blocking queue 를 close 한다.
  // queue 에 대기중인 스레드들의 해제를 위해 boradcast signal 전달 후 접근을 차단한다.
  void Close();

  // blocking advanced queue 에서 재정의된 Push
  // 내부에서 cond signal 발생 및 리턴값 추가
  // return
  //  1 : 데이터가 정상적으로 추가 및 cond signal 성공
  //  0 : 큐가 가득차거나 큐번호가 없어 데이터 미추가 및 cond signal 호출하지 않음
  // -1 : cond signal 이 비정상(pthread_cond_t 초기화 안된 경우)이어서 signal 을 사용하지 못하는 경우
  virtual int   Push(T item, int queueno=1) override;

  // blocking advanced queu 에서 재정의된 pop
  // 큐가 비어있을 경우 내부에서 cond wait
  // set_timeout(timeout millisec) 으로 설정한 값만큼 blocking 대기, 기본설정은 timeout=0 무한대기.
  // return
  //  1: 데이터를 가져온 경우
  //  0: 데이터가 없을 경우, 타임아웃에 걸린 경우
  //  -1: cond_wait 에서 오류 발생
  virtual int   Pop(T& item, int queueno=1) override;

  void set_timeout(const uint32_t& timeout) { timeout_ = timeout; }
  bool is_closed() const { return is_closed_; }

protected:
  void set_is_closed(const bool& is_closed) { is_closed_ = is_closed; }

  int   Wait_(int queueno=1);
  int   Signal_(int queueno=1);

  bool                                is_closed_ = false; // close flag (true 일 경우 외부에서 큐를 close)
  uint32_t                            timeout_ = 0;       // queue condition value timeout (ms)
  pthread_cond_t*                     cond_value_;
  static BlockingAdvancedQueue<T>*    this_ptr_;
};

template<class T>
BlockingAdvancedQueue<T>::BlockingAdvancedQueue(int queuecnt, int queuemax)
:BaseAdvancedQueue<T>(queuecnt, queuemax)
{
  cond_value_ = new pthread_cond_t[queuecnt];
  if (cond_value_ == NULL) {
    fprintf(stderr, "failed condition value allocate(mutex)\n");
    exit(1);
  }
  for (int i=0;i<queuecnt;i++) {
    // EINVAL (attr 지정하지 않으므로 미발생), ENOMEM
    if (pthread_cond_init(&cond_value_[i], NULL) != 0) exit(1);
  }
}

template<class T>
BlockingAdvancedQueue<T>::~BlockingAdvancedQueue()
{
  for (int i=0;i < this->queue_count_;i++) {
    pthread_cond_destroy(&cond_value_[i]);
  }
  delete[] cond_value_;
}

template<class T>
BlockingAdvancedQueue<T>* BlockingAdvancedQueue<T>::InitInstance(int queuecnt, int queuemax)
{
  if (!this_ptr_) {
    this_ptr_ = new BlockingAdvancedQueue<T>(queuecnt, queuemax);
  }
  return this_ptr_;
}
template<class T>
void BlockingAdvancedQueue<T>::ExitInstance()
{
  if (this_ptr_) {
    delete this_ptr_;
    this_ptr_ = NULL;
  }
}

// 해당하는 큐에 데이터를 push 한 후, condition variable 에 signal 을 날린다.
// blocking queue 가 closed 상태일 경우 push 되지 않는다.
template<class T>
int BlockingAdvancedQueue<T>::Push(T item, int queueno)
{
  int result = BlockingAdvancedQueue::ExecResult::kError;  
  if (is_closed()) return BlockingAdvancedQueue::ExecResult::kClosed;
  if (queueno > 0 && queueno <= this->queue_count_) {
    this->Lock_(queueno);
    if ((this->Size(queueno) < this->queue_max_) || (!this->queue_max_)) {
      this->queue_[queueno-1].push(item);
      if (Signal_(queueno) > 0) result = BlockingAdvancedQueue::ExecResult::kSuccess;
      else                      result = BlockingAdvancedQueue::ExecResult::kError;

    } else if(this->is_delete_old_) {  
      // Lock 이후 사이즈 체크하여 최대값은 넘은 경우이므로 IsEmpty 체크할 필요없으므로 삭제
      this->queue_[queueno-1].pop();
      this->queue_[queueno-1].push(item);

      if (Signal_(queueno) > 0) result = BlockingAdvancedQueue::ExecResult::kSuccess;
      else                      result = BlockingAdvancedQueue::ExecResult::kError;
    }
    this->Unlock_(queueno);
  }
  return result;
}

//  큐가 비어있을 경우 condition variable 에 대해서 wait.
//  signal 을 통한 wait 해제시 데이터를 가져온다.
//  cond_wait 오류나 타임아웃, 데이터가 없을 경우 return 0
//  데이터를 가져온 경우 return 1
template<class T>
int BlockingAdvancedQueue<T>::Pop(T& item, int queueno)
{
  int result    = BlockingAdvancedQueue::ExecResult::kEmpty;
  int err_wait  = BlockingAdvancedQueue::ExecResult::kSuccess;
  if (is_closed()) return ExecResult::kClosed;
  if (queueno > 0 && queueno <= this->queue_count_) {
    this->Lock_(queueno);
    // Wait 으로 대기중 signal이 발생된 경우 Lock 이후 Empty 가 아닌 경우 데이터를 가져오게 한다.
    // Empty 상태인 경우 반복하여 Wait 으로 대기하며 timeout 설정된 값으로 인해 타임아웃 발생시 Pop 에서 나간다.
    while (this->IsEmpty(queueno)) {
      if ((err_wait = Wait_(queueno)) == BlockingAdvancedQueue::ExecResult::kSuccess) {
        // 정상적인 signal 로 해제된 상태이나 queue 가 close 된 경우.
        if (is_closed()) {
          err_wait = BlockingAdvancedQueue::ExecResult::kClosed;
          break;
        }
      } else {
        // timeout 또는 오류
        break;
      }
    } 
    if (err_wait == BlockingAdvancedQueue::ExecResult::kSuccess) {
      item = this->queue_[queueno-1].front();
      this->queue_[queueno-1].pop();
    }
    result = err_wait;
    this->Unlock_(queueno);
  }
  return result;
}

// blocking queue 를 닫는다.
// blocking queue 에 대기중은 모든 스레드를 wait 상태에서 해제하기위해 broadcast signal 을 전달 한다.
template<class T>
void BlockingAdvancedQueue<T>::Close()
{
  set_is_closed(true);
  for (int i=0;i<this->queue_count_;i++) {
    pthread_cond_broadcast(&cond_value_[i]);
  }
}

//  queueno 에 해당하는 큐 condition variable 에 대해 대기한다.
//  timeout = 0 이면 시그널이 있을때까지 대기이며, 그외에는 timeout 값만큼 대기한다.
//  시그널을 통한 정상적인 해제시 return 1, timeout 의 경우 0 그외 오류에 대해서는 -1 을 리턴한다.
//  Wait_ 함수는 Pop 내부에서 호출하며 pthread_cond_wait 에서 호출하는 mutex 에 대한 lock 의 경우
//  Wait_ 함수를 호출하는 Pop 함수등의 외부에 추가한다.
template<class T>
int BlockingAdvancedQueue<T>::Wait_(int queueno)
{
  int local_err = 0;
  if (!timeout_) {
    local_err = pthread_cond_wait(&cond_value_[queueno-1], &(this->mutex_[queueno-1]));
  } else {
    struct timeval now;
    struct timespec ts;
    gettimeofday(&now, NULL);
    ts.tv_sec = now.tv_sec + static_cast<long>(timeout_ / 1000);
    ts.tv_nsec = now.tv_usec + static_cast<long>(timeout_ % 1000);
    local_err = pthread_cond_timedwait(&cond_value_[queueno-1], &(this->mutex_[queueno-1]), &ts);
  }
  if (local_err == ETIMEDOUT) return BlockingAdvancedQueue::ExecResult::kTimeout;
  else if (!local_err)        return BlockingAdvancedQueue::ExecResult::kSuccess;
  
  return BlockingAdvancedQueue::ExecResult::kError;
}

// 해당하는 큐에 siganl 을 날린다.
// pthread_cond_signal 함수는 실패의 경우 EINVAL 오류만 리턴한다.
// 초기화 되지않은 condition variable 을 사용하는 잘못된 경우인 경우 발생.
template<class T>
int BlockingAdvancedQueue<T>::Signal_(int queueno)
{
  if (!pthread_cond_signal(&cond_value_[queueno-1])) 
    return BlockingAdvancedQueue::ExecResult::kSuccess;
  
  return BlockingAdvancedQueue::ExecResult::kEmpty;
}

} // namespace container
} // namespace gstd
#endif // __linux__

#endif // GSTD_CONTAINER_BLOCKING_ADVANCED_QUEUE_H
