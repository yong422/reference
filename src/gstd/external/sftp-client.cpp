
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <curl/curl.h>
#include "sftp-client.h"

/**
  참조
  sftp upload https://curl.haxx.se/libcurl/c/sftpuploadresume.html
  sftp get https://curl.haxx.se/libcurl/c/sftpget.html
*/
struct FtpFile {
  const char* file_name_;
  FILE* stream_;
};

#if defined(_WIN32)
// 파일 업로드를 위한 local file read 콜백함수
static size_t callback_file_read(void *ptr, size_t size, size_t nmemb, void *stream)
{
  DWORD read_bytes = 0;
  BOOL result = ReadFile(static_cast<HANDLE>(stream), ptr, size * nmemb, &read_bytes, NULL);
  //fprintf(stdout, "%s : %d", (char*)ptr, read_bytes);
  return read_bytes;
}
#else
static size_t callback_file_read(void *ptr, size_t size, size_t nmemb, void *stream)
{
  FILE *file_ptr = static_cast<FILE*>(stream);  
  size_t read_bytes = 0;
 
  if(ferror(file_ptr))
    return CURL_READFUNC_ABORT;
 
  read_bytes = fread(ptr, size, nmemb, file_ptr) * size;
 
  return read_bytes;
}
#endif
// 파일 다운로드를 위한 local file write 콜백함수
static size_t callback_file_write(void *buffer, size_t size,
                                  size_t nmemb, void *stream)
{
  struct FtpFile *out = (struct FtpFile *)stream;
  if(out && !out->stream_) {
#if _MSC_VER >= 1900
    errno_t error_value = fopen_s(&(out->stream_), out->file_name_, "wb");
    if (error_value > 0) return -1;
#else
    out->stream_ = fopen(out->file_name_, "wb");
    if (!out->stream_) return -1;
#endif
  }
  return fwrite(buffer, size, nmemb, out->stream_);
}

// curl error code 를 sftp code 로 변경.
static gstd::genum::sftp::SFtpCode change_curl_error_to_genum(CURLcode code)
{
  gstd::genum::sftp::SFtpCode result = gstd::genum::sftp::kSuccess;
  switch (code) {
    case CURLE_REMOTE_FILE_NOT_FOUND:
      result = gstd::genum::sftp::kEmptyFile;
      break;
    case CURLE_REMOTE_ACCESS_DENIED:
      // permission denied 
      result = gstd::genum::sftp::kAccessDenied;
      break;
    case CURLE_SSH:
      // permission denied 
      result = gstd::genum::sftp::kErrorDirectory;
      break;
    case CURLE_LOGIN_DENIED:
      result = gstd::genum::sftp::kFailedAuth;
      break;
  };
  return result;
}

namespace gstd {

void SFtpClient::GlobalInitialize()
{
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

void SFtpClient::GlobalCleanup()
{
  curl_global_cleanup();
  //! global
  //ERR_free_strings();
  //COMP_zlib_cleanup();
  //OBJ_NAME_cleanup(-1);
  //EVP_cleanup();
  //! SSL_library_init() 에 할당된 마지막 두개의 메모리블럭 제거
  //sk_SSL_COMP_free( SSL_COMP_get_compression_methods() );
}

SFtpClient::SFtpClient(std::string host, int32_t port)
{
  curl_ = NULL;
  Clear();
  host_.assign(host);
  port_ = port;
}

SFtpClient::~SFtpClient()
{
  //
}

void SFtpClient::Clear()
{
  is_debug_ = false;
  port_ = 0;
  host_.clear();
  user_id_.clear();
  user_passwd_.clear();
}

std::string SFtpClient::CreateUrl(std::string target) const
{
  std::string url = "sftp://" + host_ + target;
  return url;
}

#if 1
/**
  sftp download, upload 등 기능에 맞는 curl option 을 설정하는 함수
*/
bool SFtpClient::SetOption(genum::sftp::SFtpType sftp_type)
{
  // default option
  bool result = true;
  std::string auth_info = user_id_ + ":" + user_passwd_;
  curl_easy_setopt(curl_, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
  curl_easy_setopt(curl_, CURLOPT_USERPWD, auth_info.c_str());
  curl_easy_setopt(curl_, CURLOPT_PORT, port_);
  curl_easy_setopt(curl_, CURLOPT_USE_SSL, CURLUSESSL_TRY);
  if (is_debug()) curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L);
  switch (sftp_type) {
    case genum::sftp::kGet :
    case genum::sftp::kGetFile :
    case genum::sftp::kDownload :
      curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, callback_file_write);
      break;
    case genum::sftp::kAppendUpload:
      // 기존 파일이 존재하는 경우 파일에 덮어쓰기로 추가하는 경우
      curl_easy_setopt(curl_, CURLOPT_APPEND, 1L);
    case genum::sftp::kUpload:
    case genum::sftp::kUpdate:
      curl_easy_setopt(curl_, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(curl_, CURLOPT_READFUNCTION, callback_file_read);
      break;
  }
  return result;
}

/**
  원격지 파일을 다운로드 하는 함수
*/
genum::sftp::SFtpCode SFtpClient::GetFile(std::string remote_file, std::string write_file)
{
  genum::sftp::SFtpCode result = genum::sftp::kSuccess;
  if (!curl_) curl_ = curl_easy_init(); 
  if (curl_) {
    std::string url = CreateUrl(remote_file);
    if (!url.empty()) {
      CURLcode curl_result = CURLE_OK;
      FtpFile ftp_file = {write_file.c_str(), NULL};
      curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &ftp_file);

      SetOption(genum::sftp::kDownload);
 
      curl_result = curl_easy_perform(curl_);

      if (CURLE_OK != curl_result) {
        last_error_.assign(curl_easy_strerror(curl_result));
        //fprintf(stdout, "%s : %d\n", last_error_.c_str(), curl_result);
        result = change_curl_error_to_genum(curl_result);
      }
      if (ftp_file.stream_) fclose(ftp_file.stream_);
    }
    curl_easy_cleanup(curl_);
    curl_ = NULL;
  } else {
    result = genum::sftp::kErrorUrl;
  }
  return result;
}

genum::sftp::SFtpCode SFtpClient::UploadFile(std::string local_file, std::string remote_file)
{
  genum::sftp::SFtpCode result = genum::sftp::kSuccess;
  if (!curl_) curl_ = curl_easy_init();//curl_ = curl_easy_init();
  if (curl_) {
    std::string url = CreateUrl(remote_file);
    if (!url.empty()) {
      CURLcode curl_result = CURLE_OK;
      TCHAR file_buffer[MAX_PATH] = {0,};
      GetFullPathName(local_file.c_str(), sizeof(file_buffer), file_buffer, NULL);
      // 파일오픈
      HANDLE handle_local_file = CreateFile(file_buffer, GENERIC_READ/*읽기권한*/, 0, NULL,
                                            OPEN_EXISTING/*파일이 있을경우 오픈*/,
                                            FILE_ATTRIBUTE_NORMAL, NULL);
      if (handle_local_file == INVALID_HANDLE_VALUE) {
        result = genum::sftp::kOpenFailed;
      } else {  // 파일오픈 성공
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_READDATA, handle_local_file);

        SetOption(genum::sftp::kUpload);

        curl_result = curl_easy_perform(curl_);

        if (CURLE_OK != curl_result) {
          last_error_.assign(curl_easy_strerror(curl_result));
          result = change_curl_error_to_genum(curl_result);
        }
        CloseHandle(handle_local_file);
      }
    }
    curl_easy_cleanup(curl_);
    curl_ = NULL;
  } else {
    result = genum::sftp::kErrorUrl;
  }
  return result;
}

#else
genum::SFtpCode SFtpClient::GetFile(std::string remote_file, std::string write_file)
{
  genum::SFtpCode result = genum::kSuccess;
  CURL* curl = curl_easy_init();
  if (curl) {
    std::string url = CreateUrl(remote_file);
    if (!url.empty()) {
      CURLcode curl_result = CURLE_OK;
      FtpFile ftp_file = {write_file.c_str(), NULL};
      std::string auth_info = user_id_ + ":" + user_passwd_;
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_file_write);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftp_file);
      curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
      curl_easy_setopt(curl, CURLOPT_USERPWD, auth_info.c_str());
      curl_easy_setopt(curl, CURLOPT_PORT, port_);
      curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
      if (is_debug()) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
 
      curl_result = curl_easy_perform(curl);

      if (CURLE_OK != curl_result) {
        last_error_.assign(curl_easy_strerror(curl_result));
        //fprintf(stdout, "errno : %d  error string : %s\n", curl_result, last_error_.c_str());
        result = change_curl_error_to_genum(curl_result);
      }
      if (ftp_file.stream_) {
        fclose(ftp_file.stream_); /* close the local file */ 
      }
    }
    curl_easy_cleanup(curl);
  } else {
    result = genum::sftp::kErrorUrl;
  }
  return result;
}

genum::SFtpCode SFtpClient::UploadFile(std::string local_file, std::string remote_file)
{
  genum::SFtpCode result = genum::kSuccess;
  CURL* curl = curl_easy_init();
  if (curl) {
    std::string url = CreateUrl(remote_file);
    if (!url.empty()) {
      CURLcode curl_result = CURLE_OK;
      TCHAR file_buffer[MAX_PATH] = {0,};
      GetFullPathName(local_file.c_str(), sizeof(file_buffer), file_buffer, NULL);
      // 파일오픈
      HANDLE handle_local_file = CreateFile(file_buffer, GENERIC_READ/*읽기권한*/, 0, NULL,
                                            OPEN_EXISTING/*파일이 있을경우 오픈*/,
                                            FILE_ATTRIBUTE_NORMAL, NULL);
      if (handle_local_file == INVALID_HANDLE_VALUE) {
        fprintf(stdout, "파일오픈 실패요 %d\n", GetLastError());
      } else {  // 파일오픈 성공
        std::string auth_info = user_id_ + ":" + user_passwd_;
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
 
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, callback_file_read);
        curl_easy_setopt(curl, CURLOPT_READDATA, handle_local_file);
        curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
        curl_easy_setopt(curl, CURLOPT_USERPWD, auth_info.c_str());
        curl_easy_setopt(curl, CURLOPT_APPEND, 1L);
        curl_easy_setopt(curl, CURLOPT_PORT, port_);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
        if (is_debug()) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_result = curl_easy_perform(curl);

        if (CURLE_OK != curl_result) {
          last_error_.assign(curl_easy_strerror(curl_result));
          result = change_curl_error_to_genum(curl_result);
        }
        CloseHandle(handle_local_file);
      }
    }
    curl_easy_cleanup(curl);
  } else {
    result = genum::sftp::kErrorUrl;
  }
  return result;
}
#endif


} // namespace gstd


#if defined(_TEST_SFTP_CLIENT) && defined(_WIN32)
/**
  sftp-client 테스트 코드
  
*/
int _tmain(void)
{
#if defined(_DEBUG)
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif

  gstd::SFtpClient::GlobalInitialize();
  gstd::genum::sftp::SFtpCode sftp_code;
  gstd::SFtpClient sftp_client("10.222.223.155", 22);
  sftp_client.set_is_debug(true);
  sftp_client.set_user_id("jo");
  sftp_client.set_user_passwd("whdydrb11");
  if ((sftp_code= sftp_client.GetFile("/home/jo/test.cpp", "test.cpp")) 
      == gstd::genum::sftp::kSuccess) {
    fprintf(stdout, "file get success\n");
  } else {
    fprintf(stdout, "file get failed %d\n", sftp_code);
  }

  if ((sftp_code = sftp_client.UploadFile("sftp-client.zip", "/home/jo/sftp-client.zip"))
    == gstd::genum::sftp::kSuccess) {
    fprintf(stdout, "file upload success\n");
  } else {
    fprintf(stdout, "file upload failed %d\n", sftp_code);
  }

  gstd::SFtpClient::GlobalCleanup();
#if defined(_DEBUG)
   _CrtDumpMemoryLeaks();
#endif
  Sleep(5000);
  return 0;
}
#endif