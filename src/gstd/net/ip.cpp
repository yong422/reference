#include "gstd/net/ip.h"
#if defined(_WIN32)
#include <WinSock.h>
#else
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace gstd {
namespace net {

uint32_t Ip::InetAton(std::string ipaddr)
{
  uint32_t result = 0;
  if (!ipaddr.empty()) {
    result = htonl(inet_addr(ipaddr.c_str()));
  }
  return result;
}

std::string Ip::InetNtoa(uint32_t ipnum)
{
  std::string result;
  if (ipnum > 0) {
    sockaddr_in saddr;
    saddr.sin_addr.s_addr = htonl(ipnum);
    result.assign(inet_ntoa(saddr.sin_addr));
  }
  return result;
}

int32_t Ip::GetIpaddressRange(std::string ipaddr, uint32_t& startip, uint32_t& endip)
{
  int32_t result = 0;
  std::string del = "/";
  size_t    pos = 0;
  uint32_t  ipn = 0;
  std::string token;
  if (ipaddr.find(del) != std::string::npos) {
    while ((pos = ipaddr.find(del)) != std::string::npos) {
      token = ipaddr.substr(0, pos);
      ipn   = htonl(inet_addr(token.c_str()));
      ipaddr.erase(0, pos + del.length());
    }
    uint16_t  nbit    = atoi(ipaddr.c_str());
    uint32_t netmask = 0xFFFFFFFF;
    uint32_t ipcount = 0;
    for (uint16_t i = nbit; i < 32; i++) {
      netmask = (netmask << 1) | 0x0;
      ipcount = (ipcount << 1) | 0x1;
    }
    startip = ipn & netmask;
    endip  = startip + ipcount;
    result = ipcount + 1;
  } else {
    startip = Ip::InetAton(ipaddr);
    endip = startip;
    result = 1;
  }
  return result;
}

// special-purpose address v2
// 참조: https://www.iana.org/assignments/iana-ipv4-special-registry/iana-ipv4-special-registry.xhtml
// 문서 Last update 2017-07-03
#define GSTD_NET_IP_NETMASK(ip, nm) (((iph) >> (32-nm)) << (32-nm))

bool Ip::IsSpecialPurposeIp(uint32_t ipnum)
{
  uint32_t iph = ntohl(ipnum);
  // 0.0.0.0/8
  if (0x0 ==   GSTD_NET_IP_NETMASK(iph, 8))
    return true;
  // 10.0.0.0/8
  if (0x0A000000 ==   GSTD_NET_IP_NETMASK(iph, 8))
    return true;
  // 127.0.0.0/8 Loopback
  if (0x7F000000 ==   GSTD_NET_IP_NETMASK(iph, 8))
    return true;
  // 100.64.0.0/10 Shared Address Space (IANA)
  if (0x64400000 ==   GSTD_NET_IP_NETMASK(iph, 10))
    return true;
  // 169.254.0.0/16 Link Local
  if (0xA9FE0000 ==   GSTD_NET_IP_NETMASK(iph, 16))
    return true;
  // 172.16.0.0/12 Private
  if (0xAC100000 ==   GSTD_NET_IP_NETMASK(iph, 12))
    return true;
  // 192.0.0.0/24
  if (0xC0000000 ==   GSTD_NET_IP_NETMASK(iph, 24))
    return true;
  // 192.0.0.0/29 IPv4 Service Continuity
  if (0xC0000000 ==   GSTD_NET_IP_NETMASK(iph, 29))
    return true;
  // 192.0.2.0/24 documentation
  if (0xC0000200 ==   GSTD_NET_IP_NETMASK(iph, 24))
    return true;
  // 192.0.0.8/32 IPv4 dummy address
  if (0xC0000008 ==   GSTD_NET_IP_NETMASK(iph, 32))
    return true;
  // 192.0.0.9/32 Port Control Protocol Anycast
  if (0xC0000009 ==   GSTD_NET_IP_NETMASK(iph, 32))
    return true;
  // 192.0.0.10/32 Traversal Using Relays around NAT Anycast
  if (0xC0000010 ==   GSTD_NET_IP_NETMASK(iph, 32))
    return true;
  // 192.0.0.170/32 NAT64/DNS64 Discovery
  if (0xC00000AA ==   GSTD_NET_IP_NETMASK(iph, 32))
    return true;
  // 192.0.0.171/32 NAT64/DNS64 Discovery
  if (0xC00000AB ==   GSTD_NET_IP_NETMASK(iph, 32))
    return true;
  // 192.31.196.0/24 AS112-v4
  if (0xC01FC400 ==   GSTD_NET_IP_NETMASK(iph, 24))
    return true;
  // 192.52.193.0/24 AMT
  if (0xC034C100 ==   GSTD_NET_IP_NETMASK(iph, 24))
    return true;
  // 192.88.99.0/24 Deprecated (6to4 Relay Anycast)
  if (0xC0586300 ==   GSTD_NET_IP_NETMASK(iph, 24))
    return true;
  // 192.168.0.0/16 Private
  if (0xC0A80000 ==   GSTD_NET_IP_NETMASK(iph, 16))
    return true;
  // 192.75.48.0/24 Direct Delegation AS112 Service
  if (0xC04B3000 ==   GSTD_NET_IP_NETMASK(iph, 24))
    return true;
  // 198.18.0.0/15 Benchmarking
  if (0xC6120000 ==   GSTD_NET_IP_NETMASK(iph, 15))
    return true;
  // 198.51.100.0/24 Documentation (TEST-NET-2)
  if (0xC6336400 ==   GSTD_NET_IP_NETMASK(iph, 24))
    return true;
  // 203.0.113.0/24 Documentation (TEST-NET-3)
  if (0xCB007100 ==   GSTD_NET_IP_NETMASK(iph, 24))
    return true;
  // 240.0.0.0/4 Reserved (IANA)
  if (0xF0000000 ==   GSTD_NET_IP_NETMASK(iph, 4))
    return true;
  // Limited Broadcast
  if (0xFFFFFFFF == iph)
    return true;
  return false;
}

bool Ip::IsSpecialPurposeIp(std::string ipaddr)
{
  if (!ipaddr.empty()) {
    return IsSpecialPurposeIp(inet_addr(ipaddr.c_str()));
  }
  return false;
}

} // namespace net
} // namespace gstd