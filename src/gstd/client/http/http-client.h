#ifndef GSTD_CLIENT_HTTP_CLIENT_H
#define GSTD_CLIENT_HTTP_CLIENT_H

#include <stdint.h>
#include <string>
#include <list>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "gstd/client/common/error_code.h"

// curl callback 함수에 전달할 데이터를 받을 구조체
struct curl_fetch_data {
  char *data_;      //* 응답 데이터 주소
  size_t length_;   //* 데이터 길이
  curl_fetch_data() {
    data_ = NULL;
    length_ = 0;
  }
  ~curl_fetch_data() {
    if (length_) {
      if (data_) {
        free(data_);
        data_ = NULL;
        length_ = 0;
      }
      length_ = 0;
    }
  }
};

namespace gstd {
namespace client {
namespace http {


//! libcurl global function
void CurlGlobalInit();
void CurlGlobalExit();

// http response code 에 대한 enum 정의
enum ResponseCode {
  kContinue = 100,
  // Success
  kOK = 200,
  kSuccess = 200,
  kCreate = 201,
  kNoContent = 204,
  kResetContent = 205,

  // Redirection
  // Client error
  kBadRequest = 400,
  kUnAuthorized = 401,
  kForbidden = 403,
  kNotFound = 404,
  kMethodNotAllowed = 405,
  kNotAcceptable = 406,
  kRequestTimeout = 408,
  kConflict = 409,
  kPayloadTooLarge = 413,
  kURITooLong = 414,

  // Server error
  kInternalServerError = 500,
  kNotImplemented = 501,
  kBadGateway = 502,
  kServiceUnavailable = 503
};

// http method enum 정의
enum ReqeustMethod {
  kNull = 0,
  kEmpty = 0,
  kGet,
  kPost,
  kPut,
  kDelete,
  kHead
};

// http client 에서 사용할 기본값에 대한 enum 정의
enum Default {
  kTimeout = 5, // sec
  kRetries = 3  // 재시도
}; // enum Default

/*
  @class HttpClient
  @brief  
    CURL 을 이용한 Http 기본 클라이언트
    libcurl 을 포함하여 빌드가 필요하다
    OpenSSL static library 를 참조하여 Https 사용 가능
    리눅스, 윈도우 전부 사용가능하도록 개발 (리눅스 테스트 예정)
*/
class HttpClient {
public:
  HttpClient();
  ~HttpClient();
  static std::string GetCurlVersion();
  static std::string GetSSLVersion();

  // executor
  void Clear();
  bool Post();
  bool Get();
  //bool Put();
  //bool Delete();

  // client setter
  // CURL post field 에 post body string 을 추가한다.
  bool SetPostField(const char* contents);
  // set TCP Keepalive 
  bool SetTcpKeepAlive();
  // CURL post form 구조체에 post 값을 추가한다.
  bool AddPost(const char* name, const char* contents);
  // CURL post form array 에 리스트 contents 를 추가한다.
#if defined(_WIN32) || (__cplusplus >= 201103L)
  bool AddPostArray(const char* name, std::initializer_list<std::string> list_contents);
#else // linux
  bool AddPostArray(const char* name, std::list<std::string> list_contents);
#endif
  // CURL header 리스트에 헤더를 추가한다.
  // 이전 헤더가 있을경우 중복이 되므로 ClearHeader 를 선행해야 한다.
  bool AddHeader(const char* header);
  // CURL 에 설정된 헤더를 초기화한다.
  void ClearHeader();
  // CURL 실행된 결과값을 초기화 한다.
  void FreeResult();

  // variables getter
  // HTTP 의 결과로 받아 저장된 문자열의 길이를 가져온다
  size_t GetResponseDataSize() const { return data_.length_; }
  // HTTP 의 결과로 받아 저장된 문자열을 가져온다
  const char* GetResponseData() const { return data_.data_; }
  // HTTP 의 결과 response code 문자열을 가져온다
  std::string GetResponseCodeString();

  common::error::Code   last_error_code() const { return last_error_code_; }
  std::string           last_error_message() const { return last_error_message_; }
  int32_t               response_code() const { return response_code_; }
  double                connection_time() const { return connection_time_; }
  double                download_speed() const { return download_speed_; }
  double                download_page_size() const { return download_page_size_; }
  bool                  is_debug() const { return is_debug_; }
  bool                  is_verify() const { return is_verify_; } 

  std::string url() const { return url_; }
  // variables setter
  void set_url(std::string url) { url_ = url; }
  void set_timeout(int32_t timeout) { timeout_ = timeout; }
  void set_retries(int32_t retries) { retries_ = retries; }
  void set_is_debug(bool is_debug) { is_debug_ = is_debug; }
  void set_is_verify(bool is_verify) { is_verify_ = is_verify; }
  void set_download_file_name(std::string download_file_name) { download_file_name_ = download_file_name;}
  void set_last_error_code(common::error::Code  error_code) { last_error_code_ = error_code; }

private:

  // 설정된 정보를 토대로 HTTP request 하는 함수
  bool Request_(ReqeustMethod method);

  // CURL 사용에 필요한 callback function, call back buffer 설정하는 함수
  // is_file 옵션 사용시 file download
  bool SetCallback_(bool is_file = false);
  // HttpClient 에 설정된 기본정보를 CURL 에 설정하는 함수
  bool SetBase_();
  // 추가된 헤더정보가 있을경우 CURL 에 설정하는 함수
  bool SetHeader_();
  // 추가된 post form data 가 있을 경우 CURL 에 설정하는 함수
  bool SetPost_();

  // CURL 실행 결과가 성공인 경우 관련 정보를 변수에 set 하는 함수
  void SetResponseValue_();

  // 초기화
  // libcurl 에서 할당된 모든 메모리를 반환한다.
  void ClearLibcurl_();
  // 요청이후의 결과값들에 대해서 초기화한다.
  void ClearResponseValue_();
  // CURL 결과버퍼를 초기화한다.
  bool ResetResultBuffer_(int32_t size);

private:
  struct curl_fetch_data data_;
  std::string last_error_message_;
  std::string url_;
  std::string buffer_;
  std::string download_file_name_;
  common::error::Code   last_error_code_;
#if defined(_WIN32) || (__cplusplus >=201103L)
  CURL* curl_ = NULL;
  struct curl_httppost* post_ = NULL;
  struct curl_httppost* last_ = NULL;
  struct curl_slist* header_ = NULL;

  int32_t timeout_ = Default::kTimeout;
  int32_t retries_ = Default::kRetries;

  int32_t response_code_ = ResponseCode::kOK;
  double  connection_time_ = 0.0;
  double  download_speed_ = 0.0;
  double  download_page_size_ = 0.0;

  bool    is_force_post_ = false;
  bool    is_debug_ = false;
  bool    is_verify_ = true;      // SSL 인증서 검증
#else
  CURL* curl_;
  struct curl_httppost* post_;
  struct curl_httppost* last_;
  struct curl_slist* header_;

  int32_t timeout_;
  int32_t retries_;

  int32_t response_code_;
  double  connection_time_;
  double  download_speed_;
  double  download_page_size_;

  bool    is_force_post_;
  bool    is_debug_;
  bool    is_verify_;      // SSL 인증서 검증
#endif
};

} // namespace http
} // namespace client
} // namespace gstd

#endif // __GSTD_CLIENT_HTTP_CLIENT_H__
