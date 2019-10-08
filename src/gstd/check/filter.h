#ifndef GSTD_CHECK_FILTER_H
#define GSTD_CHECK_FILTER_H

namespace gstd {
namespace check {

class Filter{
 public:
  Filter(){};
  ~Filter(){};
  //* start time 과 end time 사이에 check time 이 포함되는지 체크. 맞으면 1, 아니면 0
  static int IsExistTimeRange(int start_hour, int start_min, int end_hour, 
                              int end_min, int check_hour, int check_min);

  static int IsExistTimeRange(const char* startTime, const char* chkTime, const char* endTime);
 private:
   
};

} // namespace check
} // namespace gstd

#endif // __GSTD_CHECK_FILTER_H__
