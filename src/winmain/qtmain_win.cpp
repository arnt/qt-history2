/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qt_windows.h"

#include "qbytearray.h"
#include "qstring.h"
#include "qvector.h"


/*
  This file contains the code in the qtmain library for Windows.
  qtmain contains the Windows startup code and is required for
  linking to the Qt DLL.

  When a Windows application starts, the WinMain function is
  invoked. WinMain calls qWinMain in the Qt DLL/library, which
  initializes Qt.
*/

#if defined(Q_OS_TEMP)
extern void __cdecl qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);
#else
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);
#endif

#if defined(QT_NEEDS_QMAIN)
int qMain(int, char **);
#define main qMain
#else
#ifdef Q_OS_TEMP
extern "C" int __cdecl main(int, char **);
#else
extern "C" int main(int, char **);
#endif
#endif

/*
  WinMain() - Initializes Windows and calls user's startup function main().
  NOTE: WinMain() won't be called if the application was linked as a "console"
  application.
*/

#ifdef Q_OS_TEMP
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPWSTR /*wCmdParam*/, int cmdShow)
#else
extern "C"
int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdParamarg, int cmdShow)
#endif
{
    QByteArray cmdParam;
    QT_WA({
        LPTSTR cmdline = GetCommandLineW();
        cmdParam = QString::fromUtf16(cmdline).toLocal8Bit();
    }, {
        cmdParam = GetCommandLineA();
    });

    int argc = 0;
    QVector<char *> argv(8);
    qWinMain(instance, prevInstance, cmdParam.data(), cmdShow, argc, argv);

#ifdef Q_OS_TEMP
    TCHAR uniqueAppID[256];
    GetModuleFileName(0, uniqueAppID, 255);
    QString uid = QString::fromUcs2(uniqueAppID).lower().remove('\\');

    // If there exists an other instance of this application
    // it will be the owner of a mutex with the unique ID.
    HANDLE mutex = CreateMutex(NULL, true, uid.ucs2());
    if (mutex && ERROR_ALREADY_EXISTS == GetLastError()) {
        CloseHandle(mutex);

        // The app is already running, so we use the unique
        // ID to create a unique messageNo, which is used
        // as the registered class name for the windows
        // created. Set the first instance's window to the
        // foreground, else just terminate.
        UINT msgNo = RegisterWindowMessage(uid.ucs2());
        HWND aHwnd = FindWindow(QString::number(msgNo).ucs2(), 0);
        if (aHwnd)
            SetForegroundWindow(aHwnd);
        return 0;
    }
#endif // Q_OS_TEMP

    int result = main(argc, argv.data());
    return result;
}

