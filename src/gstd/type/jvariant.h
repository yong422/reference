/**
  @file    jvariant.h
  @date    2012/10/17
  @author    조용규(yong422@nate.com) Gabia
  @brief    JVARIANT 구조체가 선언됨
  @brief      jvariant nameclass의 변수가 선언됨.
  @section  수정내역(작업자/변경일/수정내용)
  - 조용규/20121017/최초작성
  - 김수겸/20121207/to_double() 함수 추가 
  - 조용규/20130719/문자열 insert 함수 사이즈체크및 재할당 추가
  - 조용규/20160804/문자열 std::string 추가
*/

#ifndef GSTD_TYPE_JVARIANT_H
#define GSTD_TYPE_JVARIANT_H

#define JVARIANT_STRSIZE 512
#include <string>
/**
  @namespace  jvariant
  @date    2012/10/17
  @author    조용규
  @brief    JVARIANT 에서 사용될 기본 상수값을 namespace화 하였음.
*/
namespace jvariant
{
  enum{
    type_empty=0,
    type_long=1, ///< long 타입
    type_lpstr=2, ///< string 타입
    type_float=3, ///< float 타입
    type_double=4, ///< double 타입
    type_int=5, ///< int 타입
    type_uint=6, ///< unsigned int 타입
    type_int64=7, ///< long long (int64) 타입
    type_uint64=8, ///< unsigned long long (unsigned int64) 타입
    type_ipaddress=31, ///< snmp용 ipaddress 타입
    str_size=512 ///< JVARIANT 스트링버퍼의 디폴트생성사이즈
  };
};
/**
  @struct    JVARIANT
  @date    2012/10/17
  @author    조용규
  @brief    여러 데이터타입 동시사용 가능한 구조체로 정의.
  @brief      window의 variant 와 비슷함.
  @warning  (없음)
*/
struct JVARIANT{
  JVARIANT(int size=jvariant::str_size);
  JVARIANT(const char* str);
  JVARIANT(std::string str);
  ~JVARIANT();

  int          type;        ///< 저장된 변수타입 
  int          strsize;      ///< 저장된 문자열버퍼 크기

  long        lVal;        ///< long 타입 변수
  char        *szVal;      ///< 문자옆 버퍼 포인터
  float        fVal;        ///< float 타입 변수
  double      dVal;        ///< double 타입 변수
  int          nVal;        ///< int 타입 변수
  unsigned int    uVal;      ///< unsigned int 타입 변수
  long long       llVal;      ///< long long(int64) 타입 변수
  unsigned long long   ullVal;          ///< unsigned long long(unsigned int64) 타입 변수

    bool isval(){return type == jvariant::type_empty ? false:true;}
  void free();
  void clear();           ///< 초기화
  void clear(int size);       ///< 문자열변수 해당size로 재할당 함수

  long to_long();         ///< 하위값 long타입 리턴함수
  char* to_string();         ///< 하위값 문자열 리턴함수
  int to_int();           ///< 하위값 int타입 리턴함수
  unsigned int to_uint();     ///< 하위값 unsigned int타입 리턴함수
  unsigned long long to_uint64(); ///< 하위값 unsigned long long타입 리턴함수
  double to_double();       ///< 하위값 double타입 리턴함수
  long long to_int64();       ///< 하위값 int64 타입 리턴함수

  long long string_to_int64(const char* val);  ///< 받은 문자열은 저장하고 변경된 int64값 리턴.
  unsigned long long string_to_uint64(const char* val);
  int string_to_int(const char* val);
  unsigned int string_to_uint(const char* val);
  char* uint64_to_string(unsigned long long val);
  char* int64_to_string(long long val);
  char* int_to_string(int val);
  char* uint_to_string(unsigned int val);
  char* double_to_string(double val);
  char* float_to_string(float val);

  void insert(long val);
  void insert(const char *val);
  void insert(float val);
  void insert(double val);
  void insert(int val);
  void insert(unsigned int val);
  void insert(long long val);
  void insert(unsigned long long val);
  void insert(std::string val);
  JVARIANT& operator=(const JVARIANT& val);
  JVARIANT operator=(const int& val);
  JVARIANT operator=(const unsigned long long& val);
};

#endif
