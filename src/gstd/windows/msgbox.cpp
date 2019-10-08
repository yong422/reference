#include <stdafx.h>
#include <windows.h>
#include <tchar.h>
#include "gstd/windows/msgbox.h"

void gstd::msg::ErrorMessageBox(DWORD dwError)
{
  LPVOID lpMsg = NULL;
  DWORD dwFormat = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS;
  DWORD dwLanguage = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

  if (!FormatMessage(dwFormat, NULL, dwError, dwLanguage, 
                      (LPTSTR)&lpMsg, 0, NULL)) {
    return;
  }
  MessageBox(NULL, (LPCTSTR)lpMsg, _T("Error"), MB_OK | MB_ICONINFORMATION);
  LocalFree(lpMsg);
}