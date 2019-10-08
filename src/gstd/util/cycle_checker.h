#ifndef GSTD_UTIL_CYCLE_CHECKER
#define GSTD_UTIL_CYCLE_CHECKER

#include <vector>
// c++11 gnuc version 4.8.1
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 8
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#include <map>

namespace gstd {
namespace util {

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 8
typedef std::unordered_map< std::string, std::pair<uint32_t, uint32_t> >      CheckTimeMap;
#else
typedef std::tr1::unordered_map< std::string, std::pair<uint32_t, uint32_t> > CheckTimeMap;
#endif
typedef CheckTimeMap::iterator                                                CheckTimeMapIterator;

// @class CycleChecker
// @brief  
//        header only
//        동작 주기 체크를 위한 cycle checker
//        초기에 설정된 string key 와 관련되어 cycle check 가능
//        내부적으로 Default "" empty string 기본으로 추가
class CycleChecker {
public:
  CycleChecker(std::vector< std::string > initialize_vector) {
    check_time_map_.clear();
    for (uint16_t index=0; index < initialize_vector.size(); index++) {
      check_time_map_.insert(std::make_pair(initialize_vector[index], std::make_pair(0,0)));
    }
    if (check_time_map_.find("") == check_time_map_.end()) {
      check_time_map_.insert(std::make_pair("", std::make_pair(0,0)));
    }
  }
  ~CycleChecker()
  {
    check_time_map_.clear();
  }
  // current_time 기준.
  // index 에 해당하는 항목의 체크 주기가 되었는지 확인한다.
  // 설정된 최대 index 값을 초과하는 index 에 대해서 요청시 코드오류로 판단하여 false 처리.
  bool CheckCycle(uint32_t current_time, uint32_t check_cycle, std::string index="")
  {

    CheckTimeMapIterator it = check_time_map_.find(index);
    if (it != check_time_map_.end()) {
      if (current_time >= (it->second.first + check_cycle)) {
        it->second.second = it->second.first;
        it->second.first = current_time;
        return true;
      }
    }
    return false;
  }
  // 주기 체크 이후 원복이 필요할 경우 rollback 한다.
  // 현재 설정된 최근 check time 을  바로 이전 check time 으로 원복.
  bool Rollback(std::string index="")
  {
    CheckTimeMapIterator it = check_time_map_.find(index);
    if (it != check_time_map_.end()) {
      it->second.first = it->second.second;
      return true;
    }
    return false;
  }

private:
  // last check time 은 각 key 별로 first(첫번째 이전 시간값) second(두번째 이전 시간값) 로 구성.
  CheckTimeMap  check_time_map_;

};
} // namespace util
} // namespace gstd

#endif // GSTD_UTIL_CYCLE_CHECKER
