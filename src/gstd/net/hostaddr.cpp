#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "gstd/net/hostaddr.h"

#define HWADDR_LEN     6
#define HWADDR_LEN_S  2

namespace gstd {
namespace net {

int get_my_first_interface_name(char* buf, int size)
{
  int ret = 0;
  struct ifaddrs *addrs,*tmp;

  getifaddrs(&addrs);
  tmp = addrs;

  while (tmp) {
    if (tmp->ifa_addr  != NULL && tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
      if(strcmp("lo", tmp->ifa_name)){
        strncpy(buf, tmp->ifa_name, size);
        ++ret;
      }
    }
    tmp = tmp->ifa_next;
  }

  freeifaddrs(addrs);
  return ret;
}

int get_my_macaddr(char* result, int size, const char* ifname)
{
  unsigned char* pt = new unsigned char[6];
  memset(result, 0x00, size);
  int fd;
  struct ifreq ifr;
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  strcpy(ifr.ifr_name, ifname);
  ioctl(fd, SIOCGIFHWADDR, &ifr);
  for (int i=0; i<HWADDR_LEN; i++) {
    pt[i] = ((unsigned char*)ifr.ifr_hwaddr.sa_data)[i];
  }
  if (strlen((const char*)pt)) { 
    snprintf(result, (HWADDR_LEN*3), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
        pt[0], pt[1], pt[2], pt[3], pt[4], pt[5]);
    for (int i=0;i<strlen(result);i++) {
      if (result[i] == ':')  
        continue;
      else  
        result[i] = (char)toupper(result[i]);
    }
  }
  if(fd > 0)  close(fd);
  delete[]  pt;
  return strlen(result);
}

int get_my_macaddr(unsigned char* result, int size, const char* ifname)
{
  memset(result, 0x00, size);
  int fd;
  struct ifreq ifr;
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  strcpy(ifr.ifr_name, ifname);
  ioctl(fd, SIOCGIFHWADDR, &ifr);
  for (int i=0; i<HWADDR_LEN; i++) {
    result[i] = ((unsigned char*)ifr.ifr_hwaddr.sa_data)[i];
  }
  if(fd > 0)  close(fd);
  return strlen((const char*)result);
}

int get_my_hostaddr(char* result, int size)
{
  int ret=0;
  if(result && size>=INET_ADDRSTRLEN){
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
    char bufhostname[1024] = {0,};
    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr != NULL && ifa ->ifa_addr->sa_family==AF_INET && strcmp(ifa->ifa_name,"lo")) { 
        // check it is IP4
        tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        char addressBuffer[INET_ADDRSTRLEN] = {0, };
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        strncpy(result, addressBuffer, size);
        ret = 1;
        break;
      }     
    }
    if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
  }
  return ret;
}

int get_my_hostname(char* result, int size)
{
  int ret=-1;
  if (result && size>0) {
    ret = gethostname(result, size);
  }
  return ret;
}

int HostInfo::GetListHostAddress(std::list<std::string>* pList)
{
  int ret=0;
  if (pList) {
    pList->clear();
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr != NULL && ifa ->ifa_addr->sa_family==AF_INET && strcmp(ifa->ifa_name,"lo")) { // check it is IP4
        tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        char addressBuffer[INET_ADDRSTRLEN] = {0,};
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        if (strcmp("0.0.0.0", addressBuffer)) {
          pList->push_back(std::string(addressBuffer));
          ++ret;
        }
      }     
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
  }
  return ret;

}

unsigned int HostInfo::GetInterfaceIpnum(const char* ifname)
{
  unsigned int ret=0;
  struct ifaddrs * ifAddrStruct=NULL;
  struct ifaddrs * ifa=NULL;
  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr != NULL && ifa ->ifa_addr->sa_family==AF_INET && !strcmp(ifa->ifa_name,ifname)) { // check it is IP4
      ret = ntohl(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr);
      break;
    }     
  }
  if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
  return ret;

}

} // namespace net
} // namespace gstd