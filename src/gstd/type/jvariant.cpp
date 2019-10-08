/**
  @file    jvariant.cpp
  @date    2012/10/17
  @author    조용규(yong422@nate.com) Gabia
  @brief    JVARIANT 구조체가 정의됨
  @section  수정내역(작업자/변경일/수정내용)
  - 조용규/20121017/최초작성
*/
#if defined(_WIN32)
#include <Windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jvariant.h"

JVARIANT::JVARIANT(int size)
{
  szVal = NULL;
  szVal = new char[size];
  strsize = size;
  clear();
  type = jvariant::type_empty;
}

JVARIANT::JVARIANT(const char* str)
{
  szVal = NULL;
  szVal = new char[strlen(str)+1];
  strsize = strlen(str)+1;
  clear();
  type = jvariant::type_empty;
  insert(str);
}

JVARIANT::JVARIANT(std::string str)
{
  szVal = NULL;
  szVal = new char[str.length()+1];
  strsize = str.length()+1;
  type = jvariant::type_empty;
  insert(str.c_str());
}

JVARIANT::~JVARIANT()
{
  free();
}

void JVARIANT::free()
{
  if(szVal)
    delete[] szVal;
  szVal = NULL;
  strsize = 0;
}

void JVARIANT::clear(){    
    type=0; 
    lVal=0; 
    if(szVal)
      memset(szVal, 0x00, strsize);
    fVal=0; 
    dVal=0; 
    nVal=0; 
    uVal=0; 
    llVal=0; 
    ullVal=0;
    type = jvariant::type_empty;
}

void JVARIANT::clear(int size)
{
  if(strsize != size){
    if(szVal){
      memset(szVal, 0x00, strsize);
      delete[] szVal;
      szVal = NULL;
    }
    szVal = new char[size];
    strsize = size;
  }
  clear();
}
/**
  함수오버로딩 insert 함수들.
*/

void JVARIANT::insert(long val)
{ 
  lVal = (long)val; 
  type=jvariant::type_long; 
}

void JVARIANT::insert(std::string val)
{
  insert(val.c_str());
}

void JVARIANT::insert(const char *val)
{ 
  int nStrSize = strlen(val);
  if(nStrSize >= strsize){  //할당된 문자열버퍼보다 큰경우.
    if(strsize && szVal){
      clear(nStrSize+1);
    }else{
      strsize = nStrSize+1;
      szVal = new char[strsize];
      memset(szVal, 0x00, strsize);
      strncpy(szVal, val, strsize-1);
    }
  }else{
    memset(szVal, 0x00, strsize);
  }
  strncpy(szVal, val, strsize-1);
  type=jvariant::type_lpstr;
}

void JVARIANT::insert(float val)
{ 
  fVal = (float)val; 
  type=jvariant::type_float;
}
void JVARIANT::insert(double val)
{ 
  dVal = (double)val;
  type=jvariant::type_double;
}
void JVARIANT::insert(int val)
{ 
  nVal = val; 
  type=jvariant::type_int; 
}
void JVARIANT::insert(unsigned int val)
{ 
  uVal = (unsigned int)val; 
  type=jvariant::type_uint;
}
void JVARIANT::insert(long long val)
{ 
  llVal = (long long)val; 
  type=jvariant::type_int64;
}
void JVARIANT::insert(unsigned long long val)
{ 
  ullVal = (unsigned long long)val; 
  type=jvariant::type_uint64; 
}

unsigned long long JVARIANT::to_uint64()
{
  unsigned long long ullRet=0;
  char* pnull=NULL;
  switch(type){
    //case 0:
  //    return 0;
    case jvariant::type_long :
      ullRet = (unsigned long long)lVal;
      break;
    case jvariant::type_lpstr :
      if(strlen(szVal)>0) {
#ifdef _WIN32
        ullRet = _strtoui64(szVal, NULL, 10);
#endif
#ifdef _LINUX
        ullRet = strtoull(szVal, &pnull, 10);
#endif
      } else {
        ullRet = 0;
      }
      break;
    case jvariant::type_float :
      ullRet = (unsigned long long)fVal;
      break;
    case jvariant::type_double :
      ullRet = (unsigned long long)dVal;
      break;
    case jvariant::type_int :
      ullRet = (unsigned long long)nVal;
      break;
    case jvariant::type_uint :
      ullRet = (unsigned long long)uVal;
      break;
    case jvariant::type_int64 :
      ullRet = (unsigned long long)llVal;
      break;
    case jvariant::type_uint64 :
      ullRet = ullVal;
      break;
  }
  return ullRet;
}

long long JVARIANT::to_int64()
{
  long long llRet=0;
  switch(type){
    //case 0:
  //    return 0;
    case jvariant::type_long :
      llRet = (long long)lVal;
      break;
    case jvariant::type_lpstr :
      if(strlen(szVal) > 0){
#ifdef _WIN32
        llRet = _strtoi64(szVal, NULL, 10);
#else
        llRet = strtoll(szVal, NULL, 10);
#endif
      }else{
        llRet = 0;
      }
      break;
    case jvariant::type_float :
      llRet = (long long)fVal;
      break;
    case jvariant::type_double :
      llRet = (long long)dVal;
      break;
    case jvariant::type_int :
      llRet = (long long)nVal;
      break;
    case jvariant::type_uint :
      llRet = (long long)uVal;
      break;
    case jvariant::type_int64 :
      llRet = llVal;
      break;
    case jvariant::type_uint64 :
      llRet = (long long) ullVal;
      break;
  }
  return llRet;
}

unsigned int JVARIANT::to_uint()
{
  unsigned int uRet=0;
  switch(type){
    case jvariant::type_long :
      uRet = (unsigned int)lVal;
      break;
    case jvariant::type_lpstr :
      uRet = atoi(szVal);
      break;
    case jvariant::type_float :
      uRet = (unsigned int)fVal;
      break;
    case jvariant::type_double :
      uRet = (unsigned int)dVal;
      break;
    case jvariant::type_int :
      uRet = (unsigned int)nVal;
      break;
    case jvariant::type_uint :
      uRet = uVal;
      break;
    case jvariant::type_int64 :
      uRet = (unsigned int)llVal;
      break;
    case jvariant::type_uint64 :
      uRet = (unsigned int)ullVal;
      break;
  }
  return uRet;
}

int JVARIANT::to_int()
{
  int nRet=0;
  switch(type){
    case jvariant::type_long :
      nRet = (int)lVal;
      break;
    case jvariant::type_lpstr :
      nRet = (int)strtol(szVal, NULL, 10);
      break;
    case jvariant::type_float :
      nRet = (int)fVal;
      break;
    case jvariant::type_double :
      nRet = (int)dVal;
      break;
    case jvariant::type_int :
      nRet = nVal;
      break;
    case jvariant::type_uint :
      nRet = (int)uVal;
      break;
    case jvariant::type_int64 :
      nRet = (int)llVal;
      break;
    case jvariant::type_uint64 :
      nRet = (int)ullVal;
      break;
  }
  return nRet;
}

double JVARIANT::to_double()
{
  char* pEnd=NULL;
  double dRet=0;
  switch(type){
    case jvariant::type_long :
      dRet = (double)lVal;
      break;
    case jvariant::type_lpstr :
      //dRet = strtoul(szVal, NULL, 10);
      dRet = strtod(szVal, &pEnd);
      break;
    case jvariant::type_float :
      dRet = (double)fVal;
      break;
    case jvariant::type_double :
      dRet = (double)dVal;
      break;
    case jvariant::type_int :
      dRet = nVal;
      break;
    case jvariant::type_uint :
      dRet = (double)uVal;
      break;
    case jvariant::type_int64 :
      dRet = (double)llVal;
      break;
    case jvariant::type_uint64 :
      dRet = (double)ullVal;
      break;
  }
  return dRet;
}

char* JVARIANT::to_string()
{
  switch(type){
    case jvariant::type_long :
      sprintf(szVal,"%ld",lVal);
      break;
    case jvariant::type_ipaddress :
    case jvariant::type_lpstr :
      break;
    case jvariant::type_float :
      sprintf(szVal,"%.2f",fVal);
      break;
    case jvariant::type_double :
      sprintf(szVal,"%.2f",dVal);
      break;
    case jvariant::type_int :
      sprintf(szVal, "%d", nVal);
      break;
    case jvariant::type_uint :
      sprintf(szVal, "%u", uVal);
      break;
    case jvariant::type_int64 :
      sprintf(szVal, "%lld", llVal);
      break;
    case jvariant::type_uint64 :
      sprintf(szVal, "%llu", ullVal);
      break;
  }
  return szVal;
}

char* JVARIANT::int_to_string(int val)
{
  clear();
  insert(val);
  return to_string();
}

char* JVARIANT::uint_to_string(unsigned int val)
{
  clear();
  insert(val);
  return to_string();
}

char* JVARIANT::uint64_to_string(unsigned long long val)
{
  clear();
  insert(val);
  return to_string();
}

char* JVARIANT::int64_to_string(long long val)
{
  clear();
  insert(val);
  return to_string();
}

char* JVARIANT::double_to_string(double val)
{
  clear();
  insert(val);
  return to_string();
}

char* JVARIANT::float_to_string(float val)
{
  clear();
  insert(val);
  return to_string();
}

long long JVARIANT::string_to_int64(const char* val)
{
  clear();
  insert(val);
  return to_int64();
}

unsigned long long JVARIANT::string_to_uint64(const char* val)
{
  clear();
  insert(val);
  return to_uint64();
}

int JVARIANT::string_to_int(const char* val)
{
  clear();
  insert(val);
  return to_int();
}

unsigned int JVARIANT::string_to_uint(const char* val)
{
  clear();
  insert(val);
  return to_uint();
}


JVARIANT& JVARIANT::operator=(const JVARIANT& val)
{
  if(val.type == jvariant::type_lpstr){
    free();
    strsize = strlen(val.szVal)+1;
#ifdef _WIN32
    LPSTR lpstr = new CHAR[strsize];
#else
    char* lpstr = new char[strsize];
#endif
    strncpy(lpstr, val.szVal, strsize-1);
    type = jvariant::type_lpstr;  
  }else{
    this->type = val.type;
    this->lVal = val.lVal;
    this->fVal = val.fVal;
    this->dVal = val.dVal;
    this->nVal = val.nVal;
    this->uVal = val.uVal;
    this->llVal = val.llVal;
    this->ullVal = val.ullVal;
  }
  return *this;
}

/**
JVARIANT JVARIANT::operator=(const int& val)
{
  JVARIANT var;
  var.free();
  var.insert(val);
  return var;
}

JVARIANT JVARIANT::operator=(const unsigned long long& val)
{
  JVARIANT var;
  var.free();
  var.insert(val);
  return var;
}
*/
