
//  @brief  std::queue import한 static template class
//          소스전체 공용 사용위한 advanced queue
//          동일한 타입에 대해 다중 큐로 사용
//          max값 설정시 최대 queue사이즈 이상 증가방지
//          mutex lock 적용
#ifndef GSTD_CONTAINER_ADVANCED_QUEUE_H
#define GSTD_CONTAINER_ADVANCED_QUEUE_H

#include <queue>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#include <WinBase.h>
#else  // _LINUX
#include <pthread.h>
#endif

#include "gstd/container/base_advancedqueue.h"

namespace gstd {
namespace container {

// std::queue import한 static template class
// 소스전체 공용 사용위한 advanced queue
// 동일한 타입에 대해 다중 큐로 사용
// max값 설정시 최대 queue사이즈 이상 증가방지
// mutex lock 적용
template<class T>
class AdvancedQueue : public BaseAdvancedQueue<T> { 
public:
  AdvancedQueue<T>(int queuecnt=1, int maxsize=0);
  virtual ~AdvancedQueue<T>();

  static void ExitInstance();
  static AdvancedQueue<T>* Get(){return AdvancedQueue<T>::this_ptr_;}
  static AdvancedQueue<T>* InitInstance(int queuecnt=1, int maxsize=0);


protected:

  static AdvancedQueue<T>*  this_ptr_;
};

template<class T>
AdvancedQueue<T>::AdvancedQueue(int queuecnt, int maxsize)
:BaseAdvancedQueue<T>(queuecnt, maxsize)
{

}

template<class T>
AdvancedQueue<T>::~AdvancedQueue()
{

}

template<class T>
AdvancedQueue<T>* AdvancedQueue<T>::InitInstance(int queuecnt, int queuemax)
{
  if (!this_ptr_) {
    this_ptr_ = new AdvancedQueue<T>(queuecnt, queuemax);
  }
  return this_ptr_;
}
template<class T>
void AdvancedQueue<T>::ExitInstance()
{
  if (this_ptr_) {
    delete this_ptr_;
    this_ptr_ = NULL;
  }
}

} // namespace container
} // namespace gstd
#endif
