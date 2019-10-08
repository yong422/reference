#ifndef GSTD_NET_IPCHECKER_H
#define GSTD_NET_IPCHECKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>
#include "maxminddb.h"

namespace gstd {
namespace net {

// @brief Warpper Maxminddb Data type  /usr/local/include/maxminddb.h 참조.
class IpChecker{
public:
  IpChecker(const char* dbfile=NULL);
  virtual ~IpChecker();

private:
  // entry 검색성공후 국가정보가 없는 ip의경우 continent(대륙) 정보를 가져온다.
  bool  GetContinent_(char* countryCode, char* countryName);

  bool  GetEntry_(char* buf, int size, ...);
public:
  bool  IsOpen(){ return is_open_; }
  // maxmind db 를 오픈한다.
  bool  Open(const char* dbfile=NULL);
  // maxmind db 를 닫는다.
  void  Close();
  // maxmind db 파일을 재오픈한다.
  bool  RenewDatabase();
  bool  Find(const char* ipstr);
  bool  GetCountry(char* countryCode, char* countryName);
  bool  GetISP(char* ispName, int size);
  bool  GetOrganization(char* organizationName, int size);

  const char* GetLastErrorMsg() const { return error_message_.c_str(); }
  const char* GetMMDBInfo() const { return db_info_.c_str(); }
  // v2
  std::string error_message() const { return error_message_; }
  std::string db_info() const { return db_info_; }
  bool is_open() const { return is_open_; }

  static bool IsSpecialPurposeIp(const char* ip);
  static bool IsSpecialPurposeIp(uint32_t ipnum);
  
private:
  std::string           db_file_;
  std::string           error_message_;
  std::string           db_info_;
  MMDB_s                mmdb_;
  MMDB_lookup_result_s  result_;
  bool                  is_open_;
};

} // namespace net
} // namespace gstd
#endif // __GSTD_NET_IPCHECKER_H__
