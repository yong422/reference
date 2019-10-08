#ifndef GSTD_WINDOWS_MSGBOX_H
#define GSTD_WINDOWS_MSGBOX_H
#pragma once
namespace gstd {
namespace msg {
  // Windows error code를 문자열 메시지박스로 알린다.
  void ErrorMessageBox(DWORD dwError);
} //namespace msg
} //namespace gstd

#endif