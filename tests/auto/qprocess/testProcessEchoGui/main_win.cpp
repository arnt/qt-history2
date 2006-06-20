/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    
    HANDLE hStderr = GetStdHandle(STD_ERROR_HANDLE); 

    for (;;) {

        char c = 0;
        DWORD read = 0;
        if (!ReadFile(hStdin, &c, 1, &read, 0) || read == 0 || c == 'q' || c == '\0')
            break;
        DWORD wrote = 0;
        WriteFile(hStdout, &c, 1, &wrote, 0);
        WriteFile(hStderr, &c, 1, &wrote, 0);
    }
    return 0;
}