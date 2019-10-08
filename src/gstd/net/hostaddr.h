#ifndef GSTD_NET_HOSTADDR_H
#define GSTD_NET_HOSTADDR_H


#include <list>
#include <string>

//! 해당 함수는 gstd/net/ip.h 로 이동.
//! 관련 헤더 참조하여 변경이 필요.

// namespace jref
// {
// unsigned int   inet_aton(std::string ipaddr);
// std::string    inet_ntoa(unsigned int);
// int            get_ipaddress_range(std::string ipaddr, unsigned int& startip, unsigned int& endip);
// }


namespace gstd {
namespace net {

// @brief hostname 가져오기.
// @params char* result 결과문자열 ptr
// @params int size     문자열 사이즈.
// @return 실패시 -1, 성공시 0 return
int get_my_hostname(char* result, int size);

//* host ip address 가져오기.
int get_my_hostaddr(char* result, int size);

int get_my_macaddr(char* result, int size, const char* ifname="eth0");
int get_my_macaddr(unsigned char* result, int size, const char* ifname="eth0");

//! lo 를 제외한 interface_list에서 가장첫번째.
int get_my_first_interface_name(char* result, int size);

class HostInfo{
public:
  HostInfo(){}
  ~HostInfo(){}
  static int GetListHostAddress(std::list<std::string>* pVec);
  static unsigned int GetInterfaceIpnum(const char* ifname);
  static unsigned int GetIpRangeBySubnetMast(std::string ipv, unsigned int& startIp, unsigned int& endIp);
};

} // namespace net
} // namespace gstd

#endif
