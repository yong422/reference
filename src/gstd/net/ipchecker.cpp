#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <sstream>

#include "gstd/net/ipchecker.h"

namespace gstd {
namespace net {

IpChecker::IpChecker(const char* dbfile)
{
  is_open_ = false;
  if (dbfile) {
    db_file_.assign(dbfile);
  }
}

IpChecker::~IpChecker()
{
  Close();
}

void IpChecker::Close()
{
  if(is_open_){
    MMDB_close(&mmdb_);
    is_open_ = false;
  }
}

bool IpChecker::Open(const char* dbfile)
{
  std::ostringstream string_stream;
  if (dbfile) {
    db_file_.assign(dbfile);
  }
  //! MMDB_open((const char*)file, (uint32_t)flags, (MMDB_s *const)mmdb);
  //! flags MMDB_MODE_MMAP : oepn the database with mmap() (current default)
  if (db_file_.empty()) {
    error_message_.assign("maxminddb filename empty");
    return false;
  }
  int status = MMDB_open(db_file_.c_str(), MMDB_MODE_MMAP, &mmdb_);
  if (status != MMDB_SUCCESS) {
    if (MMDB_IO_ERROR == status) {
      string_stream << "IO error (file : " << db_file_<< ") : " << strerror(errno);
      error_message_ = string_stream.str();
    } else {
      string_stream << "Can't open "<<db_file_<<" - "<<strerror(errno);
      error_message_ = string_stream.str();
    }
    return false;
  } else {
    string_stream << "maxminddb v"<<mmdb_.metadata.binary_format_major_version<<"."
                  <<mmdb_.metadata.binary_format_minor_version<<"-"
                  <<mmdb_.metadata.build_epoch<<" [record:"<<mmdb_.metadata.record_size<<"]";
    db_info_ = string_stream.str();
  }
  is_open_ = true;
  return true;  
}

bool IpChecker::RenewDatabase()
{
  Close();
  // 기존 저장된 데이터베이스 파일 정보로 오픈
  return Open();
}


// @brief  ipstr에 해당하는 ip주소의 entry(정보 row)를 lookup 한다.
bool IpChecker::Find(const char* ipstr)
{
  std::ostringstream string_stream;
  int gai_error, mmdb_error;
  if (!is_open_) return false;
  /**
    typedef struct MMDB_lookup_result_s {
        bool found_entry;
        MMDB_entry_s entry;
        uint16_t netmask;
    } MMDB_lookup_result_s;
    
    found_entry = false 인경우 lookup에 사용된 ip주소가 의미없는 주소인경우. (검색값 없음)
    found_entry = true ipaddress 에 대한 정보가 있는경우.

    entry = 검색된 ipaddress 정보에 쓰이는 구조체.
  */
  memset(&result_, 0x00, sizeof(result_));
  result_ = 
    MMDB_lookup_string(&mmdb_, ipstr, &gai_error, &mmdb_error);
  
  //! MMDB_lookup_string 함수는 내부에서 getaddrinfo 호출. 실패시 gai_error값은 0이아닌 다른값을 반환.
  if (gai_error != 0) {
    string_stream << "Error from getaddrinfo for "
                  << ipstr << " - " << gai_strerror(gai_error);
    error_message_ = string_stream.str();
    return false;
  }
  
  if (MMDB_SUCCESS != mmdb_error) {
    string_stream << "Got an error from libmaxminddb: "
                  << MMDB_strerror(mmdb_error);
    error_message_ = string_stream.str();
    return false;
  }
  //! 검색 entry 안에 해당 ipaddress가 존재하지 않음.
  if(!result_.found_entry){
    string_stream << "No entry for this IP address ("<<ipstr<<") was found";
    error_message_ = string_stream.str();
    return false;
  }
  return true;
}


// @brief  Find 검색 성공시 해당하는 결과값에서 국가코드, 국가명(english)을 가져온다.
bool IpChecker::GetCountry(char* countryCode, char* countryName)
{
  std::ostringstream string_stream;
  MMDB_entry_data_s data;
  /**
    MMDB_get_value()  검색된 entry에서 data를 선택하여 가져온다.
              검색조건을 이용하여 해당하는 entry만 가져옴.

    @warning  utf8_string, bytes, uint128(uint8 array) 등의 pointer values 는 database의 data section을
          다이렉트로 사용하므로 메모리값 변경등의 작업은 하지않는다.
          필요한경우 strdup, memcpy 등을 사용하여 복사하여 사용.
          utf8_string을 그대로 문자열로 쓸경우 오류발생. data_size 만큼만 복사하여 사용한다.
  */
  int status = MMDB_get_value(&result_.entry, &data, "country", "names", "en", NULL);
  if (status != MMDB_SUCCESS) {
    if ((status = MMDB_get_value(&result_.entry, &data, "registered_country", "names", "en", NULL)) 
          != MMDB_SUCCESS) {
      //! 국가정보가 없을경우 대륙정보를 가져온다.
      return GetContinent_(countryCode, countryName);
    }
  }
  if (data.has_data) {
    switch (data.type) {
      case MMDB_DATA_TYPE_UTF8_STRING:
        if (countryName != NULL)  memcpy(countryName, data.utf8_string, data.data_size);
        //if(&countryName[data.data_size] != NULL)  countryName
        break;
      //! 추후 다른타입의 데이터를 검색하여 가져와야 하는경우 데이터타입을 추가한다. 
      default:
        return false;
        
    }  
  } else {
    return false;  
  }

  status = MMDB_get_value(&result_.entry, &data, "country", "iso_code", NULL);
  if (status != MMDB_SUCCESS) {
    if ((status = MMDB_get_value(&result_.entry, &data, "registered_country", "iso_code", NULL)) 
        != MMDB_SUCCESS) {
      string_stream << "Got an error looking up the entry data - " << MMDB_strerror(status);
      error_message_ = string_stream.str();
      //MMDB_free_entry_data_list(pData);
      return false;
    }
  }
  if (data.has_data) {
    switch (data.type) {
      case MMDB_DATA_TYPE_UTF8_STRING:
        if (countryCode != NULL)  memcpy(countryCode, data.utf8_string, data.data_size);
        break; 
      default:
        break;
        
    }  
  } else {
    return false;  
  }

  return true;
}

bool IpChecker::GetContinent_(char* countryCode, char* countryName)
{
  MMDB_entry_data_s data;
  std::ostringstream string_stream;
  int status = MMDB_get_value(&result_.entry, &data, "continent", "names", "en", NULL);
  if (status != MMDB_SUCCESS){
    string_stream << "Got an error looking up the entry data - " << MMDB_strerror(status);
    error_message_ = string_stream.str();
    return false;
  }
  if (data.has_data) {
    switch (data.type) {
      case MMDB_DATA_TYPE_UTF8_STRING:
        if (countryName != NULL)  memcpy(countryName, data.utf8_string, data.data_size);
        break;
      //! 추후 다른타입의 데이터를 검색하여 가져와야 하는경우 데이터타입을 추가한다. 
      default:
        return false;
        
    }  
  } else {
    return false;  
  }

  status = MMDB_get_value(&result_.entry, &data, "continent", "code", NULL);
  if (status != MMDB_SUCCESS) {
    string_stream << "Got an error looking up the entry data - " << MMDB_strerror(status);
    error_message_ = string_stream.str();
    return false;
  }
  if (data.has_data) {
    switch (data.type) {
      case MMDB_DATA_TYPE_UTF8_STRING:
        if (countryCode != NULL)  memcpy(countryCode, data.utf8_string, data.data_size);
        break; 
      default:
        break;
    }  
  } else {
    return false;  
  }
  return true;
}

/**
  @brief  Find 검색 성공시 해당하는 결과값에서 국가코드, 국가명(english)을 가져온다.
*/
bool IpChecker::GetISP(char* ispName, int size)
{
  return GetEntry_(ispName, size, "isp", NULL);  
}

bool IpChecker::GetOrganization(char* organizationName, int size)
{
  return GetEntry_(organizationName, size, "organization", NULL);
}

bool IpChecker::GetEntry_(char* buf, int size, ...)
{
  bool ret=true;
  va_list vaChk;
  va_start(vaChk, size);
  MMDB_entry_data_s data;
  /**
    MMDB_get_value()  검색된 entry에서 data를 선택하여 가져온다.
              검색조건을 이용하여 해당하는 entry만 가져옴.

    @warning  utf8_string, bytes, uint128(uint8 array) 등의 pointer values 는 database의 data section을
          다이렉트로 사용하므로 메모리값 변경등의 작업은 하지않는다.
          필요한경우 strdup, memcpy 등을 사용하여 복사하여 사용.
          utf8_string을 그대로 문자열로 쓸경우 오류발생. data_size 만큼만 복사하여 사용한다.
  */
  int status = MMDB_vget_value(&result_.entry, &data, vaChk);
  if (status == MMDB_SUCCESS) {
    if (data.has_data) {
      switch (data.type) {
        case MMDB_DATA_TYPE_UTF8_STRING:
          if (buf != NULL && size > 0) {
            if (size > (int)data.data_size) {
              memcpy(buf, data.utf8_string, data.data_size);
            } else {
              memcpy(buf, data.utf8_string, size);
            }
          }
          break;
        //! 추후 다른타입의 데이터를 검색하여 가져와야 하는경우 데이터타입을 추가한다. 
        default:
          ret = false;
          break;
      }  
    } else {
      ret = false;
    }
  }
  va_end(vaChk);
  return ret;
}

// special-purpose address v2
// 참조: https://www.iana.org/assignments/iana-ipv4-special-registry/iana-ipv4-special-registry.xhtml
// 문서 Last update 2017-07-03
#define IPCHECKER_NETMASK(ip, nm) (((iph) >> (32-nm)) << (32-nm))

//bool IpChecker::IsPrivateIp(uint32_t ipnum)
bool IpChecker::IsSpecialPurposeIp(uint32_t ipnum)
{
  uint32_t iph = ntohl(ipnum);
  // 0.0.0.0/8
  if (0x0 == IPCHECKER_NETMASK(iph, 8))
    return true;
  // 10.0.0.0/8
  if (0x0A000000 == IPCHECKER_NETMASK(iph, 8))
    return true;
  // 127.0.0.0/8 Loopback
  if (0x7F000000 == IPCHECKER_NETMASK(iph, 8))
    return true;
  // 100.64.0.0/10 Shared Address Space (IANA)
  if (0x64400000 == IPCHECKER_NETMASK(iph, 10))
    return true;
  // 169.254.0.0/16 Link Local
  if (0xA9FE0000 == IPCHECKER_NETMASK(iph, 16))
    return true;
  // 172.16.0.0/12 Private
  if (0xAC100000 == IPCHECKER_NETMASK(iph, 12))
    return true;
  // 192.0.0.0/24
  if (0xC0000000 == IPCHECKER_NETMASK(iph, 24))
    return true;
  // 192.0.0.0/29 IPv4 Service Continuity
  if (0xC0000000 == IPCHECKER_NETMASK(iph, 29))
    return true;
  // 192.0.2.0/24 documentation
  if (0xC0000200 == IPCHECKER_NETMASK(iph, 24))
    return true;
  // 192.0.0.8/32 IPv4 dummy address
  if (0xC0000008 == IPCHECKER_NETMASK(iph, 32))
    return true;
  // 192.0.0.9/32 Port Control Protocol Anycast
  if (0xC0000009 == IPCHECKER_NETMASK(iph, 32))
    return true;
  // 192.0.0.10/32 Traversal Using Relays around NAT Anycast
  if (0xC0000010 == IPCHECKER_NETMASK(iph, 32))
    return true;
  // 192.0.0.170/32 NAT64/DNS64 Discovery
  if (0xC00000AA == IPCHECKER_NETMASK(iph, 32))
    return true;
  // 192.0.0.171/32 NAT64/DNS64 Discovery
  if (0xC00000AB == IPCHECKER_NETMASK(iph, 32))
    return true;
  // 192.31.196.0/24 AS112-v4
  if (0xC01FC400 == IPCHECKER_NETMASK(iph, 24))
    return true;
  // 192.52.193.0/24 AMT
  if (0xC034C100 == IPCHECKER_NETMASK(iph, 24))
    return true;
  // 192.88.99.0/24 Deprecated (6to4 Relay Anycast)
  if (0xC0586300 == IPCHECKER_NETMASK(iph, 24))
    return true;
  // 192.168.0.0/16 Private
  if (0xC0A80000 == IPCHECKER_NETMASK(iph, 16))
    return true;
  // 192.75.48.0/24 Direct Delegation AS112 Service
  if (0xC04B3000 == IPCHECKER_NETMASK(iph, 24))
    return true;
  // 198.18.0.0/15 Benchmarking
  if (0xC6120000 == IPCHECKER_NETMASK(iph, 15))
    return true;
  // 198.51.100.0/24 Documentation (TEST-NET-2)
  if (0xC6336400 == IPCHECKER_NETMASK(iph, 24))
    return true;
  // 203.0.113.0/24 Documentation (TEST-NET-3)
  if (0xCB007100 == IPCHECKER_NETMASK(iph, 24))
    return true;
  // 240.0.0.0/4 Reserved (IANA)
  if (0xF0000000 == IPCHECKER_NETMASK(iph, 4))
    return true;
  // Limited Broadcast
  if (0xFFFFFFFF == iph)
    return true;
  return false;
}

//bool IpChecker::IsPrivateIp(const char* ipaddr)
bool IpChecker::IsSpecialPurposeIp(const char* ipaddr)
{
  if (ipaddr && strlen(ipaddr) > 0) {
    return IsSpecialPurposeIp(inet_addr(ipaddr));
  }
  return false;
}

} // namespace net
} // namespace gstd