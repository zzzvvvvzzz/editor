#include <windows.h>
#include <commdlg.h>
#include <list>
#include <iterator>
#include "editor.h"
using namespace std;
static OPENFILENAME ofn;
extern BOOL FILEOPENED;
void FileInit(HWND hwnd)
{
  static TCHAR szFilter[] = TEXT("Text Files(*.txt)\0*.txt\0")  \
                            TEXT("ASCII Files(*.ASC)\0*.asc\0")\
                            TEXT("All Fill(*.*)\0*.*\0");

  ofn.lStructSize       = sizeof(OPENFILENAME);
  ofn.hwndOwner         = hwnd;
  ofn.hInstance         = NULL;
  ofn.lpstrFilter       = szFilter;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter    = 0;
  ofn.nFilterIndex      = 0;
  ofn.lpstrFile         = NULL;
  ofn.nMaxFile          = MAX_PATH;
  ofn.lpstrFileTitle    = NULL;
  ofn.nMaxFileTitle     = MAX_PATH;
  ofn.lpstrInitialDir   = NULL;
  ofn.lpstrTitle        = NULL;
  ofn.Flags             = 0;
  ofn.nFileOffset       = 0;
  ofn.nFileExtension    = 0;
  ofn.lpstrDefExt       = TEXT("txt");
  ofn.lCustData         = 0L;
  ofn.lpfnHook          = NULL;
  ofn.lpTemplateName    = NULL;
}

BOOL FileOpenDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
  FileInit(hwnd);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = pstrFileName;
  ofn.lpstrFileTitle = pstrTitleName;
  ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;

  return GetOpenFileName(&ofn);
}
BOOL FileSaveDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTileName)
{
  FileInit(hwnd);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = pstrFileName;
  ofn.lpstrFileTitle = pstrTileName;
  ofn.Flags = OFN_OVERWRITEPROMPT;

  return GetSaveFileName(&ofn);
}


BOOL TextFileToList(PTSTR pstrFileName,list<list<TCHAR>> &TextList , int& CodeMode )
{
  HANDLE hFile;
  int iFileLength, iTransLen;
  DWORD dwBytesRead;
  list<TCHAR> LineList;
  if (INVALID_HANDLE_VALUE ==
    (hFile = CreateFile(pstrFileName, GENERIC_READ, 
    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
    OPEN_EXISTING, 0, NULL)))
    return FALSE;
  iFileLength = GetFileSize(hFile, NULL);
  PBYTE pBuffer = new BYTE[iFileLength+2];
  PTCHAR pTransBuffer = new TCHAR[iFileLength];
  ReadFile(hFile, pBuffer, iFileLength, &dwBytesRead, NULL);
  CloseHandle(hFile);
  pBuffer[iFileLength] = '\0';
  pBuffer[iFileLength + 1] = '\0';
  if (MAKEWORD(pBuffer[0], pBuffer[1]) == 0xFEFF)
  {
    CodeMode = TXTUNICODE;
    iTransLen = (iFileLength - 2) / 2;
    for (int i = 0; i < iFileLength -2; i += 2)
    {
      pTransBuffer[i / 2] = MAKEWORD(pBuffer[i+2], pBuffer[i + 3]);
    }
  }
  else if (MAKEWORD(pBuffer[0], pBuffer[1]) == 0xFFFE)
  {
    CodeMode = TXTUNICODEBE;
    iTransLen = (iFileLength - 2) / 2;
    for (int i = 0; i < iFileLength - 2; i += 2)
    {
      pTransBuffer[i / 2] = MAKEWORD(pBuffer[i + 3], pBuffer[i +2]);
    }
  }
  else if (pBuffer[0] == 0xEF && pBuffer[1] == 0xBB
           && pBuffer[2] == 0xBF)
  {
    CodeMode = TXTUTF8;
    iTransLen = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)(pBuffer+3), iFileLength-3, NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, (LPCCH)(pBuffer+3), iFileLength-3, pTransBuffer, iTransLen);
  }
  else
  {
    CodeMode = TXTANSI;
    iTransLen = MultiByteToWideChar(CP_ACP, 0, (LPCCH)pBuffer, iFileLength, NULL, 0);
    MultiByteToWideChar(CP_ACP, 0, (LPCCH)pBuffer, iFileLength, pTransBuffer, iTransLen);
  }

  for (int i = 0; i < iTransLen; i++)
  {
    if (pTransBuffer[i] == 0x000A)
    {
      LineList.push_back(pTransBuffer[i]);
      TextList.push_back(LineList);
      LineList.clear();
    }
    else
    {
      LineList.push_back(pTransBuffer[i]);
    }
  }
  TextList.push_back(LineList);
  LineList.clear();

 delete []pBuffer;
 delete []pTransBuffer;
 FILEOPENED = TRUE;
 return TRUE;
}


BOOL  ListToTextFile(PTSTR pstrFileName, list<list<TCHAR>> &TextList, int CodeMode)
{
  HANDLE hFile;
  list<TCHAR>::iterator itorWord;
  list<list<TCHAR>>::iterator itorLine;
  if (INVALID_HANDLE_VALUE ==
    (hFile = CreateFile(pstrFileName, GENERIC_WRITE, 
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, CREATE_ALWAYS, 0, NULL)))
    return FALSE;
  int CountofChar = 0, iTransLen;

  for (itorLine = TextList.begin(); itorLine != TextList.end(); ++itorLine)
  {
    CountofChar += (*itorLine).size();
  }
  PTCHAR pBuffer = new TCHAR[CountofChar];
  PTCHAR pTemp = pBuffer;
  PBYTE pTransBuffer = new BYTE[CountofChar * 3];
  for (itorLine = TextList.begin(); itorLine != TextList.end(); ++itorLine)
  {
    for (itorWord = (*itorLine).begin(); itorWord != (*itorLine).end(); ++itorWord,++pTemp)
    {
      *pTemp = *itorWord;
    }
  }
  pTemp = NULL;
  switch (CodeMode)
  {
  case TXTUNICODE:
    iTransLen = (CountofChar + 1) * 2;
    pTransBuffer[0] = 0xFEFF;
    for (int i = 0; i < CountofChar ; ++i )
    {
      pTransBuffer[i + 1] = MAKEWORD(pBuffer[2 * i], pBuffer[2 * i + 1]);
    }
    break;

  case TXTUNICODEBE:
    iTransLen = (CountofChar + 1) * 2;
    pTransBuffer[0] = 0xFFFE;
    for (int i = 0; i < CountofChar; ++i)
    {
      pTransBuffer[i + 1] = MAKEWORD(pBuffer[2 * i + 1], pBuffer[2 * i]);
    }
    break;

  case TXTANSI:
    iTransLen = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)pBuffer, CountofChar, NULL, 0, NULL, FALSE);
    WideCharToMultiByte(CP_ACP, 0, (LPCWCH)pBuffer, CountofChar, (LPSTR)pTransBuffer, iTransLen, NULL, FALSE);
    break;

    case TXTUTF8:
      iTransLen = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)pBuffer, CountofChar, NULL, 0, NULL, FALSE);
      WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)pBuffer, CountofChar, (LPSTR)(pTransBuffer+3), iTransLen, NULL, FALSE);
      pTransBuffer[0] = 0xEF;
      pTransBuffer[1] = 0xBB;
      pTransBuffer[2] = 0xBF;
      iTransLen += 3;
    break;

  default:
    return FALSE;
    break;
  }
  WriteFile(hFile,pTransBuffer,iTransLen,NULL,NULL);
  delete []pBuffer;
  delete []pTransBuffer;
  return TRUE;
}

