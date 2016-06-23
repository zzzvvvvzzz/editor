/*Editor.cpp----my editor(c) zhuxj,2016*/

#include <windows.h>
#include "resource.h"
#include <list>
#include "editor.h"
using namespace std;
BOOL FILEOPENED = FALSE;
static TCHAR szAppName[] = TEXT("editor");

/* Wnd,AboutDlg,HelpDlg Process */
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ViewHelpDlgProc(HWND, UINT, WPARAM, LPARAM);



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  PSTR szCmdLine, int iCmdShow)
{

  HWND hwnd;
  HACCEL hAccel;
  MSG msg;
  WNDCLASS wndclass;

  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = hInstance;
  wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(EICON));
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName = szAppName;
  wndclass.lpszClassName = szAppName;

  if (!RegisterClass(&wndclass))
  {
    MessageBox(NULL, TEXT("Registe wndclass fail!"), szAppName, MB_ICONERROR);
    return 0;
  }

  hwnd = CreateWindow(szAppName, szAppName,
    WS_OVERLAPPEDWINDOW | WS_VSCROLL| WS_HSCROLL,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    NULL, NULL, hInstance, NULL);

  ShowWindow(hwnd, iCmdShow);
  UpdateWindow(hwnd);
  hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

  while (GetMessage(&msg, NULL, 0, 0))
  {
    if (!TranslateAccelerator(hwnd, hAccel, &msg))
    {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    }
  }
  return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  HDC hdc;
  static HINSTANCE hInstance;
  static HMENU hMenu, hPopMenu;
  PAINTSTRUCT ps;
  RECT rect;
  TCHAR szBuffer[10];
  TEXTMETRIC tm;
  SCROLLINFO si;
  LPCWSTR strfile;
  static POINT point, ptCaret;
  static int cxChar, cxCaps, cyChar, cxClient, cyClient, cLine,
             cWidth, CharWidth, CodeMode, iFileLength;
  static int MaxWidth = 0;
  static TCHAR szFileName[MAX_PATH], szFileTitle[MAX_PATH];
  static list<list<TCHAR>> TextList, ClipList;
  static list<list<TCHAR>>::iterator itorLine;
  static list<TCHAR>::iterator itorWord;
  static TCHAR WordBuffer;


  /*Test Param*/
  int i=0;
  switch (message)
  {
  case WM_CREATE:
    hdc = GetDC(hwnd);
    hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
    hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(EDITOR));
    hPopMenu = LoadMenu(hInstance, MAKEINTRESOURCE(POPMENU));
    hPopMenu = GetSubMenu(hPopMenu, 0);
    GetTextMetrics(hdc, &tm);
    cxChar = tm.tmAveCharWidth;
    cyChar = tm.tmHeight;
    cxClient = LOWORD(lParam);
    cyClient = HIWORD(lParam);
    SetMenu(hwnd, hMenu);
    CreateCaret(hwnd, NULL, 1, cyChar);
    ShowCaret(hwnd);
    SetCaretPos(0, 0);
    ReleaseDC(hwnd, hdc);
    HideCaret(hwnd);
    return 0;

  case WM_SIZE:
    cxClient = LOWORD(lParam);
    cyClient = HIWORD(lParam);
    break;
  case WM_PAINT:
    hdc = BeginPaint(hwnd, &ps);
    ShowCaret(hwnd);
    if (FILEOPENED != FALSE)
    {
       cLine = 0;
       cWidth = 0;
       for (itorLine = TextList.begin();
            itorLine != TextList.end(); ++itorLine)
       {
         for (itorWord = (*itorLine).begin(); 
              itorWord != (*itorLine).end();
              ++itorWord)
         {
           if (*itorWord == 0x000D)
           {
             ++cLine;
             cWidth = 0;
             break;
           }
           else
           {
             wsprintf(&WordBuffer, TEXT("%c"), (*itorWord));
             TextOut(hdc, cWidth, cLine*cyChar, &WordBuffer, 1);
             GetCharWidth(hdc, WordBuffer, WordBuffer, &CharWidth);
             cWidth += CharWidth;
             if (cWidth > MaxWidth)
               MaxWidth = cWidth;
           }
         }
       }
       SetCaretPos(cWidth, cyChar*cLine);
    }
  
    HideCaret(hwnd);
    EndPaint(hwnd, &ps);
    return 0;
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case ID_HELP_ABOUT:
      DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
      return 0;

    case ID_HELP_VIEWHELP:
      DialogBox(hInstance, MAKEINTRESOURCE(IDD_HELP), hwnd, ViewHelpDlgProc);
      return 0;

    case ID_FILE_OPEN:
      if (FileOpenDlg(hwnd, szFileName, szFileTitle))
      {
        TextList.clear();
        TextFileToList(szFileName, TextList, CodeMode);
        InvalidateRect(hwnd, NULL, TRUE);
      }
      return 0;
    case ID_FILE_SAVE:
      ListToTextFile(szFileName, TextList, CodeMode);
      return 0;


    case ID_FILE_SAVEAS:
      if ( FileSaveDlg(hwnd, szFileName, szFileTitle))
      ListToTextFile(szFileName, TextList, CodeMode);
      return 0;
    case ID_FILE_EXIT:
      PostQuitMessage(0);
      return 0;
    }
    return 0;

  case WM_RBUTTONUP:
    point.x = LOWORD(lParam);
    point.y = HIWORD(lParam);
    ClientToScreen(hwnd, &point);
    TrackPopupMenu(hPopMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_CHAR:
    hdc = GetDC(hwnd);
    GetCaretPos(&ptCaret);
    HideCaret(hwnd);
    wsprintf(&WordBuffer, TEXT("%c"), wParam);
    TextOut(hdc, ptCaret.x, ptCaret.y, &WordBuffer, 1);
    GetCharWidth(hdc, WordBuffer, WordBuffer, &CharWidth);
    SetCaretPos(ptCaret.x + CharWidth, ptCaret.y);
    ShowCaret(hwnd);
    ReleaseDC(hwnd, hdc);
    return 0;
    
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
    return TRUE;
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
    case IDCANCEL:
    EndDialog(hDlg, 0);
    return TRUE;
    }
    break;
  }
  return FALSE;
}
BOOL CALLBACK ViewHelpDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
    return TRUE;
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
    case IDCANCEL:
      EndDialog(hDlg, 0);
      return TRUE;
    }
    break;
  }
  return FALSE;
}

