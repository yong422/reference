#include <map>
#include <list>
#include <vector>

#include "gstd/util/mail_sender.h"
#include "gstd/net/CSmtp.h"

CSendMail::CSendMail(char* smtpServer, int smtpPort, 
          char* admin, char* adminMail)
{
  Clear();
  if(smtpServer)
    _smtpServer.assign(smtpServer);
  if(admin)
    _senderName.assign(admin);
  if(adminMail)
    _senderMail.assign(adminMail);
  _uSmtpPort = static_cast<unsigned int>(smtpPort);
  _lastError.clear();
}

CSendMail::~CSendMail()
{

}

void CSendMail::Clear()
{
  _smtpServer.clear();
  _senderName.clear();
  _senderMail.clear();
  _lastError.clear();
  _uSmtpPort = 0;
}


int CSendMail::Send(char* subject, char* msg, char* targetMail)
{
  int ret=0;
  if(msg&&subject){
    CSmtp mail;
    try{
      mail.SetSMTPServer(_smtpServer.c_str(), _uSmtpPort);
      mail.SetSenderName(_senderName.c_str());
      mail.SetSenderMail(_senderMail.c_str());
      mail.SetReplyTo(_senderMail.c_str());
      mail.SetSubject(subject);
      mail.SetXPriority(XPRIORITY_NORMAL);
      mail.SetTextType("text");
      mail.AddMsgLine(msg);
      mail.AddRecipient(targetMail);
      mail.Send();
      ret++;
    }catch(ECSmtp e){
      _lastError.clear();
      _lastError = "Error: " + e.GetErrorText();
      mail.Close();
    }  
  }
  return ret;
}

#ifdef _TEST_MAILSENDER
int main(int argc, char** argv)
{
  CSendMail mail("inmail.gabia.com", 2525, "Mon TEST", "ykjo@gabia.com");
  mail.Send("[TMS ML] (가산 CNS) TMS all intrusion detection"
      ,"time: 2016-09-29 11:31:44\nnflows: 19308\n     10324         6      1509      2498     13559         0     17362  0"        , "ykjo@gabia.com");

  return 1;
}
#endif

