#ifndef GSTD_CONTAINER_BASE_ADVANCED_QUEUE_H
#define GSTD_CONTAINER_BASE_ADVANCED_QUEUE_H

#include <queue>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#include <WinBase.h>
#else  // _LINUX
#include <pthread.h>
#endif

namespace gstd {
namespace container {

// std::queue import한 static template class
// 소스전체 공용 사용위한 advanced queue
// 동일한 타입에 대해 다중 큐로 사용
// max값 설정시 최대 queue사이즈 이상 증가방지
// mutex lock 적용
template<class T>
class BaseAdvancedQueue { 
public:
  BaseAdvancedQueue<T>(int queuecnt=1, int maxsize=0);
  virtual ~BaseAdvancedQueue<T>();

  virtual int   Push(T item, int queueno=1);
  virtual int   Pop(T& item, int queueno=1);
  bool  IsEmpty(int queueno=1);
  int   Count(int queueno=1);
  int   Size(int queueno=1);

  //* queueno 해당하는 큐를 클리어한다. 0일경우 전체 클리어. return -1 인경우 실패
  int   Clear(int queueno=1);

  void  SetDeleteOldData(){is_delete_old_=true;}
  void  set_is_delete_old(bool is_delete_old) { is_delete_old_ = is_delete_old; }

protected:
  void  Lock_(int queueno=1);
  void  Unlock_(int queueno=1);

#if defined(_WIN32)
  PCRITICAL_SECTION         critical_section_;
#else  //* _LINUX
  pthread_mutex_t*          mutex_;
#endif
  std::queue<T>*            queue_;
  int                       queue_count_;
  int                       queue_max_;
  bool                      is_delete_old_;
};

/*      정의      */

template<class T>
BaseAdvancedQueue<T>::BaseAdvancedQueue(int queuecnt, int queuemax)
:queue_count_(queuecnt), queue_max_(queuemax)
{
  is_delete_old_=false;
#if defined(_WIN32)
  critical_section_ = new CRITICAL_SECTION[queuecnt];
  if (critical_section_ == NULL) {
    fprintf(stderr, "failed Advanced queue allocate(CRITICAL_SECTION)\n");
    exit(0);
  }
  for (int i=0;i<queuecnt;i++) {
    InitializeCriticalSection(&critical_section_[i]);
  }
#else  //! _LINUX
  mutex_ = new pthread_mutex_t[queuecnt];
  if (mutex_ == NULL) {
    fprintf(stderr, "failed Advanced queue allocate(mutex)\n");
    exit(0);
  }
  for (int i=0;i<queuecnt;i++) {
    pthread_mutex_init(&mutex_[i], NULL);
  }  
#endif
  queue_ = new std::queue<T>[queuecnt];
  if (queue_ == NULL) {
    fprintf(stderr, "Failed advanced queue allocate(queue)\n");
    exit(0);
  }
}

template<class T>
BaseAdvancedQueue<T>::~BaseAdvancedQueue()
{
#ifdef _WIN32
  if (critical_section_) {
    for (int i=0;i<queue_count_;i++) {
      DeleteCriticalSection(&critical_section_[i]);
    }
    delete[] critical_section_;
    critical_section_ = NULL;
  }
#else  //! _LINUX
  if (mutex_) {
    delete[] mutex_;
    mutex_ = NULL;
  }
#endif
  if (queue_) {
    delete[] queue_;
    queue_ = NULL;
  }
}

template<class T>
void BaseAdvancedQueue<T>::Lock_(int queueno)
{
  if (queueno > 0 && queueno <= queue_count_)
#ifdef _WIN32
    EnterCriticalSection(&critical_section_[queueno-1]);
#else
    pthread_mutex_lock(&mutex_[queueno-1]);  
#endif
}
template<class T>
void BaseAdvancedQueue<T>::Unlock_(int queueno)
{
  if (queueno > 0 && queueno <= queue_count_)  
#ifdef _WIN32
    LeaveCriticalSection(&critical_section_[queueno-1]);
#else
    pthread_mutex_unlock(&mutex_[queueno-1]);
#endif
}
template<class T>
bool BaseAdvancedQueue<T>::IsEmpty(int queueno)
{
  bool ret=false;
  if (queueno > 0 && queueno <= queue_count_)  
    ret = queue_[queueno-1].empty();
  return ret;
}
template<class T>
int BaseAdvancedQueue<T>::Size(int queueno)
{
  int ret=0;
  if (queueno > 0 && queueno <= queue_count_)  
    ret = queue_[queueno-1].size();
  return ret;
}
template<class T>
int BaseAdvancedQueue<T>::Count(int queueno)
{
  return Size(queueno);
}
template<class T>
int BaseAdvancedQueue<T>::Push(T item, int queueno)
{
  int ret=0;  
  if (queueno > 0 && queueno <= queue_count_) {
    if ((Size(queueno) < queue_max_) || (!queue_max_)) {
      Lock_(queueno);
      queue_[queueno-1].push(item);
      Unlock_(queueno);
      ++ret;
    } else if(is_delete_old_) {  
      // max가 차있을때 오래된데이터 삭제옵션.
      Lock_(queueno);
      if (!IsEmpty(queueno)) {
        queue_[queueno-1].pop();
        queue_[queueno-1].push(item);
        ++ret;
      }
      Unlock_(queueno);
    }
  }
  return ret;
}
template<class T>
int BaseAdvancedQueue<T>::Pop(T& item, int queueno)
{
  int ret=0;
  if (queueno > 0 && queueno <= queue_count_) {
    Lock_(queueno);
    if (!IsEmpty(queueno)) {
      item = queue_[queueno-1].front();
      ++ret;
      queue_[queueno-1].pop();
    }
    Unlock_(queueno);
  }
  return ret;
}

template<class T>
int BaseAdvancedQueue<T>::Clear(int queueno)
{
  int ret=0;
  if (queueno > 0 && queueno <= queue_count_) {
    Lock_(queueno);
    while(!IsEmpty(queueno)){
      queue_[queueno-1].pop();
      ++ret;
    }
    Unlock_(queueno);
  } else if(!queueno) {
    for(int no=1 ;no <= queue_count_ ;no++){
      Lock_(no);
      while(!IsEmpty(no)){
        queue_[no-1].pop();
        ++ret;
      }
      Unlock_(no);
    }
  } else {
    ret = -1;
  }
  return ret;
}

}
}

#endif
