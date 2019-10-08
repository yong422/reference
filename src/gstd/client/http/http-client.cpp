#include "http-client.h"

#include <errno.h>
#include <iostream>
#include <cstring>

#include <openssl/ssl.h>
//! cleanup
#include <openssl/err.h>
//! ENGINE
#include <openssl/engine.h>

namespace gstd {
namespace client {
namespace http {

// http file download 에 사용할 data class
class HttpDownloadFile {
public:
  HttpDownloadFile(char* file_name = NULL, FILE* stream = NULL)
  :file_name_(file_name), stream_(stream) {};
  ~HttpDownloadFile() {
    if (stream_) {
      fclose(stream_);
    }
    stream_ = NULL;
  }

  char* file_name_;
  FILE* stream_;
};

} // namespace http
} // namespace client
} // namespace gstd

// http file download 에 사용할 callback function
size_t callback_http_file_write(void *buffer, size_t size,
                                size_t nmemb, void *stream)
{
  size_t write_bytes = -1;
  gstd::client::http::HttpDownloadFile *out = (gstd::client::http::HttpDownloadFile *)stream;
  if(out && !out->stream_) {
//#if defined(__linux__) && (__cplusplus < 201103L) // c++98
#if defined(__linux__)
    out->stream_ = fopen(out->file_name_, "wb");
    if (!out->stream_) return -1;
#else // c++11 or windows
    errno_t error_value = fopen_s(&(out->stream_), out->file_name_, "wb");
    if (error_value > 0) return -1;
#endif
  }
  return fwrite(buffer, size, nmemb, out->stream_);
}

// CURL 호출시 결과 문자열을 저장하기 위한 콜백함수
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct curl_fetch_data *s)
{
  size_t new_len = s->length_ + size * nmemb;
  char* buffer_ptr = NULL;
  // realloc 에서 null 이 리턴되는 경우 이전 메모리 주소정보가 사라져 메모리릭이 발생될 수 있다.
  buffer_ptr = (char*)realloc(s->data_, new_len + 1);
  if (buffer_ptr == NULL) {
    free(s->data_);
    s->data_ = NULL;
    s->length_ = 0;
    return 0;
  } else {
    s->data_ = buffer_ptr;
  }
  memcpy(s->data_ + s->length_, ptr, size*nmemb);
  s->data_[new_len] = '\0';
  s->length_ = new_len;

  return size * nmemb;
}

namespace gstd {
namespace client {
namespace http {

void CurlGlobalInit()
{
  curl_global_init(CURL_GLOBAL_ALL);
  // SSL_library_init() 을 포함한다.
  // 사용후에 반드시 ssl 에서 할당된 메모리도 반환 해야 한다.
}

void CurlGlobalExit()
{
  curl_global_cleanup();
  //! CRYPTO_malloc 으로 할당된 ssl 메모리 반환.

  CRYPTO_cleanup_all_ex_data();
  //! thread-safe
  ENGINE_cleanup();
  ERR_remove_state(0);

  //! global
  ERR_free_strings();
  //COMP_zlib_cleanup();
  OBJ_NAME_cleanup(-1);
  EVP_cleanup();
  //! SSL_library_init() 에 할당된 마지막 두개의 메모리블럭 제거
  sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
}

HttpClient::HttpClient()
{
#if defined(__linux__) && (__cplusplus < 201103L)
  curl_ = NULL;
  post_ = NULL;
  last_ = NULL;
  header_ = NULL;  

  timeout_ = genum::http::value::kTimeout;
  retries_ = genum::http::value::kRetries;

  response_code_ = genum::http::kOK;
  connection_time_ = 0.0;
  download_speed_ = 0.0;
  download_page_size_ = 0.0;

  is_force_post_ = false;
  is_debug_ = false;
  is_verify_ = true;      // SSL 인증서 검증
#endif
  curl_ = curl_easy_init();
  if (curl_) {
    SetCallback_();
  } else {
    last_error_message_.assign("failed CURL allocation");
  }
}

HttpClient::~HttpClient()
{
  Clear();
  if (curl_) {
    curl_easy_cleanup(curl_);
    curl_ = NULL;
  }
  if (data_.data_) {
    free(data_.data_);
    data_.data_ = NULL;
    data_.length_ = 0;
  }
  //! 스레드별로 호출되어야 하는 cleanup 함수
  CRYPTO_cleanup_all_ex_data();
  ERR_remove_state(0);
}

// http client 에서 사용중인 curl 의 버전정보를 가져온다.
std::string HttpClient::GetCurlVersion()
{
  curl_version_info_data* data = curl_version_info(CURLVERSION_NOW);
  return std::string(data->version);
}

// http client 의 curl 에서 사용중인 ssl library 명칭과 버전을 가져온다.
std::string HttpClient::GetSSLVersion()
{
  curl_version_info_data* data = curl_version_info(CURLVERSION_NOW);
  return std::string(data->ssl_version);
}

void HttpClient::Clear()
{
  ClearLibcurl_();
  ClearResponseValue_();
}

// private

// CURL 사용에 사용되었던 메모리를 초기화 한다.
// CURL 구조체는 초기화후 재생성하며 post 와 header 구조체는 초기화 한다.
void HttpClient::ClearLibcurl_()
{
  if (curl_) {
    curl_easy_cleanup(curl_);
    curl_ = NULL;
    curl_ = curl_easy_init();
    if (curl_)   SetCallback_();
  }
  if (post_) {
    // fromadd를 이용 post데이터 추가시 firstitem(_post) lastitem(_last) 는 
    // formpost chain 이므로 firstitem으로 free 후 first와 last 둘다 
    // NULL처리하여 메모리의 잘못된접근 방지
    curl_formfree(post_);
    post_ = NULL;
    last_ = NULL;
  }
  if (header_) {
    curl_slist_free_all(header_);
    header_ = NULL;
  }
}

// HTTP response 에 대한 멤버변수들을 초기화 한다.
void HttpClient::ClearResponseValue_()
{
  response_code_ = 0;
  connection_time_ = 0;
  download_speed_ = 0;
  download_page_size_ = 0;
  last_error_code_ = common::error::kNoError;
  // curl reponse buffer 를 초기화한다.
  ResetResultBuffer_(0);
}

// 생성된 CURL 구조체에 콜백함수와 콜백함수에 데이터를 쓰기위한 구조체를 설정 한다
bool HttpClient::SetCallback_(bool is_file)
{
  if (curl_) {
    if (!is_file) { // 파일이 아닌 경우
      if (curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writefunc) != CURLE_OK) {
        last_error_message_.assign("failed setopt [CURLOPT_WRITEFUNCTION]");
        return false;
      }
      if (curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &data_) != CURLE_OK) {
        last_error_message_.assign("failed setopt [CURLOPT_WRITEDATA]");
        return false;
      }
    } else { // 파일 다운로드 인 경우
      if (curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, callback_http_file_write) != CURLE_OK) {
        last_error_message_.assign("failed setopt [CURLOPT_WRITEFUNCTION]");
        return false;
      }
      // file download CURLOPT_WRITEDATA 콜백데이터 설정은 함수 호출시 파일명 구조체로 설정
    }
  } else {
    last_error_message_.assign("CURL not allocated");
    return false;
  }
  return true;
}

// CURL 결과 버퍼를 초기화한다.
bool HttpClient::ResetResultBuffer_(int32_t size)
{
  bool result = true;
  if (data_.data_) {
    free(data_.data_);
    data_.data_ = NULL;
    data_.length_ = 0;
  }
  data_.data_ = (char*)malloc(size + 1);
  //data_.len = size+1;
  if (!data_.data_) {
    last_error_message_.assign("memory allocation failed");
    result = false;
  } else {
    memset(data_.data_, 0x00, size + 1);
  }
  return result;
}

// HttpClient 에 설정된 기본정보를 CURL object 에 설정한다.
// 실패시 false 를 리턴하며, CURL 에러메시지를 버퍼에 추가한다.
bool HttpClient::SetBase_()
{
  CURLcode curl_code = CURLE_OK;
  if (!url_.empty()) {
    if ((curl_code = curl_easy_setopt(curl_, CURLOPT_URL, url_.c_str())) != CURLE_OK) {
      last_error_message_ = "CURLOPT_URL setopt > "
        + std::string(curl_easy_strerror(curl_code));
      return false;
    }
  }
  if ((curl_code = curl_easy_setopt(curl_, CURLOPT_TIMEOUT, timeout_)) != CURLE_OK) {
    last_error_message_ = "CURLOPT_TIMEOUT setopt > "
      + std::string(curl_easy_strerror(curl_code));
    return false;
  }
  if ((curl_code = curl_easy_setopt(curl_, CURLOPT_FAILONERROR, false)) != CURLE_OK) {
    last_error_message_ = "CURLOPT_FAILONERROR setopt > "
      + std::string(curl_easy_strerror(curl_code));
    return false;
  }
  if ((curl_code = curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1)) != CURLE_OK) {
    last_error_message_ = "CURLOPT_NOSIGNAL setopt > "
      + std::string(curl_easy_strerror(curl_code));
    return false;
  }
  // 디버깅 모드 설정
  if (is_debug()) {
    if ((curl_code = curl_easy_setopt(curl_, CURLOPT_VERBOSE, true)) != CURLE_OK) {
      last_error_message_ = "CURLOPT_VERBOSE setopt > "
        + std::string(curl_easy_strerror(curl_code));
      return false;
    }
  }
  // SSL verification pass
  if (!is_verify()) {
    if ((curl_code = curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, false)) != CURLE_OK) {
      last_error_message_ = "CURLOPT_SSL_VERIFYHOST setopt > "
        + std::string(curl_easy_strerror(curl_code));
      return false;
    }
    if ((curl_code = curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, false)) != CURLE_OK) {
      last_error_message_ = "CURLOPT_SSL_VERIFYPEER setopt > "
        + std::string(curl_easy_strerror(curl_code));
      return false;
    }
  }
  return true;
}

// 헤더 리스트에 추가된 헤더정보를 CURL object 에 설정한다.
// 실패시 false 를 리턴하며, CURL 에러메시지를 버퍼에 추가한다.
bool HttpClient::SetHeader_()
{
  CURLcode curl_code = CURLE_OK;
  if (header_) {
    if ((curl_code = curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, header_)) == CURLE_OK) {
      if (is_debug()) std::cout << "HttpClient set header succeed" << std::endl;
    } else { // failed set header
      last_error_message_ = "CURLOPT_HTTPHEADER setopt > "
        + std::string(curl_easy_strerror(curl_code));
      return false;
    }
  } // 추가된 헤더없는경우 true
  return true;
}

// post form data 에 추가된 정보를 CURL object 에 설정한다.
// 실패시 false 를 리턴하며, CURL 에러메시지를 버퍼에 추가한다.
bool HttpClient::SetPost_()
{
  CURLcode curl_code = CURLE_OK;
  if (post_) {
    // post form 정보가 있을 경우 설정.
    // 이전에 post fields 
    if ((curl_code = curl_easy_setopt(curl_, CURLOPT_HTTPPOST, post_)) == CURLE_OK) {
      if (is_debug()) std::cout << "HttpClient set post succeed" << std::endl;
    } else {
      // failed set post
      last_error_message_ = "Failed CURLOPT_HTTPPOST" 
        + std::string(curl_easy_strerror(curl_code));
      return false;
    }
  }
  return true;
}

// CURL response 값들을 멤버변수에 저장한다.
void HttpClient::SetResponseValue_()
{
  curl_easy_getinfo(curl_, CURLINFO_CONNECT_TIME, &connection_time_);
  curl_easy_getinfo(curl_, CURLINFO_SPEED_DOWNLOAD, &download_speed_);
  download_speed_ *= 8;
  curl_easy_getinfo(curl_, CURLINFO_HTTP_CODE, &response_code_);
  curl_easy_getinfo(curl_, CURLINFO_SIZE_DOWNLOAD, &download_page_size_);
}

// HttpClient 에 설정된 기본정보와 헤더, POST 등을 설정한다.
// 각 Method 별 추가적용이 필요한부분은 Method 에 맞게 설정 한다.
// 
bool HttpClient::Request_(ReqeustMethod method)
{
  bool result = false;
  CURLcode curl_code = CURLE_OK;
  HttpDownloadFile download_file;
  // url, 기본 옵션등을 CURL 에 설정
  if (!SetBase_()) {
    // SetBase_ 내 에러메시지는 함수 내부에서 처리.
    set_last_error_code(common::error::kOther);
    return result;
  }
  if (!SetHeader_()) {
    set_last_error_code(common::error::kOther);
    return result;
  }
  // HTTP Post method 에 맞는 설정을 진행한다.
  if (method == kPost) {
    // post form data 를 CURL 에 설정한다.
    // post form data 를 설정하거나 post fields 에 추가가 되어야만 Http method 가
    // curl_easy_perform 에서 POST로 동작한다
    if (!SetPost_()) {
      set_last_error_code(common::error::kOther);
      return result;
    }
  } else if (method == kGet && download_file_name_.length() > 0) {
    // write callback function 을 파일 다운로드 용으로 변경
    if (!SetCallback_(true)) {
      set_last_error_code(common::error::kOther);
      return result;
    }
    download_file.file_name_ = const_cast<char*>(download_file_name_.c_str());
    if ((curl_code = curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &download_file)) != CURLE_OK) {
      set_last_error_code(common::error::kOther);
      last_error_message_ = "Failed CURLOPT_WRITEDATA" 
        + std::string(curl_easy_strerror(curl_code));
      return result;
    }
  } 
  curl_code = curl_easy_perform(curl_);
  if (curl_code == CURLE_OK) {
    // 결과성공시 result code 등을 결과 설정.
    if (is_debug()) std::cout << "http request succeed" << std::endl;
    SetResponseValue_();
    result = true;
  } else {
    last_error_message_ = "failed curl_easy_perform > " + std::string(curl_easy_strerror(curl_code));
    if (curl_code == CURLE_OPERATION_TIMEDOUT) {
      set_last_error_code(common::error::kTimeout);
    } else if (curl_code == CURLE_COULDNT_CONNECT) {
      set_last_error_code(common::error::kCouldntConnect);
    } else {
      set_last_error_code(common::error::kOther);
    }
  }
  return result;
}

// public

bool HttpClient::SetTcpKeepAlive()
{
  if (curl_) {
    if (curl_easy_setopt(curl_, CURLOPT_TCP_KEEPALIVE, 1L) == CURLE_OK)   return true;
  }
  return false;
}
// 변수로받는 문자열을 CURL post fields 에 추가한다.
bool HttpClient::SetPostField(const char* contents)
{
  if (curl_) {
    if (curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, contents) == CURLE_OK)   return true;
  }
  return false;
}

// key, value  형식의 post form data 를 추가한다.
bool HttpClient::AddPost(const char* name, const char* contents)
{
  curl_formadd(&post_, &post_, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, contents, CURLFORM_END);
  if (post_) return true;
  if (is_debug()) std::cout << "HttpClient::AddPost => error add post : " << errno << std::endl;
  return false;
}

// array 형식의 post form data 를 추가한다. 
#if defined(_WIN32) || (__cplusplus >=201103L)
bool HttpClient::AddPostArray(const char* name, std::initializer_list<std::string> list_contents)
#else
bool HttpClient::AddPostArray(const char* name, std::list<std::string> list_contents)
#endif
{
  if (list_contents.size() > 0) {
    int array_size = list_contents.size() + 1;
    struct curl_forms* forms = new struct curl_forms[array_size];
    if (forms) {
      int i = 0;      
#if defined(_WIN32) || (__cplusplus >=201103L)
      for (auto it = list_contents.begin(); it != list_contents.end(); it++, i++) {
#else
      std::list<std::string>::iterator it = list_contents.begin();
      for (; it != list_contents.end(); it++, i++) {      
#endif
        forms[i].option = CURLFORM_COPYCONTENTS;
        forms[i].value = it->c_str();
      }
      forms[i].option = CURLFORM_END;
      curl_formadd(&post_, &last_, CURLFORM_COPYNAME, name, CURLFORM_ARRAY, forms, CURLFORM_END);
      delete[] forms;
      if (post_) {
        if (is_debug()) std::cout << "HttpClient::AddPostArray post is not null" << std::endl;
        return true;
      }
    }
  }
  if (is_debug()) std::cout << "HttpClient::AddPostArray => error add post array : " << errno << std::endl;
  return false;
}

// HTTP header 를 추가한다.
bool HttpClient::AddHeader(const char* header)
{
  header_ = curl_slist_append(header_, header);
  if (header_)   return true;
  // 에러로깅 추가
  return false;
}

// HTTP header 를 초기화 한다.
void HttpClient::ClearHeader()
{
  if (header_) {
    curl_slist_free_all(header_);
    header_ = NULL;
  }
}

// 가장최근 HTTP response 에 대해 초기화한다.
void HttpClient::FreeResult()
{
  ClearResponseValue_();
}

// HTTP response code 에 대한 문자열을 리턴 한다.
std::string HttpClient::GetResponseCodeString()
{
  std::string result;
  switch (static_cast<ResponseCode>(response_code())) {
    case kContinue :
      result.assign("continue");
      break;
    case kOK :
      result.assign("ok");
      break;
    case kCreate :
      result.assign("created");
      break;
    case kBadRequest :
      result.assign("bad request");
      break;
    case kUnAuthorized :
      result.assign("unauthorized");
      break;
    case kNotFound :
      result.assign("not found");
      break;
    case kInternalServerError :
      result.assign("internal server error");
      break;
    case kRequestTimeout :
      result.assign("timeout");
      break;
    case kNoContent :
      result.assign("no content");
      break;
    case kPayloadTooLarge :
      result.assign("payload too large");
      break;
    case kURITooLong:
      result.assign("URI too long");
      break;
    case kMethodNotAllowed:
      result.assign("method not allowed");
      break;
    default:
      result.assign("etc error");
      break;
  }
  return result;
}

// public executor

// HTTP Post 함수
// post form data 가 추가되어 있지 않거나, post fields 가 추가되지 않은 경우 Get 으로 동작
// CURL이 정상 실행될 경우 true 이며, HTTP 에서의 성공 실패는 외부 application에서 
// response code 로 판별.
bool HttpClient::Post()
{
  bool result = false;
  if (curl_) {
    result = Request_(kPost);
  }
  return result;
}

// HTTP Get 함수
// Post form 이 설정되어있어도 CURL object 에 추가하지않고 Get 으로 호출되도록 실행
// CURL이 정상 실행될 경우 true 이며, HTTP 에서의 성공 실패는 외부 application에서 
// response code 로 판별.
bool HttpClient::Get()
{
  bool result = false;
  if (curl_) {
    result = Request_(kGet);
  }
  return result;
}

} // namespace http
} // namespace client
} // namespace gstd