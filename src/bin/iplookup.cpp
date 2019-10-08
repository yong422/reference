// @brief  ip정보가 저장된 csv 파일에서 ip 리스트를 이용하여 국가 및 isp 정보를 검색.
//        검색된 국가 및 isp 정보를 원본 csv 파일 뒤에 추가 컬럼으로 입력한다.
//        국가 및 isp 통계는 별도의 파일로 출력한다.

#include <list>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include "gstd/util/strtool.h"
#include "gstd/util/cmdexecute.h"
#include "gstd/util/filetool.h"
#include "gstd/util/timer.h"
#include "gstd/net/ipchecker.h"


// ip 리스트가 저장된 csv 파일의 ip 를 geoip maxminddb 에서 국가코드와 ISP 정보를 확인.
// TODO: maxminddb 디비파일 경로 설정가능하도록 추가.

#if 1
// v2
// 전체 csv 파일중 ip column 선택
// Country, Organization, ISP 순으로 검색하여 입력포맷에 + 하여 출력
int main(int argc, char** argv)
{
  char ipbuf[32] = {0,};
  int maxcount=0;

  if (argc > 4) {
    // ok
  } else {
    fprintf(stdout, "Usage : %s <file> <csv 전체컬럼> <ipaddress 컬럼> <결과파일>\n", argv[0]);
    exit(0);
  }
  int maxcolumn=atoi(argv[2]);
  int ipaddrcolumn=atoi(argv[3]);
  gstd::util::CmdExecute cmd;
  std::map<std::string, int>  map_ccode;
  std::map<std::string, int>  map_isp;
  gstd::net::IpChecker checker;
  gstd::net::IpChecker ispChecker;
  gstd::util::FileTool file_tool;
  gstd::util::Timer ltimer;
  if (!checker.Open("/home/geoip/share/GeoIP/GeoIP2-Country.mmdb")) {
    fprintf(stdout, "Open failed : %s\n", checker.GetLastErrorMsg());
    exit(0);
  }
  if (!ispChecker.Open("/home/geoip/share/GeoIP/GeoIP2-ISP.mmdb")) {
    fprintf(stderr, "Open failed : %s\n", ispChecker.GetLastErrorMsg());
    exit(0);
  }
  __CPRINTF_SUCCESS(true, "mmdb open success");
  __CPRINTF_SUCCESS(true, checker.db_info().c_str());
  if (!file_tool.Open(argv[4], "wb")) {
    __CPRINTF_ERROR(true, "출력 파일 오픈 실패");
    exit(0);
  } else {
    file_tool.SetPermission(0755);
  }

  int count = 0;
  std::list<std::string> listval;
  while (true) {
    int krcount=0;
    ltimer.Start();
    if (cmd.GetStringList(argv[1], &listval, true)) {
      fprintf(stdout, "%s 가져오기성공\n", argv[1]);
    }

    while (!listval.empty()) {
      char countrybuf[128] = {0,};
      char isobuf[128] = {0,};
      char ispbuf[128] = {0,};
      char oribuf[128] = {0,};
      char totalbuf[1024]={0,};
      // 국가정보를 찾을 수 없는 경우
      std::vector<std::string> lRows;
      if (gstd::util::StringTool::Split(listval.front(), ',', &lRows)) {
        if (lRows.size() < maxcolumn) {
          __CPRINTF_ERROR(true, "error column size");
          listval.pop_front();
          continue;
        }
        // \r\n 일경우 줄 마지막을 null 처리
        if(lRows[maxcolumn-1][lRows[maxcolumn-1].length()-1] == '\r') 
            lRows[maxcolumn-1][lRows[maxcolumn-1].length()-1] = '\0';
        if (gstd::net::IpChecker::IsSpecialPurposeIp(lRows[ipaddrcolumn-1].c_str())) {
          // 예약 IP 일 경우
          fprintf(stdout, "%s > special purpose ip\n", lRows[ipaddrcolumn-1].c_str());
          if(map_ccode.find("private") != map_ccode.end()) {
            ++map_ccode["private"];
            ++map_isp["private"];
          } else {
            map_ccode.insert(std::pair<std::string, int>("private", 1));
            map_isp.insert(std::pair<std::string, int>("private", 1));
          }
        } else {
          if (checker.Find(lRows[ipaddrcolumn-1].c_str())) {
            
            if (checker.GetCountry(isobuf, countrybuf)) {
              std::map<std::string, int>::iterator it = map_ccode.find(isobuf);
              if (it != map_ccode.end()) {
                ++(it->second);
              } else {
                map_ccode.insert(std::pair<std::string, int>(isobuf, 1));
              }
            }
          } else {
            // 예약 IP 도 아닌 알수 없는 IP 인 경우
            if(map_ccode.find("--") != map_ccode.end())  
              ++map_ccode["--"];
            else
              map_ccode.insert(std::pair<std::string, int>("--", 1));
          }
        } 

        if (ispChecker.Find(lRows[ipaddrcolumn-1].c_str())) {
          ispChecker.GetISP(ispbuf, sizeof(ispbuf));
          ispChecker.GetOrganization(oribuf, sizeof(oribuf));
          std::map<std::string, int>::iterator it = map_isp.find(oribuf);
          if (it != map_isp.end()) {
            ++(it->second);
          } else {
            map_isp.insert(std::pair<std::string, int>(ispbuf, 1));
          }
        } else {
          if(map_isp.find("--") != map_isp.end())  
              ++map_isp["--"];
            else
              map_isp.insert(std::pair<std::string, int>("--", 1));
        }
        for (int i=0; i < maxcolumn; i++) {
          if(i>0) strcat(totalbuf,",");
          strcat(totalbuf, lRows[i].c_str());        
        }
        strcat(totalbuf, ",");
        strcat(totalbuf, countrybuf);
        strcat(totalbuf, ",");
        strcat(totalbuf, oribuf);
        strcat(totalbuf, ",");
        strcat(totalbuf, ispbuf);
        strcat(totalbuf, "\r\n");
        file_tool.Write(totalbuf, strlen(totalbuf));
        count++;
        listval.pop_front();
      }
    }
    fprintf(stdout, "ip address check count %d => time : %.4f sec\n", count, krcount, ltimer.Stop());
    break;
  }

  std::map<std::string, int>::iterator itret = map_ccode.begin();
  for(;itret != map_ccode.end(); itret++){
    fprintf(stdout, "COUNTRY CODE : %5s  ==> result count(%6d)\n",
            itret->first.c_str(), itret->second);
  }
  itret = map_isp.begin();
  for(;itret != map_isp.end(); itret++){
    fprintf(stdout, "ISP : %64s  ==> result count( %6d )\n",
            itret->first.c_str(), itret->second);
  }
  return 0;
  // program end
}
#else
int main(int argc, char** argv)
{
  char ipbuf[32] = {0,};
  int maxcount=0;

  if (argc > 1) {
    // ok
  } else {
    fprintf(stdout, "Usage : %s <file>\n", argv[0]);
    exit(0);
  }

  CCmdExecute cmd;
  std::map<std::string, int>  map_ccode;
  std::map<std::string, int>  map_isp;
  CIpChecker checker;
  CIpChecker ispChecker;
  gstd::util::Timer ltimer;
  if (!checker.Open("GeoIP2-Country.mmdb")) {
    fprintf(stdout, "Open failed : %s\n", checker.GetLastErrorMsg());
    exit(0);
  }
  if (!ispChecker.Open("GeoIP2-ISP.mmdb")) {
    fprintf(stderr, "Open failed : %s\n", ispChecker.GetLastErrorMsg());
    exit(0);
  }

  int count = 0;
  std::list<std::string> listval;
  while (true) {
    int krcount=0;
    ltimer.Start();
    if (cmd.GetStringList(argv[1], &listval, true)) {
      fprintf(stdout, "test.csv 가져오기성공\n");
    }

    while (!listval.empty()) {
      char countrybuf[128] = {0,};
      char isobuf[128] = {0,};
      char ispbuf[128] = {0,};
      // 국가정보를 찾을 수 없는 경우
      if (CIpChecker::IsSpecialPurposeIp(listval.front().c_str())) {
        // 예약 IP 일 경우
        if(map_ccode.find("private") != map_ccode.end()) {
          ++map_ccode["private"];
          ++map_isp["private"];
        } else {
          map_ccode.insert(std::pair<std::string, int>("private", 1));
          map_isp.insert(std::pair<std::string, int>("private", 1));
        }
      } else {
        if (checker.Find(listval.front().c_str())) {
          
          if (checker.GetCountry(isobuf, countrybuf)) {
            std::map<std::string, int>::iterator it = map_ccode.find(isobuf);
            if (it != map_ccode.end()) {
              ++(it->second);
            } else {
              map_ccode.insert(std::pair<std::string, int>(isobuf, 1));
            }
          }
        } else {
          // 예약 IP 도 아닌 알수 없는 IP 인 경우
          if(map_ccode.find("--") != map_ccode.end())  
            ++map_ccode["--"];
          else
            map_ccode.insert(std::pair<std::string, int>("--", 1));
        }
      } 

      if (ispChecker.Find(listval.front().c_str())) {
        //ispChecker.GetISP(ispbuf, sizeof(ispbuf));
        ispChecker.GetOrganization(ispbuf, sizeof(ispbuf));
        std::map<std::string, int>::iterator it = map_isp.find(ispbuf);
        if (it != map_isp.end()) {
          ++(it->second);
        } else {
          map_isp.insert(std::pair<std::string, int>(ispbuf, 1));
        }
      } else {
        if(map_isp.find("--") != map_isp.end())  
            ++map_isp["--"];
          else
            map_isp.insert(std::pair<std::string, int>("--", 1));
      }
      count++;
      listval.pop_front();
    }
    fprintf(stdout, "ip address check count %d => time : %.4f sec\n", count, krcount, ltimer.Stop());
    break;
  }

  std::map<std::string, int>::iterator itret = map_ccode.begin();
  for(;itret != map_ccode.end(); itret++){
    fprintf(stdout, "COUNTRY CODE : %5s  ==> result count(%6d)\n",
            itret->first.c_str(), itret->second);
  }
  itret = map_isp.begin();
  for(;itret != map_isp.end(); itret++){
    fprintf(stdout, "ISP : %64s  ==> result count( %6d )\n",
            itret->first.c_str(), itret->second);
  }
  return 0;
  // program end
}
#endif