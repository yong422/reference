#ifndef GSTD_NET_IP_H
#define GSTD_NET_IP_H

#include <string>
#include <stdint.h>

namespace gstd {
namespace net {

// @brief   ip 관련한 함수
//          ip변환, ip 계산, 예약 ip 검색등에 사용.
class Ip {
public:
  Ip() {};
  ~Ip() {};

  //* Ip address -> Ip number 변환 함수
  static uint32_t     InetAton(std::string ipaddr);

  //* Ip address <- Ip number 변환 함수
  static std::string  InetNtoa(uint32_t ipnum);

  //* bit 로 표현된 subnetmask 값으로 ip 대역값을 구하는 함수
  // ex. 121.78.95.0/24 -> 121.78.95.0 ~ 121.78.95.255
  static int32_t      GetIpaddressRange(std::string ipaddr, uint32_t& startip, uint32_t& endip);

  //* ip address 에 대하여 특별목적 IP Address 인지 확인하는 함수 (사설IP 포함)
  //  https://www.iana.org/assignments/iana-ipv4-special-registry/iana-ipv4-special-registry.xhtml
  static bool         IsSpecialPurposeIp(uint32_t ipnum);
  static bool         IsSpecialPurposeIp(std::string ipaddr);
};

} // namespace net
} // namespace gstd

#endif // GSTD_NET_IP_H