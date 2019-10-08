#ifndef GSTD_SIMPLE_ZIP_H__
#define GSTD_SIMPLE_ZIP_H__

#if defined(_WIN32)
#include <Windows.h>
#endif
#include <stdint.h>
#include <string>
#include <list>
#include <unordered_map>
#include <zip.h>
#include <unzip.h>
#include <iowin32.h>

// unzip 참조 https://github.com/luvit/zlib/blob/master/contrib/minizip/unzip.h
// zip 참조   https://github.com/luvit/zlib/blob/master/contrib/minizip/zip.h

// @author ykjo
// minizip(with  zlib) 을 이용한 zip, unzip 동작 클래스
// simple unzip 추가 (2018-09-04)
// windows 용으로 개발되어 사용
// TODO: Linux 에서도 사용가능하도록 추가
// TODO: 압축 및 압축해제시 암호입력 기능 추가

#define UNZ_SUCCEED(X) \
  (UNZ_OK == X)
#define UNZ_FAILED(X) \
  (UNZ_OK != X)
#define UNZ_ENDED(X) \
  (UNZ_END_OF_LIST_OF_FILE == X)
#define UNZ_NOT_ENDED(X) \
  (UNZ_END_OF_LIST_OF_FILE != X)

namespace gstd {

namespace genum {
namespace zip {

// 파라미터 체크모드에 사용할 상수
enum CheckType {
  kZipType = 0,   // zip 모드 파라미터 체크
  kUnZipType = 1  // unzip 파라미터 체크
  // 기타 추가 타입 정의
};

enum CheckFile {
  // 디렉토리
  kDirectory = 0,
  // 비어있는 파일 또는 디렉토리
  kEmpty = 1,
  // 존재하는 파일
  kExistFile = 2,
  // 체크 실패
  kFailed = 3
};

} // namespace zip
} // namespace genum

class Entry;

class Entry {
public:
  Entry(std::string name)
  {
    is_directory_ = false;
    name_.assign(name);
    entry_map_.clear();
    absolute_path_.clear();
    relative_path_.clear();
  }
  ~Entry(){}
  
  bool is_directory_;         // is a directory (if not the file)
  std::string name_;          // file or directory name
  std::string absolute_path_; // ex: C:\path1\file      (current path => path1)
  std::string relative_path_; // ex: path1\file         (current path => path1)
  std::unordered_map<std::string, Entry> entry_map_;//  (entries to compress)
};

class SimpleZip {
 public:
  SimpleZip();
  ~SimpleZip();

  bool over_write() const { return over_write_; }
  uint32_t error_history_size() const { return error_history_.size(); }
  std::vector<std::string> error_history() const {return error_history_;}
  std::string error_message() const {return error_message_;}
  std::string output_file() const {return output_file_;}
  void set_over_write(bool over_write) { over_write_ = over_write; }
  void set_output_file(std::string output_file) {output_file_.assign(output_file);}
  void set_unzip_file(std::string unzip_file) {unzip_file_.assign(unzip_file);}
  void set_unzip_path(std::string unzip_path) {
    std::string::size_type pos;
    if ((pos = unzip_path.find_last_of("/\\")) == std::string::npos) {
      unzip_path_ = unzip_path + "\\";
    }
  }

  // 설정된 정보로 파일압축을 진행하는 함수
  uint32_t Zip();
  // 설정된 정보로 압축파일 해제를 진행하는 함수.
  // target_zip_file 파라미터를 안줄 경우 class 에 set 된 압축파일을 해제한다.
  uint32_t UnZip(std::string target_zip_file = "");
  // full_path 에 해당하는 경로의 폴더를 생성한다.
  // 문자열의 마지막값이 / or \ 가 아닌 경우 파일로 간주하고 그아래까지 폴더를 생성한다.
  void MakeDirectory(std::string full_path);

  // 압축 대상 파일을 추가하는 함수
  void AppendTarget(std::string target);
  // 내부 설정값 초기화 함수
  void Clear();
  // 압축 대상 모든 Entry 에 대해서 콘솔 출력하는 함수
  void PrintAllEntry();

  // 해당 path 값이 폴더인지 체크하는 함수
  // 폴더인 경우 true, 없거나 파일인 경우 false 를 리턴한다.
  // false 인 경우 exception_value 에 오류값(gstd::genum::zip::CheckFile)을 저장한다.
  bool IsDirectory(std::string path, uint32_t& exception_value);

 private:
   // common private function
  void PrintAllEntry_(Entry* entry, int depth);
  uint32_t CheckParameter_(uint32_t check_mode);

  // zip private function
  uint32_t Zip_(zipFile zip_file);
  uint32_t ZipEntry_(Entry* entry, zipFile zip_file);
  uint32_t ZipEntryFile_(Entry* entry, zipFile zip_file);
  uint32_t ZipEntryWriteFile_(Entry* entry, zipFile zip_file);
  uint32_t CheckEntry_();
  uint32_t CheckEntry_(Entry* entry);

  // unzip private function
  uint32_t UnZip_(unzFile unzip_file);
  int32_t WriteFileFromZipFile_(unzFile unzip_file, std::string output_file);

 private:
  // unzip 실행시 압축 해제 경로에 파일이 있을 경우 덮어쓰기 유무
  bool                      over_write_;
  std::string               error_message_;
  // zip 실행시 출력 압축 파일명
  std::string               output_file_;
  // unzip 실행시 압축을 해제할 경로
  std::string               unzip_path_;
  // unzip 실행시 압축을 해제할 파일명
  std::string               unzip_file_;
  std::vector<std::string>  error_history_;
  std::list<Entry>          target_list_;
};

} // namespace gstd

#endif  // GSTD_SIMPLE_ZIP_H__