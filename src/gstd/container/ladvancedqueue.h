#ifndef GSTD_CONTAINER_LADVANCED_QUEUE_H
#define GSTD_CONTAINER_LADVANCED_QUEUE_H

#include <queue>
#include <stdio.h>
#if defined(_WIN32)
#include <Windows.h>
#include <WinBase.h>
#else
#include <pthread.h>
#endif

namespace gstd {
namespace container {

// @brief  advancedqueue와 동일하나 local변수로 사용할 template
template<class T>
class LAdvancedQueue{
public:
  LAdvancedQueue<T>(int queuecnt=1, int maxsize=0);
  ~LAdvancedQueue<T>();

  int  Push(T item, int queueno=1);
  int  Pop(T& item, int queueno=1);
  bool IsEmpty(int queueno=1);
  int  Count(int queueno=1);
  int  Size(int queueno=1);
  void SetLimit(int maxsize){queue_max_ = maxsize;}
  void SetMax(int maxsize){queue_max_ = maxsize;}
  int  Clear(int queueno=1);

  void set_is_delete_old(bool is_delete_old) { is_delete_old_ = is_delete_old; }

private:
  void Lock_(int queueno=1);
  void Unlock_(int queueno=1);

private:
#if defined(_WIN32)
  PCRITICAL_SECTION     critical_section_;
#else
  pthread_mutex_t*      mutex_;
#endif
  std::queue<T>*        queue_;
  int                   queue_count_;
  int                   queue_max_;
  bool                  is_delete_old_;
};

template<class T>
LAdvancedQueue<T>::LAdvancedQueue(int queuecnt, int queuemax)
:queue_count_(queuecnt), queue_max_(queuemax)
{
  is_delete_old_=false;
#if defined(_WIN32)
  critical_section_ = new CRITICAL_SECTION[queuecnt];
  if (critical_section_ == NULL) {
    fprintf(stderr, "failed Advanced queue allocate(CRITICAL_SECTION)\n");
    exit(0);
  }
  for (int i=0; i < queuecnt; i++) {
    InitializeCriticalSection(&critical_section_[i]);
  }
#else
  mutex_ = new pthread_mutex_t[queuecnt];
  if (mutex_ == NULL) {
    fprintf(stderr, "failed Advanced queue allocate(mutex)\n");
    exit(0);
  }
  for (int i=0; i < queuecnt; i++) {
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
LAdvancedQueue<T>::~LAdvancedQueue()
{
#if defined(_WIN32)
  if (critical_section_) {
    for (int i=0; i < queue_count_; i++) {
      DeleteCriticalSection(&critical_section_[i]);
    }
    delete[] critical_section_;
    critical_section_ = NULL;
  }
#else
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
void LAdvancedQueue<T>::Lock_(int queueno)
{
  if (queueno > 0 && queueno <= queue_count_)  
#if defined(_WIN32)
    EnterCriticalSection(&critical_section_[queueno-1]);
#else
    pthread_mutex_lock(&mutex_[queueno-1]);  
#endif
}

template<class T>
void LAdvancedQueue<T>::Unlock_(int queueno)
{
  if (queueno > 0 && queueno <= queue_count_)  
#if defined(_WIN32)
    LeaveCriticalSection(&critical_section_[queueno-1]);
#else
    pthread_mutex_unlock(&mutex_[queueno-1]);
#endif
}

template<class T>
bool LAdvancedQueue<T>::IsEmpty(int queueno)
{
  bool result = false;
  if (queueno > 0 && queueno <= queue_count_)  
    result = queue_[queueno-1].empty();
  return result;
}

template<class T>
int LAdvancedQueue<T>::Size(int queueno)
{
  int result = 0;
  if (queueno > 0 && queueno <= queue_count_)  
    result = queue_[queueno-1].size();
  return result;
}

template<class T>
int LAdvancedQueue<T>::Count(int queueno)
{
  return Size(queueno);
}

template<class T>
int LAdvancedQueue<T>::Push(T item, int queueno)
{
  int result = 0;  
  if (queueno > 0 && queueno <= queue_count_) {
    if ((Size(queueno) < queue_max_) || (!queue_max_)) {
      Lock_(queueno);
      queue_[queueno-1].push(item);
      Unlock_(queueno);
      ++result;
    } else if(is_delete_old_) {  
      // max가 차있을때 오래된데이터 삭제옵션.
      Lock_(queueno);
      if (!IsEmpty(queueno)) {
        queue_[queueno-1].pop();
        queue_[queueno-1].push(item);
        ++result;
      }
      Unlock_(queueno);
    }
  }
  return result;
}

template<class T>
int LAdvancedQueue<T>::Pop(T& item, int queueno)
{
  int result = 0;
  if (queueno > 0 && queueno <= queue_count_) {
    Lock_(queueno);
    if (!IsEmpty(queueno)) {
      item = queue_[queueno-1].front();
      ++result;
      queue_[queueno-1].pop();
    }
    Unlock_(queueno);
  }
  return result;
}

template<class T>
int LAdvancedQueue<T>::Clear(int queueno)
{
  int result = 0;
  if (queueno > 0 && queueno <= queue_count_) {
    Lock_(queueno);
    while (!IsEmpty(queueno)) {
      queue_[queueno-1].pop();
      ++result;
    }
    Unlock_(queueno);
  } else if (!queueno) {
    for (int no=1; no <= queue_count_; no++) {
      Lock_(no);
      while (!IsEmpty(no)) {
        queue_[no-1].pop();
        ++result;
      }
      Unlock_(no);
    }
  } else {
    result = -1;
  }
  return result;
}

} // namespace container
} // namespace gstd
#endif // GSTD_CONTAINER_LADVANCED_QUEUE_H
