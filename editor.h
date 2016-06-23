#include <list>
using namespace std;
#ifndef _EDITOR_H
#define _EDITOR_H
/*Code Mode of TXT Document*/

#define TXTUNICODE   1
#define TXTUNICODEBE 2
#define TXTANSI      3
#define TXTUTF8      4

/*Functions in EditorFile.cpp      */
void FileInit(HWND hwnd);
BOOL FileOpenDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);
BOOL FileSaveDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTileName);
BOOL TextFileToList(PTSTR pstrFileName, list<list<TCHAR>>& TextList,int& CodeMode);
BOOL ListToTextFile(PTSTR pstrFileName, list<list<TCHAR>>& TextList, int CodeMode);

#endif