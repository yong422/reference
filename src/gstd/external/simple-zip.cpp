#include "stdafx.h"

#if defined(_WIN32)
#include <Shlwapi.h>
#endif

#include <iostream>
#include <fstream>
#include <errno.h>
#include "simple-zip.h"
namespace gstd {


// 압축할 파일의 정보에 현재 시간값을 설정
// 참조: zlib/contrib/minizip/minizip.c
#ifdef _WIN32
ULONG SetFileTime(char *f, tm_zip *tmzip, ULONG* dt)
{
  int ret = 0;
  FILETIME ftLocal;
  HANDLE hFind;
  WIN32_FIND_DATAA ff32;
  hFind = FindFirstFileA(f,&ff32);

  if (hFind != INVALID_HANDLE_VALUE) {
    FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
    FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
    FindClose(hFind);
    ret = 1;
  }
  return ret;
}
#endif


SimpleZip::SimpleZip()
{
  // con
  Clear();
}

SimpleZip::~SimpleZip()
{
  // des
  Clear();
}

void SimpleZip::Clear()
{
  output_file_.clear();
  target_list_.clear();
  error_history_.clear();
  unzip_file_.clear();
  unzip_path_.clear();
  set_over_write(false);
}


// 압축대상을 추가한다.
void SimpleZip::AppendTarget(std::string target)
{
  Entry entry(target);
  entry.relative_path_.assign(target);
  target_list_.push_back(entry);
}


// 동작방식별 설정된 변수값이 적절한지에 대한 판단하는 함수
// 모든 동작함수는 해당함수로 변수체크를 하도록 정의
uint32_t SimpleZip::CheckParameter_(uint32_t check_mode)
{
  uint32_t result = 1;
  if (check_mode == genum::zip::kZipType) {  // zip
    if (output_file_.empty()){
      result = 0;
      error_message_ = "empty output file";
      error_history_.push_back(error_message_);
    } else if(target_list_.empty()) {
      result = 0;
      error_message_ = "empty target file";
      error_history_.push_back(error_message_);
    }
  } else if (check_mode == genum::zip::kUnZipType) { // unzip
    if (unzip_file_.empty()) {
      result = 0;
      error_message_ = "empty unzip file";
      error_history_.push_back(error_message_);
    }
  } else {
    // 이외 체크타입은 없으며 실패처리.
    error_message_ = "unsupported check";
    result = 0;
  }
  return result;
}


// 설정된 압축파일과 압축될 파일리스트를 이용하여 압축을 진행한다.
// 1. 설정된 변수 정상여부
// 2. Entry 체크 (디렉토리 하위 파일 및 하위디렉토리 전체 체크)
// 3. zip 파일 오픈
// 4. 체크된 Entry 전체 zip파일에 추가
uint32_t SimpleZip::Zip()
{
  uint32_t result = 0;
  if (!CheckParameter_(genum::zip::kZipType)) return 0;

  result += CheckEntry_();
  zlib_filefunc_def ffunc;
  fill_win32_filefunc(&ffunc);
  zipFile zip_file = zipOpen2(output_file_.c_str(), 0, NULL, &ffunc);

  if (zip_file != NULL && result > 0) {
    // CheckEntry 로 확인된 압축대상 파일이 존재하는 경우 압축 진행
    result += Zip_(zip_file);
    zipClose(zip_file,NULL);
  } else {
    error_message_ = "zipOpen failed";
    error_history_.push_back(error_message_);
  }

  return result;
}

// 미리 체크된 Entry 에 대해서 zip file 에 추가한다.
uint32_t SimpleZip::Zip_(zipFile zip_file)
{
  uint32_t result = 0;
  if (!zip_file) return NULL;
  
  for (auto it = target_list_.begin() ;it != target_list_.end() ;it++) {
    result += ZipEntry_(&(*it), zip_file);
  }
  return result;
}


// zip file 에 entry를 추가한다.
// directory 인 경우 하위파일을 체크하도록 재귀함수 호출.
// file 인경우 file write 함수 호출.
uint32_t SimpleZip::ZipEntry_(Entry* entry, zipFile zip_file)
{
  uint32_t result = 0;
  if (!entry) return 0;

  if (entry->is_directory_) {
    for(auto it = entry->entry_map_.begin() ; 
          it != entry->entry_map_.end() ; it++) {
      ZipEntry_(&(it->second), zip_file);
    }
  } else {
    result += ZipEntryFile_(entry, zip_file);
  }

  return result;
}


// File entry 에 대해서 압축파일에 추가한다.
uint32_t SimpleZip::ZipEntryFile_(Entry* entry, zipFile zip_file)
{
  uint32_t result = 0;
  zip_fileinfo* zip_info = new zip_fileinfo;
  memset((void*)&(zip_info->tmz_date), 0x00, sizeof(zip_info->tmz_date));
  zip_info->external_fa = 0;
  zip_info->internal_fa = 0;
  zip_info->dosDate = 0;
  SetFileTime(const_cast<char*>(entry->relative_path_.c_str()), &(zip_info->tmz_date), &(zip_info->dosDate));

  // zip 파일안에 압축파일을 오픈한다.
  int zip_error = zipOpenNewFileInZip3(zip_file, entry->relative_path_.c_str(),zip_info,
                                    NULL,0,NULL,0,NULL /* comment*/,
                                    Z_DEFLATED,
                                    Z_DEFAULT_COMPRESSION, 0,
                                    /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                    -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                    NULL, 0);
  if (zip_error != ZIP_OK) {
    error_message_ = "error in opening " + entry->relative_path_ + " in zipfile";
    error_history_.push_back(error_message_);
    return result;
  }
  result = ZipEntryWriteFile_(entry, zip_file);
  zipCloseFileInZip(zip_file);

  if(zip_info) delete zip_info;
  return result;
}


// 오픈된 zip file 내의 파일에 실제 파일을 쓰도록 한다.
uint32_t SimpleZip::ZipEntryWriteFile_(Entry* entry, zipFile zip_file)
{
  uint32_t result = 0;
  int size_read = 0;
  int zip_error = ZIP_OK;
  char buf[4086] = { 0, };
  char error_buffer[256] = { 0, };
  // 압축을 실행할 실제 파일을 오픈한다.
  FILE* insert_file = NULL;
  errno_t err = fopen_s(&insert_file, entry->absolute_path_.c_str(), "rb");
  if (err) {
    // read file open 실패. 로깅 후 실패 처리
    zip_error = ZIP_ERRNO;
#if _MSC_VER >= 1700
    strerror_s(error_buffer, err);
    error_message_ = "error in opening " + entry->absolute_path_ + " for reading > "
      + std::string(error_buffer);
#else
    error_message_ = "error in opening " + entry->absolute_path_ + " for reading > "
      + std::string(strerror(err));
#endif
    error_history_.push_back(error_message_);
    return result;
  }
  do {
    zip_error = ZIP_OK;
    size_read = (int)fread(buf,1,sizeof(buf),insert_file);
    if (size_read  <  sizeof(buf)){
      if (feof(insert_file) == 0) {
        error_message_ = "error in reading " + entry->relative_path_;
        error_history_.push_back(error_message_);
        zip_error = ZIP_ERRNO;
      }
    }
    if (size_read > 0) {
      // 압축파일내 파일안에 파일을 쓴다.
      zip_error = zipWriteInFileInZip(zip_file,buf,size_read);
      if (zip_error < 0) {
        error_message_ =  "error in writing " + entry->relative_path_ + " in the zipfile";
        error_history_.push_back(error_message_);
      }
    }
  } while ((zip_error == ZIP_OK) && (size_read > 0));
  ++result;
  fclose(insert_file);
  return result;
}


// 압축해제 함수
// 변수로 받은 파일 또는 모듈에 설정된 zip파일을 압축 해제한다.
// 실패시 0, 성공시 압축해제된 파일과 디렉토리의 총개수를 리턴한다.
uint32_t SimpleZip::UnZip(std::string target_zip_file)
{
  uint32_t result = 0;
  CHAR path_buffer[MAX_PATH] = { 0, };

  if (target_zip_file.size() > 0)  set_unzip_file(target_zip_file);
  if (!CheckParameter_(genum::zip::kUnZipType)) return result;

  GetCurrentDirectory(sizeof(path_buffer) - 1, path_buffer);
  unzip_file_ = std::string(path_buffer) + "\\" + unzip_file_;

  zlib_filefunc_def ffunc;
  fill_win32_filefunc(&ffunc);
  // 해제할 zip file open 한다.
  // 오픈할 수 없는 경우 NULL 을 리턴.
  unzFile unz_file = unzOpen2(unzip_file_.c_str(), &ffunc);
  if (unz_file) {
    result = UnZip_(unz_file);
    unzClose(unz_file);
    unz_file = NULL;
  } else {
#if _MSC_VER >= 1700
    error_message_ = "Failed to open zip file [ " + std::to_string(GetLastError()) + "] > "
      + unzip_file_;
#else
    error_message_ = "Failed to open zip file [ " 
      + std::to_string(static_cast<uint64_t>(GetLastError())) + "] > " + unzip_file_;
#endif
  }
  return result;
}

// private
// 오픈된 unzip file 에 대해서 압축해제를 진행 한다.
// unzip file 내 첫번재 파일부터 순환하여 모든 파일에 대하여 파일생성을 진행한다.
// 실패시 0, 성공시 압축해제된 파일과 디렉토리의 총개수를 리턴한다.
uint32_t SimpleZip::UnZip_(unzFile unzip_file) 
{
  uint32_t result = 0;
  uint32_t check_file = 0;
  // Set the current file of the zipfile to the first file.
  // return UNZ_OK if there is no problem
  int32_t unzip_status = unzGoToFirstFile(unzip_file);
  int32_t method = 0, level = 0;
  CHAR file_name_buffer[MAX_PATH] = {0,};
  unz_file_info   file_info;
  std::string file_full_path;
  std::string error_message;
  if (UNZ_SUCCEED(unzip_status)) {
    do {
      // 현재 엔트리의 파일정보를 가져온다.
      // 
      unzip_status = unzGetCurrentFileInfo(unzip_file, &file_info, file_name_buffer, 
                                          sizeof(file_name_buffer), NULL, 0, NULL, 0);
      if (UNZ_FAILED(unzip_status)) {
        // 파일정보를 가져오지 못한경우 해당 파일명으로 파일쓰기가 불가.
        // 압축파일이 깨진 경우이며 실패 처리.
#if _MSC_VER >= 1700
        error_message_ = "failed get fileinfo [" + std::to_string(unzip_status) + "]";
#else
        error_message_ = "failed get fileinfo [" + std::to_string(static_cast<uint64_t>(unzip_status)) 
                          + "]";
#endif
        result = 0;
        break;
      }
      // raw = 1 옵션을 줄경우 압축된 포맷 그대로 읽도록 파일 오픈.
      // raw = 0 인 경우 원래 포맷의 데이터를 읽도록 파일 오픈.
      // zip file copy 또는 zip file 내의 특정파일 삭제때 사용.
      unzip_status = unzOpenCurrentFile2(unzip_file, &method, &level, 0 /*raw*/);
      if (UNZ_SUCCEED(unzip_status)) {
        if (!unzip_path_.empty())
          file_full_path = unzip_path_ + std::string(file_name_buffer);
        else
          file_full_path.assign(file_name_buffer);
         
        // 해당 파일의 전체 경로의 디렉토리를 생성한다.
        MakeDirectory(file_full_path);
        if (!IsDirectory(file_full_path, check_file)) {
          if (check_file == genum::zip::kEmpty ||
            (check_file == genum::zip::kExistFile && over_write())) {
            // 압축을 해제할 파일의 경로가 비어있거나, 파일이 있으나 덮어쓰기 모드인 경우
            int32_t read_size = WriteFileFromZipFile_(unzip_file, file_full_path);
            if (read_size == -1) {
              result = 0;
              break;
            } 
#if defined(_TEST_MODE) || defined(_DEBUG)
            std::cout << "file : " << file_name_buffer << " size : " << read_size << std::endl;
#endif
          } else if (check_file == genum::zip::kExistFile && !over_write()) {
            // 압축해제 경로에 기존파일이 있으면서 덮어쓰기가 false 인 경우 에러 히스토리에 추가
            error_message = "file > " + std::string(file_name_buffer) + " exists";
            error_history_.push_back(error_message);
          } else {
#if _MSC_VER >= 1700
            error_message = "file > " + std::string(file_name_buffer) 
              + " > error > " + std::to_string(GetLastError());
#else
            error_message = "file > " + std::string(file_name_buffer) 
              + " > error > " + std::to_string(static_cast<uint64_t>(GetLastError()));
#endif
            error_history_.push_back(error_message);
          }
        }
        // 읽기를 끝낸 압축파일내 파일 close
        unzCloseCurrentFile(unzip_file);
        ++result;
      }
      // 다음 파일로 넘어간다. unzip file 의 끝일 경우 종료처리한다.
      if (UNZ_ENDED((unzip_status = unzGoToNextFile(unzip_file)))) break;
    } while (UNZ_SUCCEED(unzip_status));
  }
  return result;
}

// open 된 unz file 의 최신파일을 읽어 아웃풋 파일로 생성한다.
// 성공시 파일사이즈 (0 이상), 실패시 -1
int32_t SimpleZip::WriteFileFromZipFile_(unzFile unzip_file, std::string output_file)
{
  int32_t read_size = 0;
  int32_t write_size = 0;
  int32_t total_read_size = 0;
  char error_buffer[256] = { 0, };
  char read_buffer[4096] = { 0, };
  if (!output_file.empty()) {
    FILE * insert_file = NULL;
    errno_t err = fopen_s(&insert_file, output_file.c_str(), "wb");
    if (!err) {
      while (true) {
        // return the number of byte copied if somes bytes are copied
        // return 0 if the end of file was reached
        // return < 0 with error code if there is an error
        // (UNZ_ERRNO for IO error, or zLib error for uncompress error)
        read_size = unzReadCurrentFile(unzip_file, (void*)read_buffer, sizeof(read_buffer));
        if (read_size < 0) {
          total_read_size = -1;
#if _MSC_VER >= 1700
          error_message_ = "failed to read unz file [" + std::to_string(read_size)
            + "]> " + output_file;
#else
          error_message_ = "failed to read unz file [" + std::to_string(static_cast<uint64_t>(read_size))
            + "]> " + output_file;
#endif
          break;
        } else if (!read_size) {
          // 파일 읽기 완료
          break;
        } else {
          if ((write_size = fwrite(read_buffer, sizeof(char), read_size, insert_file)) < 0) {
            total_read_size = -1;
#if _MSC_VER >= 1700
            error_message_ = "failed to write file [" + std::to_string(ferror(insert_file))
              + "]> " + output_file;
#else
            error_message_ = "failed to write file [" 
              + std::to_string(static_cast<uint64_t>(ferror(insert_file)))
              + "]> " + output_file;
#endif
            break;
          } else {
            if (write_size != read_size) {
              total_read_size = -1;
#if _MSC_VER >= 1700
              error_message_ = "failed to write file [" + std::to_string(ferror(insert_file))
                + "]> " + output_file;
#else
              error_message_ = "failed to write file [" 
                + std::to_string(static_cast<uint64_t>(ferror(insert_file)))
                + "]> " + output_file;
#endif
              break;
            } else {
              total_read_size += read_size;
            }
          }
        }
      }
      fclose(insert_file);
    } else {
      total_read_size = -1;
#if _MSC_VER >= 1900
      strerror_s(error_buffer, err);
      error_message_ = "failed to open file [" + std::string(error_buffer)
        + "]> " + output_file;
#else
      error_message_ = "failed to open file [" + std::string(strerror(err))
        + "]> " + output_file;
#endif
    }
  } else {
    error_message_ = "empty output entry file";
  }
  return total_read_size;
}

void SimpleZip::MakeDirectory(std::string full_path)
{
  uint32_t check_file = 0;
  std::string path = full_path;
  std::string::size_type pos = path.find_last_of("/\\");
  TCHAR current_path[MAX_PATH] = { 0, };
  GetCurrentDirectory(sizeof(current_path) - 1, current_path);
  if (pos != std::string::npos) {
    // 마지막 파일경로를 제외하고 삭제.
    // 디렉토리만 있는경우 마지막이 '/' 이므로 마지막 생성해야할 디렉토리만 남는다.
    path.erase(++pos, std::string::npos);
  }
  // 시작위치 초기화
  pos = 0;
  // 하위부터 상위로 이동하면서 디렉토리가 없으면 생성
  while ((pos = path.find_first_of("/\\", pos)) != std::string::npos) {
    // 가장 하위 디렉토리만 복사후 다음 경로로 이동
    std::string sub_path = std::string(current_path) + "\\" + path.substr(0, pos++);
    if (!IsDirectory(sub_path, check_file)) {
      // 디렉토리가 아니며 경로가 비어있는 경우 생성한다.
      if (check_file == genum::zip::kEmpty) {
        if (CreateDirectory(sub_path.c_str(), NULL)) {
#if defined(_TEST_MODE) || defined(_DEBUG)
          std::cout << "create directory : " << sub_path << std::endl;
#endif
        }
      }
    }
  }
}

/**
  등록된 target entry 를 체크한다.
  현재 경로를 기준으로 등록된 파일과 디렉토리를 체크하여 절대경로 설정 및 
  하위엔트리를 체크하여 등록한다.
*/
uint32_t SimpleZip::CheckEntry_()
{
  uint32_t result = 0;
  std::string absolute_path;
  DWORD file_type;
  TCHAR current_path[MAX_PATH] = {0,};
  GetCurrentDirectory(sizeof(current_path)-1, current_path);
  for (auto it = target_list_.begin() ;it != target_list_.end() ;) {
    absolute_path = std::string(current_path) + std::string("\\") + it->name_;
    file_type = ::GetFileAttributes(absolute_path.c_str());
    it->absolute_path_ = absolute_path;
    if (file_type == INVALID_FILE_ATTRIBUTES) {
      // 경로 또는 파일이 존재하지 않으므로 삭제
      error_message_ = "file does not exist ["+ absolute_path +"]";
      error_history_.push_back(error_message_);
      target_list_.erase(it++);
      continue;
    }
    if (file_type & FILE_ATTRIBUTE_DIRECTORY) {
      // 해당 엔트리는 폴더
      it->is_directory_ = true;
      // 폴더 일경우 디렉토리내 파일 및 디렉토리 체크하여 엔트리에 추가
      result += CheckEntry_(&(*it));
    } else {
      // 해당 엔트리는 파일
      ++result;
      it->is_directory_ = false;
    }
    it++;
  }
  return result;
}

/**
  하위 엔트리 체크 재귀함수
  전달받은 directory entry 내의 모든 파일과 디렉토리를 검색하여 전달받은
  directory entry 에 추가한다.
  전달받은 directory entry 에 하위 directory 가 존재할 경우 재귀함수로 호출한다.
*/
uint32_t SimpleZip::CheckEntry_(Entry* entry)
{
  WIN32_FIND_DATA find_file_data;
  HANDLE handle_find;
  uint32_t result = 0;
  std::string search_path = entry->absolute_path_ + std::string("\\*");

  handle_find = FindFirstFile(search_path.c_str(), &find_file_data);
  if (handle_find == INVALID_HANDLE_VALUE)  return 0;
  do {
    if(!strcmp(find_file_data.cFileName, ".") || !strcmp(find_file_data.cFileName, ".."))  continue;
    Entry insert_entry(find_file_data.cFileName);
    insert_entry.absolute_path_ = entry->absolute_path_ + std::string("\\") + insert_entry.name_;
    insert_entry.relative_path_ = entry->relative_path_ + std::string("\\") 
                                  + std::string(find_file_data.cFileName);
    if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      insert_entry.is_directory_ = true;
      result += CheckEntry_(&insert_entry);
    } else {
      insert_entry.is_directory_ = false;
      ++result;
    }
    entry->entry_map_.insert(std::make_pair(find_file_data.cFileName, insert_entry));

  } while (FindNextFile(handle_find, &find_file_data));

  FindClose(handle_find);
  return result;
}

/*
  등록된 모든 엔트리에 대해서 stdout 으로 출력
*/
void SimpleZip::PrintAllEntry()
{
  for (auto it = target_list_.begin() ;it != target_list_.end() ;it++) {
    PrintAllEntry_(&(*it), 0);
  }
}

/*
  하위 엔트리 출력 재귀함수
*/
void SimpleZip::PrintAllEntry_(Entry* entry, int depth)
{
  if (entry) {
    if (entry->is_directory_) {
      for (int i=0 ; i<depth*2  ; i++) std::cout << " ";
      std::cout << "Dir : " << entry->name_ << std::endl;
      for (auto it = entry->entry_map_.begin() ; 
          it != entry->entry_map_.end() ; it++) {
        PrintAllEntry_(&(it->second), depth+1);
      }
    } else {
      for(int i=0;i<depth*2;i++) std::cout << " ";
      std::cout << "File : " << entry->name_ << std::endl;
    }
  }
}

// 참조 https://docs.microsoft.com/ko-kr/windows/desktop/FileIO/file-attribute-constants
// 해당 path 가 디렉토리인지 체크한다.
// 디렉토리 일 경우 true,  아닌 경우 false 를 리턴한다.
bool SimpleZip::IsDirectory(std::string path, uint32_t& exception_value)
{
  DWORD file_type = INVALID_FILE_ATTRIBUTES;
  DWORD last_error = 0;
  file_type = ::GetFileAttributes(path.c_str());
  if (file_type == INVALID_FILE_ATTRIBUTES) { // 0xffffffff
    if ((last_error = GetLastError()) == ERROR_FILE_NOT_FOUND) {
      // 존재하지 않는 경로
      exception_value = genum::zip::kEmpty;
    } else {
      exception_value = genum::zip::kFailed;
    }
    return false;
  } else if (file_type & FILE_ATTRIBUTE_DIRECTORY) {
    exception_value = genum::zip::kDirectory;
    return true;
  } else {
    exception_value = genum::zip::kExistFile;
    return false;
  }
}

} // namespace gstd