#ifndef GSTD_CHECK_VERSION_H
#define GSTD_CHECK_VERSION_H

#include <stdio.h>
#include <stdint.h>
#include <string>

#define _GSTD_MAXCOUNT_SEMANTIC_VERSION 3

namespace gstd {
namespace check {
namespace version {

enum Type {
  kSemantic = 0,  // major.minor.patch
  kGabiaMon = 1   // major.minor-build
};

enum Result {
  // 동일한 버전
  kSame = 0,
  // 기준보다 높은 버전
  kHigher = 1,
  // 기준보다 낮은 버전
  kLower = 2,

  // 버전값 parsing 실패
  kFailedParsing = 5,
  // 지원하지 않는 버전 검색 타입
  kNotSupportedType = 6
};

} // namespace version

// @class Version
// @brief
//  type 별 version string 과 관련하여 비교를 위한 클래스
//  기본은 semantic 을 적용하며 기타 추가가 필요한 버저닝에 대해서 추가 작업
//
// TODO: 일차적으로 작업시 sscanf_s 로 작업하였으나 이후 추가 및 수정시 정규표현식으로 수정.
class Version {
public:
  Version() {};
  ~Version() {};

  // type 에 맞는 버전형식 중 old_version 과 new_version 이 다른지 체크
  // 결과값은 gstd::check::version::Result
  static uint32_t CheckChanged(version::Type type,
                               std::string current_version,
                               std::string new_version);

private:
  static uint32_t CheckChangedSemantic_(std::string current_version,
                                        std::string new_version);
  static uint32_t CheckChangedGabiaMon_(std::string current_version,
                                        std::string new_version);
};
} // namespace check
} // namespace gstd

#endif  // namespace gstd
