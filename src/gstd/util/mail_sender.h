#ifndef GSTD_UTIL_MAIL_SENDER_H
#define GSTD_UTIL_MAIL_SENDER_H

#include <list>
#include <map>
#include <string>
//#include <boost/shared_ptr.hpp>


//  @brief  mail발송을 위한 wrapper class
//          CSmtp_v1_8b 참조.
//          기본값으로 내부에서 사용가능한 inmail.gabia.com 이용하여 메일발송.
class CSendMail{
public:
  CSendMail(char* smtpServer="inmail.gabia.com", int smtpPort=2525, char* admin="Monitoring", char* adminMail="");
  CSendMail(std::string smtpServer="inmail.gabia.com", int smtpPort=2525, std::string admin="Monitoring", std::string adminMail="");
  ~CSendMail();
  void Clear();
  
  int Send(char* subject, char* msg, char* targetMail);

  std::string GetLastError() { return _lastError; };

private:
  std::string    _lastError;
  std::string   _smtpServer;
  std::string   _senderName;
  std::string   _senderMail;
  unsigned int  _uSmtpPort;
};

#endif
