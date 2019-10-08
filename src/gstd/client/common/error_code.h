#ifndef GSTD_CLIENT_COMMON_ERROR_CODE_H
#define GSTD_CLIENT_COMMON_ERROR_CODE_H

namespace gstd {
namespace client {
namespace common {
namespace error {

// client 클래스들의 공통 통신관련 에러
// TODO: 오류 발생 케이스들에 대해서 추가작업 예정
//       1차적으로 별도의 예외처리가 필요한 항목에 대해서만 추가후 나머진 기타 오류처리.
enum Code {
  kEtc            = -1,
  kOther          = -1,
  kNoError        = 0,  // 에러없음
  kCouldntConnect = 1,  // 접속불가
  kTimeout        = 2   // 통신 타임아웃
};

} // namespace error
} // namespace common
} // namespace client 
} // namespace gstd

#endif