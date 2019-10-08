#ifndef GSTD_SFTP_CLIENT_H__
#define GSTD_SFTP_CLIENT_H__

#include <stdint.h>
#include <string>
#include <curl/curl.h>



namespace gstd {

namespace genum {
namespace sftp{
  enum SFtpCode {
    kSuccess = 1,
    kFailedAuth = 2,      // 인증실패
    kEmptyFile = 3,       // 원격지파일 없음
    kErrorUrl = 4,        // url 오류
    kAccessDenied = 5,    // 원격지파일 접근불가
    kErrorDirectory = 6,  // 접근파일이 디렉토리인 경우
    kOpenFailed = 7,      // 로컬 파일오픈 실패
    //... 추가
  };
  enum SFtpType {
    kDownload = 0,    // 파일 다운로드
    kGetFile = 1,     // 파일 다운로드
    kGet = 2,         // 파일 다운로드
    kUpload = 3,      // 파일 업로드   (없을경우 업로드)
    kUpdate = 4,      // 파일 업데이트 (있을경우 새로이 업로드)
    kAppendUpload = 5 // 기존 파일 있을경우 추가하여 덮어쓰기 업로드
  };
} // namespace sftp
} // namespace genum

/**
  libcurl(libssh2) 이용한 simple sftp client
  간단한 파일 다운로드 및 업로드를 위해 구현
*/
class SFtpClient {
 public:
  static void GlobalInitialize();
  static void GlobalCleanup();

  SFtpClient(std::string host, int32_t port);
  ~SFtpClient();
  void Clear();

  genum::sftp::SFtpCode GetFile(std::string remote_file, std::string wrtie_file);
  genum::sftp::SFtpCode UploadFile(std::string local_file, std::string remote_file);

  // get
  bool is_debug() const {return is_debug_;}
  std::string last_error() const {return last_error_;}

  // set
  void set_host(const std::string& value) {host_ = value;}
  void set_port(const int32_t& value) {port_ = value;}
  void set_user_id(const std::string& value) {user_id_ = value;}
  void set_user_passwd(const std::string& value)  {user_passwd_ = value;}
  void set_is_debug(const bool& value) {is_debug_ = value;}

 private:
  std::string CreateUrl(std::string target) const;
  bool SetOption(genum::sftp::SFtpType sftp_type);

 private:
  bool        is_debug_;
  int32_t     port_;
  std::string host_;
  std::string user_id_;
  std::string user_passwd_;
  std::string last_error_;
  CURL*       curl_;
};
} // namespace gstd
#endif
