#include <stdio.h>
#include "gstd/check/filter.h"

namespace gstd {
namespace check {

int Filter::IsExistTimeRange(const char* startTime, const char* chkTime, const char* endTime)
{
  int sH=0, sM=0, cH=0, cM=0, eH=0, eM=0;
  if( startTime && chkTime && endTime) {
    if(startTime)   sscanf(startTime, "%d:%d", &sH, &sM);
    if(chkTime)     sscanf(chkTime, "%d:%d", &cH, &cM);
    if(endTime)     sscanf(endTime, "%d:%d", &eH, &eM);
    return IsExistTimeRange(sH, sM, eH, eM, cH, cM);
  }
  return 0;
}

int Filter::IsExistTimeRange(int start_hour, int start_min,
                     int end_hour, int end_min,
                     int check_hour, int check_min)
{
  if (start_hour == end_hour && start_min == end_min)   return 1;
  //! 시작 설정시간이 큰경우.
  //! ex 22:00 ~ 06:00  새벽수신
  if (start_hour > end_hour || (start_hour == end_hour && start_min > end_min)) {
    if (check_hour == start_hour) {
      if (check_min >= start_min)   return 1;
      else                              return 0;   //22:30 ~ 22:29 ~ 6:00
    } else if (check_hour > start_hour) {
      return 1;
    } else if (end_hour >= check_hour && end_min > check_min) {
      if( end_hour == check_hour) {
        if (check_min <= end_min) return 1;
        else                           return 0;
      } else if (end_hour > check_hour) {
        return 1;
      }
    }
  } else {   //! 시작 설정이 작은 경우
    int starttime = start_hour*60 + start_min;
    int endtime = end_hour*60 + end_min;
    int chktime = check_hour*60 + check_min;
    if(starttime <= chktime && endtime >= chktime)   return 1;
  }
  return 0;
}

} // namespace check
} // namespace gstd


#ifdef _TEST_FILTER

int main(int argc, char** argv)
{
   CFilter filter;
   
   if(filter.IsExistTimeRange(10,30, 16,30, 20,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }   
   
   if(filter.IsExistTimeRange(10,30, 2,30, 4,30)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   
   if(filter.IsExistTimeRange(22,30, 4,30, 3,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }

   if(filter.IsExistTimeRange(3,30, 5,30, 4,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(10,30, 23,30, 0,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(22,30, 5,30, 12,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(18,0, 22,0, 18,9)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(18,0, 22,0, 18,18)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(18,0, 18,5, 18,1)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(18,5, 18,0, 18,18)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(18,5, 18,0, 18,3)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(1,0, 18,0, 22,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(18,0, 18,0, 6,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(18,0, 22,0, 8,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(8,0, 18,0, 10,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(8,0, 18,0, 5,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   if(filter.IsExistTimeRange(8,0, 18,0, 20,0)){
      fprintf(stdout, " TRUE\n");
   }else{
      fprintf(stdout, " FALSE\n");
   }
   return 0;
}

#endif
