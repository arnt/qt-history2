/****************************************************************************
**
** Implementation of QCoreApplication class for Windows.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcoreapplication.h"
#include "qt_windows.h"
#include "qvector.h"
#include "qmutex.h"


// ############### DONT EXPORT HERE!!!
Q_CORE_EXPORT char         appFileName[256];                // application file name
Q_CORE_EXPORT char         appName[256];                        // application name
Q_CORE_EXPORT HINSTANCE appInst        = 0;                // handle to app instance
Q_CORE_EXPORT HINSTANCE appPrevInst        = 0;                // handle to prev app instance
Q_CORE_EXPORT int appCmdShow = 0;

Q_CORE_EXPORT HINSTANCE qWinAppInst()                // get Windows app handle
{
    return appInst;
}

Q_CORE_EXPORT HINSTANCE qWinAppPrevInst()                // get Windows prev app handle
{
    return appPrevInst;
}

Q_CORE_EXPORT bool qt_winEventFilter(MSG* msg)
{
    return QCoreApplication::instance()->winEventFilter(msg);
}

static void        msgHandler(QtMsgType, const char*);

void set_winapp_name()
{
    static bool already_set = false;
    if (!already_set) {
        already_set = true;
#ifndef Q_OS_TEMP
        GetModuleFileNameA(0, appFileName, sizeof(appFileName));
#else
        QString afm;
        afm.setLength(256);
        afm.setLength(GetModuleFileName(0, (unsigned short*)afm.unicode(), 255));
        strncpy(appFileName, afm.latin1(), afm.length());
#endif
        const char *p = strrchr(appFileName, '\\');        // skip path
        if (p)
            memcpy(appName, p+1, qstrlen(p));
        int l = qstrlen(appName);
        if ((l > 4) && !qstricmp(appName + l - 4, ".exe"))
            appName[l-4] = '\0';                // drop .exe extension
    }
}

Q_CORE_EXPORT const char *qAppFileName()                // get application file name
{
    return appFileName;
}

Q_CORE_EXPORT const char *qAppName()                        // get application name
{
    if (!appName[0])
        set_winapp_name();
    return appName;
}


#if defined(Q_CC_MSVC) && !defined(Q_OS_TEMP)
#include <crtdbg.h>
#endif

static void msgHandler(QtMsgType t, const char* str)
{
    // OutputDebugString is not threadsafe.
    static QMutex staticMutex;

    if (!str)
        str = "(null)";

    staticMutex.lock();
    QT_WA({
        QString s(str);
        s += "\n";
        OutputDebugStringW((TCHAR*)s.ucs2());
    }, {
        QByteArray s(str);
        s += "\n";
        OutputDebugStringA(s.data());
    })
    staticMutex.unlock();
    if (t == QtFatalMsg)
#ifndef Q_OS_TEMP
#if defined(Q_CC_MSVC) && defined(_DEBUG) && defined(_CRT_ERROR)
        _CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, QT_VERSION_STR, str);
#else
        ExitProcess(1);
#endif
#else
        exit(1);
#endif
}


/*****************************************************************************
  qWinMain() - Initializes Windows. Called from WinMain() in qtmain_win.cpp
 *****************************************************************************/

#if defined(Q_OS_TEMP)
Q_CORE_EXPORT void __cdecl qWinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdParam,
               int cmdShow, int &argc, QVector<pchar> &argv)
#else
Q_CORE_EXPORT
void qWinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdParam,
               int cmdShow, int &argc, QVector<pchar> &argv)
#endif
{
    static bool already_called = false;

    if (already_called) {
        qWarning("Qt internal error: qWinMain should be called only once");
        return;
    }
    already_called = true;

  // Install default debug handler

    qInstallMsgHandler(msgHandler);

  // Create command line

    set_winapp_name();

    char *p = cmdParam;
    char *p_end = p + strlen(p);

    argc = 1;
    argv[0] = appFileName;

    while (*p && p < p_end) {                                // parse cmd line arguments
        while (isspace((uchar) *p))                        // skip white space
            p++;
        if (*p && p < p_end) {                                // arg starts
            int quote;
            char *start, *r;
            if (*p == '\"' || *p == '\'') {        // " or ' quote
                quote = *p;
                start = ++p;
            } else {
                quote = 0;
                start = p;
            }
            r = start;
            while (*p && p < p_end) {
                if (*p == '\\') {                // escape char?
                    p++;
                    if (*p == '\"' || *p == '\'')
                        ;                        // yes
                    else
                        p--;                        // treat \ literally
                } else if (quote) {
                    if (*p == quote) {
                        p++;
                        if (isspace((uchar) *p))
                            break;
                        quote = 0;
                    }
                } else {
                    if (*p == '\"' || *p == '\'') {        // " or ' quote
                        quote = *p++;
                        continue;
                    } else if (isspace((uchar) *p))
                        break;
                }
                if (p)
                    *r++ = *p++;
            }
            if (*p && p < p_end)
                p++;
            *r = '\0';

            if (argc >= (int)argv.size()-1)        // expand array
                argv.resize(argv.size()*2);
            argv[argc++] = start;
        }
    }
    argv[argc] = 0;
  // Get Windows parameters

    appInst = instance;
    appPrevInst = prevInstance;
    appCmdShow = cmdShow;

#ifdef Q_OS_TEMP
    TCHAR uniqueAppID[256];
    GetModuleFileName(0, uniqueAppID, 255);
    appUniqueID = RegisterWindowMessage(
                  QString::fromUcs2(uniqueAppID)
                  .lower().remove('\\').ucs2());
#endif
}

/*!
    The message procedure calls this function for every message
    received. Reimplement this function if you want to process window
    messages \e msg that are not processed by Qt. If you don't want
    the event to be processed by Qt, then return true; otherwise
    return false.
*/
bool QCoreApplication::winEventFilter(MSG * /*msg*/)        // Windows event filter
{
    return false;
}

#if defined(Q_WS_WIN) && !defined(QT_NO_DEBUG)
/*****************************************************************************
  Convenience functions for convert WM_* messages into human readable strings,
  including a nifty QDebug operator<< for simpel QDebug() << msg output.
 *****************************************************************************/
#include <windowsx.h>
#include "qdebug.h"
#if !defined(GET_X_LPARAM)
#  define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#  define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif
#ifdef _WIN32_WCE
#  ifndef WM_NCACTIVATE
#    define WM_NCACTIVATE 0x86
#  endif
#endif

// The values below should never change. Note that none of the usual
// WM_...FIRST & WM_...LAST values are in the list, as they normally have other
// WM_... representations
struct {
    uint WM;
    const char* str;
} knownWM[] =
{{ 0x0000, "WM_NULL" },
 { 0x0001, "WM_CREATE" },
 { 0x0002, "WM_DESTROY" },
 { 0x0003, "WM_MOVE" },
 { 0x0005, "WM_SIZE" },
 { 0x0006, "WM_ACTIVATE" },
 { 0x0007, "WM_SETFOCUS" },
 { 0x0008, "WM_KILLFOCUS" },
 { 0x000A, "WM_ENABLE" },
 { 0x000B, "WM_SETREDRAW" },
 { 0x000C, "WM_SETTEXT" },
 { 0x000D, "WM_GETTEXT" },
 { 0x000E, "WM_GETTEXTLENGTH" },
 { 0x000F, "WM_PAINT" },
 { 0x0010, "WM_CLOSE" },
 { 0x0011, "WM_QUERYENDSESSION" },
 { 0x0013, "WM_QUERYOPEN" },
 { 0x0016, "WM_ENDSESSION" },
 { 0x0012, "WM_QUIT" },
 { 0x0014, "WM_ERASEBKGND" },
 { 0x0015, "WM_SYSCOLORCHANGE" },
 { 0x0018, "WM_SHOWWINDOW" },
 { 0x001A, "WM_WININICHANGE" },
 { 0x001B, "WM_DEVMODECHANGE" },
 { 0x001C, "WM_ACTIVATEAPP" },
 { 0x001D, "WM_FONTCHANGE" },
 { 0x001E, "WM_TIMECHANGE" },
 { 0x001F, "WM_CANCELMODE" },
 { 0x0020, "WM_SETCURSOR" },
 { 0x0021, "WM_MOUSEACTIVATE" },
 { 0x0022, "WM_CHILDACTIVATE" },
 { 0x0023, "WM_QUEUESYNC" },
 { 0x0024, "WM_GETMINMAXINFO" },
 { 0x0026, "WM_PAINTICON" },
 { 0x0027, "WM_ICONERASEBKGND" },
 { 0x0028, "WM_NEXTDLGCTL" },
 { 0x002A, "WM_SPOOLERSTATUS" },
 { 0x002B, "WM_DRAWITEM" },
 { 0x002C, "WM_MEASUREITEM" },
 { 0x002D, "WM_DELETEITEM" },
 { 0x002E, "WM_VKEYTOITEM" },
 { 0x002F, "WM_CHARTOITEM" },
 { 0x0030, "WM_SETFONT" },
 { 0x0031, "WM_GETFONT" },
 { 0x0032, "WM_SETHOTKEY" },
 { 0x0033, "WM_GETHOTKEY" },
 { 0x0037, "WM_QUERYDRAGICON" },
 { 0x0039, "WM_COMPAREITEM" },
 { 0x003D, "WM_GETOBJECT" },
 { 0x0041, "WM_COMPACTING" },
 { 0x0044, "WM_COMMNOTIFY" },
 { 0x0046, "WM_WINDOWPOSCHANGING" },
 { 0x0047, "WM_WINDOWPOSCHANGED" },
 { 0x0048, "WM_POWER" },
 { 0x004A, "WM_COPYDATA" },
 { 0x004B, "WM_CANCELJOURNAL" },
 { 0x004E, "WM_NOTIFY" },
 { 0x0050, "WM_INPUTLANGCHANGEREQUEST" },
 { 0x0051, "WM_INPUTLANGCHANGE" },
 { 0x0052, "WM_TCARD" },
 { 0x0053, "WM_HELP" },
 { 0x0054, "WM_USERCHANGED" },
 { 0x0055, "WM_NOTIFYFORMAT" },
 { 0x007B, "WM_CONTEXTMENU" },
 { 0x007C, "WM_STYLECHANGING" },
 { 0x007D, "WM_STYLECHANGED" },
 { 0x007E, "WM_DISPLAYCHANGE" },
 { 0x007F, "WM_GETICON" },
 { 0x0080, "WM_SETICON" },
 { 0x0081, "WM_NCCREATE" },
 { 0x0082, "WM_NCDESTROY" },
 { 0x0083, "WM_NCCALCSIZE" },
 { 0x0084, "WM_NCHITTEST" },
 { 0x0085, "WM_NCPAINT" },
 { 0x0086, "WM_NCACTIVATE" },
 { 0x0087, "WM_GETDLGCODE" },
 { 0x0088, "WM_SYNCPAINT" },
 { 0x00A0, "WM_NCMOUSEMOVE" },
 { 0x00A1, "WM_NCLBUTTONDOWN" },
 { 0x00A2, "WM_NCLBUTTONUP" },
 { 0x00A3, "WM_NCLBUTTONDBLCLK" },
 { 0x00A4, "WM_NCRBUTTONDOWN" },
 { 0x00A5, "WM_NCRBUTTONUP" },
 { 0x00A6, "WM_NCRBUTTONDBLCLK" },
 { 0x00A7, "WM_NCMBUTTONDOWN" },
 { 0x00A8, "WM_NCMBUTTONUP" },
 { 0x00A9, "WM_NCMBUTTONDBLCLK" },
 { 0x00AB, "WM_NCXBUTTONDOWN" },
 { 0x00AC, "WM_NCXBUTTONUP" },
 { 0x00AD, "WM_NCXBUTTONDBLCLK" },
 { 0x00FF, "WM_INPUT" },
 { 0x0100, "WM_KEYDOWN" },
 { 0x0101, "WM_KEYUP" },
 { 0x0102, "WM_CHAR" },
 { 0x0103, "WM_DEADCHAR" },
 { 0x0104, "WM_SYSKEYDOWN" },
 { 0x0105, "WM_SYSKEYUP" },
 { 0x0106, "WM_SYSCHAR" },
 { 0x0107, "WM_SYSDEADCHAR" },
 { 0x0109, "WM_UNICHAR" },
 { 0x010D, "WM_IME_STARTCOMPOSITION" },
 { 0x010E, "WM_IME_ENDCOMPOSITION" },
 { 0x010F, "WM_IME_COMPOSITION" },
 { 0x0110, "WM_INITDIALOG" },
 { 0x0111, "WM_COMMAND" },
 { 0x0112, "WM_SYSCOMMAND" },
 { 0x0113, "WM_TIMER" },
 { 0x0114, "WM_HSCROLL" },
 { 0x0115, "WM_VSCROLL" },
 { 0x0116, "WM_INITMENU" },
 { 0x0117, "WM_INITMENUPOPUP" },
 { 0x011F, "WM_MENUSELECT" },
 { 0x0120, "WM_MENUCHAR" },
 { 0x0121, "WM_ENTERIDLE" },
 { 0x0122, "WM_MENURBUTTONUP" },
 { 0x0123, "WM_MENUDRAG" },
 { 0x0124, "WM_MENUGETOBJECT" },
 { 0x0125, "WM_UNINITMENUPOPUP" },
 { 0x0126, "WM_MENUCOMMAND" },
 { 0x0127, "WM_CHANGEUISTATE" },
 { 0x0128, "WM_UPDATEUISTATE" },
 { 0x0129, "WM_QUERYUISTATE" },
 { 0x0132, "WM_CTLCOLORMSGBOX" },
 { 0x0133, "WM_CTLCOLOREDIT" },
 { 0x0134, "WM_CTLCOLORLISTBOX" },
 { 0x0135, "WM_CTLCOLORBTN" },
 { 0x0136, "WM_CTLCOLORDLG" },
 { 0x0137, "WM_CTLCOLORSCROLLBAR" },
 { 0x0138, "WM_CTLCOLORSTATIC" },
 { 0x0200, "WM_MOUSEMOVE" },
 { 0x0201, "WM_LBUTTONDOWN" },
 { 0x0202, "WM_LBUTTONUP" },
 { 0x0203, "WM_LBUTTONDBLCLK" },
 { 0x0204, "WM_RBUTTONDOWN" },
 { 0x0205, "WM_RBUTTONUP" },
 { 0x0206, "WM_RBUTTONDBLCLK" },
 { 0x0207, "WM_MBUTTONDOWN" },
 { 0x0208, "WM_MBUTTONUP" },
 { 0x0209, "WM_MBUTTONDBLCLK" },
 { 0x020A, "WM_MOUSEWHEEL" },
 { 0x020B, "WM_XBUTTONDOWN" },
 { 0x020C, "WM_XBUTTONUP" },
 { 0x020D, "WM_XBUTTONDBLCLK" },
 { 0x0210, "WM_PARENTNOTIFY" },
 { 0x0211, "WM_ENTERMENULOOP" },
 { 0x0212, "WM_EXITMENULOOP" },
 { 0x0213, "WM_NEXTMENU" },
 { 0x0214, "WM_SIZING" },
 { 0x0215, "WM_CAPTURECHANGED" },
 { 0x0216, "WM_MOVING" },
 { 0x0218, "WM_POWERBROADCAST" },
 { 0x0219, "WM_DEVICECHANGE" },
 { 0x0220, "WM_MDICREATE" },
 { 0x0221, "WM_MDIDESTROY" },
 { 0x0222, "WM_MDIACTIVATE" },
 { 0x0223, "WM_MDIRESTORE" },
 { 0x0224, "WM_MDINEXT" },
 { 0x0225, "WM_MDIMAXIMIZE" },
 { 0x0226, "WM_MDITILE" },
 { 0x0227, "WM_MDICASCADE" },
 { 0x0228, "WM_MDIICONARRANGE" },
 { 0x0229, "WM_MDIGETACTIVE" },
 { 0x0230, "WM_MDISETMENU" },
 { 0x0231, "WM_ENTERSIZEMOVE" },
 { 0x0232, "WM_EXITSIZEMOVE" },
 { 0x0233, "WM_DROPFILES" },
 { 0x0234, "WM_MDIREFRESHMENU" },
 { 0x0281, "WM_IME_SETCONTEXT" },
 { 0x0282, "WM_IME_NOTIFY" },
 { 0x0283, "WM_IME_CONTROL" },
 { 0x0284, "WM_IME_COMPOSITIONFULL" },
 { 0x0285, "WM_IME_SELECT" },
 { 0x0286, "WM_IME_CHAR" },
 { 0x0288, "WM_IME_REQUEST" },
 { 0x0290, "WM_IME_KEYDOWN" },
 { 0x0291, "WM_IME_KEYUP" },
 { 0x02A0, "WM_NCMOUSEHOVER" },
 { 0x02A1, "WM_MOUSEHOVER" },
 { 0x02A2, "WM_NCMOUSELEAVE" },
 { 0x02A3, "WM_MOUSELEAVE" },
 { 0x02B1, "WM_WTSSESSION_CHANGE" },
 { 0x02C0, "WM_TABLET_FIRST" },
 { 0x02C1, "WM_TABLET_FIRST + 1" },
 { 0x02C2, "WM_TABLET_FIRST + 2" },
 { 0x02C3, "WM_TABLET_FIRST + 3" },
 { 0x02C4, "WM_TABLET_FIRST + 4" },
 { 0x02C5, "WM_TABLET_FIRST + 5" },
 { 0x02C6, "WM_TABLET_FIRST + 6" },
 { 0x02C7, "WM_TABLET_FIRST + 7" },
 { 0x02C8, "WM_TABLET_FIRST + 8" },
 { 0x02C9, "WM_TABLET_FIRST + 9" },
 { 0x02CA, "WM_TABLET_FIRST + 10" },
 { 0x02CB, "WM_TABLET_FIRST + 11" },
 { 0x02CC, "WM_TABLET_FIRST + 12" },
 { 0x02CD, "WM_TABLET_FIRST + 13" },
 { 0x02CE, "WM_TABLET_FIRST + 14" },
 { 0x02CF, "WM_TABLET_FIRST + 15" },
 { 0x02D0, "WM_TABLET_FIRST + 16" },
 { 0x02D1, "WM_TABLET_FIRST + 17" },
 { 0x02D2, "WM_TABLET_FIRST + 18" },
 { 0x02D3, "WM_TABLET_FIRST + 19" },
 { 0x02D4, "WM_TABLET_FIRST + 20" },
 { 0x02D5, "WM_TABLET_FIRST + 21" },
 { 0x02D6, "WM_TABLET_FIRST + 22" },
 { 0x02D7, "WM_TABLET_FIRST + 23" },
 { 0x02D8, "WM_TABLET_FIRST + 24" },
 { 0x02D9, "WM_TABLET_FIRST + 25" },
 { 0x02DA, "WM_TABLET_FIRST + 26" },
 { 0x02DB, "WM_TABLET_FIRST + 27" },
 { 0x02DC, "WM_TABLET_FIRST + 28" },
 { 0x02DD, "WM_TABLET_FIRST + 29" },
 { 0x02DE, "WM_TABLET_FIRST + 30" },
 { 0x02DF, "WM_TABLET_LAST" },
 { 0x0300, "WM_CUT" },
 { 0x0301, "WM_COPY" },
 { 0x0302, "WM_PASTE" },
 { 0x0303, "WM_CLEAR" },
 { 0x0304, "WM_UNDO" },
 { 0x0305, "WM_RENDERFORMAT" },
 { 0x0306, "WM_RENDERALLFORMATS" },
 { 0x0307, "WM_DESTROYCLIPBOARD" },
 { 0x0308, "WM_DRAWCLIPBOARD" },
 { 0x0309, "WM_PAINTCLIPBOARD" },
 { 0x030A, "WM_VSCROLLCLIPBOARD" },
 { 0x030B, "WM_SIZECLIPBOARD" },
 { 0x030C, "WM_ASKCBFORMATNAME" },
 { 0x030D, "WM_CHANGECBCHAIN" },
 { 0x030E, "WM_HSCROLLCLIPBOARD" },
 { 0x030F, "WM_QUERYNEWPALETTE" },
 { 0x0310, "WM_PALETTEISCHANGING" },
 { 0x0311, "WM_PALETTECHANGED" },
 { 0x0312, "WM_HOTKEY" },
 { 0x0317, "WM_PRINT" },
 { 0x0318, "WM_PRINTCLIENT" },
 { 0x0319, "WM_APPCOMMAND" },
 { 0x031A, "WM_THEMECHANGED" },
 { 0x0358, "WM_HANDHELDFIRST" },
 { 0x0359, "WM_HANDHELDFIRST + 1" },
 { 0x035A, "WM_HANDHELDFIRST + 2" },
 { 0x035B, "WM_HANDHELDFIRST + 3" },
 { 0x035C, "WM_HANDHELDFIRST + 4" },
 { 0x035D, "WM_HANDHELDFIRST + 5" },
 { 0x035E, "WM_HANDHELDFIRST + 6" },
 { 0x035F, "WM_HANDHELDLAST" },
 { 0x0360, "WM_AFXFIRST" },
 { 0x0361, "WM_AFXFIRST + 1" },
 { 0x0362, "WM_AFXFIRST + 2" },
 { 0x0363, "WM_AFXFIRST + 3" },
 { 0x0364, "WM_AFXFIRST + 4" },
 { 0x0365, "WM_AFXFIRST + 5" },
 { 0x0366, "WM_AFXFIRST + 6" },
 { 0x0367, "WM_AFXFIRST + 7" },
 { 0x0368, "WM_AFXFIRST + 8" },
 { 0x0369, "WM_AFXFIRST + 9" },
 { 0x036A, "WM_AFXFIRST + 10" },
 { 0x036B, "WM_AFXFIRST + 11" },
 { 0x036C, "WM_AFXFIRST + 12" },
 { 0x036D, "WM_AFXFIRST + 13" },
 { 0x036E, "WM_AFXFIRST + 14" },
 { 0x036F, "WM_AFXFIRST + 15" },
 { 0x0370, "WM_AFXFIRST + 16" },
 { 0x0371, "WM_AFXFIRST + 17" },
 { 0x0372, "WM_AFXFIRST + 18" },
 { 0x0373, "WM_AFXFIRST + 19" },
 { 0x0374, "WM_AFXFIRST + 20" },
 { 0x0375, "WM_AFXFIRST + 21" },
 { 0x0376, "WM_AFXFIRST + 22" },
 { 0x0377, "WM_AFXFIRST + 23" },
 { 0x0378, "WM_AFXFIRST + 24" },
 { 0x0379, "WM_AFXFIRST + 25" },
 { 0x037A, "WM_AFXFIRST + 26" },
 { 0x037B, "WM_AFXFIRST + 27" },
 { 0x037C, "WM_AFXFIRST + 28" },
 { 0x037D, "WM_AFXFIRST + 29" },
 { 0x037E, "WM_AFXFIRST + 30" },
 { 0x037F, "WM_AFXLAST" },
 { 0x0380, "WM_PENWINFIRST" },
 { 0x0381, "WM_PENWINFIRST + 1" },
 { 0x0382, "WM_PENWINFIRST + 2" },
 { 0x0383, "WM_PENWINFIRST + 3" },
 { 0x0384, "WM_PENWINFIRST + 4" },
 { 0x0385, "WM_PENWINFIRST + 5" },
 { 0x0386, "WM_PENWINFIRST + 6" },
 { 0x0387, "WM_PENWINFIRST + 7" },
 { 0x0388, "WM_PENWINFIRST + 8" },
 { 0x0389, "WM_PENWINFIRST + 9" },
 { 0x038A, "WM_PENWINFIRST + 10" },
 { 0x038B, "WM_PENWINFIRST + 11" },
 { 0x038C, "WM_PENWINFIRST + 12" },
 { 0x038D, "WM_PENWINFIRST + 13" },
 { 0x038E, "WM_PENWINFIRST + 14" },
 { 0x038F, "WM_PENWINLAST" },
 { 0x0400, "WM_USER" },
 { 0x8000, "WM_APP" },
 { 0,0 }}; // End of known messages

// Looks up the WM_ message in the table above
const char* findWMstr(uint msg)
{
    uint i = 0;
    const char* result = 0;
    // Known WM_'s
    while (knownWM[i].str && (knownWM[i].WM != msg))
        ++i;
    result = knownWM[i].str;
    return result;
};

// Convenience function for converting flags and values into readable strings
struct FLAG_STRING
{
    FLAG_STRING() { value=0; str=0; }
    FLAG_STRING(uint v, const char *s) { value=v; str=s; }
    uint value;
    const char* str;
};
#define FLGSTR(x) FLAG_STRING(x, #x)

// Returns an ORed (" | ") together string for the flags active in the actual
// value. (...) must consist of FLAG_STRING, with a FLAG_STRING() as the last
// value in the list passed to the function
QString flagCheck(uint actual, ...)
{
    va_list ap;
    va_start(ap, actual);

    QString result;
    int count = 0;
    FLAG_STRING v;
    while((v=va_arg(ap,FLAG_STRING)).str) {
        if ((actual & v.value) == v.value) {
            if (count++)
                result += " | ";
            result += v.str;
        }
    }
    va_end(ap);
    return result;
};

// Returns the string representation of the value in 'actual'. (...) must
// consist of FLAG_STRING, with a FLAG_STRING() as the last value in the list
// passed to the function
QString valueCheck(uint actual, ...)
{
    va_list ap;
    va_start(ap, actual);

    QString result;
    FLAG_STRING v;
    while((v=va_arg(ap,FLAG_STRING)).str && (actual != v.value))
        ;
    result = v.str;

    va_end(ap);
    return result;
};

// Returns a "human readable" string representation of the MSG and the
// information it points to
Q_CORE_EXPORT QString decodeMSG(const MSG& msg)
{
    const WPARAM wParam = msg.wParam;
    const LPARAM lParam = msg.lParam;
    QString wmmsg = findWMstr(msg.message);
    // Unknown WM_, so use number
    if (wmmsg.isEmpty())
        wmmsg = QString("WM_(%1)").arg(msg.message);

    QString rawParameters;
    rawParameters.sprintf("hwnd(0x%08x) ", msg.hwnd);

    // Custom WM_'s
    if (msg.message > WM_APP)
        wmmsg = QString("WM_APP + %1").arg(msg.message - WM_APP);
    else if (msg.message > WM_USER)
        wmmsg = QString("WM_USER + %1").arg(msg.message - WM_USER);

    QString parameters;
    switch (msg.message) {
#ifdef WM_ACTIVATE
        case WM_ACTIVATE:
            {
                QString activation = valueCheck(wParam,
                                                FLAG_STRING(WA_ACTIVE,      "Activate"),
                                                FLAG_STRING(WA_INACTIVE,    "Deactivate"),
                                                FLAG_STRING(WA_CLICKACTIVE, "Activate by mouseclick"),
                                                FLAG_STRING());
                parameters.sprintf("%s Hwnd (0x%08x)", activation.latin1(), msg.hwnd);
            }
            break;
#endif
#ifdef WM_CAPTURECHANGED
        case WM_CAPTURECHANGED:
            parameters.sprintf("Hwnd gaining capture (0x%08x)", lParam);
            break;
#endif
#ifdef WM_CREATE
        case WM_CREATE:
            {
                LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
                QString styles = flagCheck(lpcs->style,
                                           FLGSTR(WS_BORDER),
                                           FLGSTR(WS_CAPTION),
                                           FLGSTR(WS_CHILD),
                                           FLGSTR(WS_CLIPCHILDREN),
                                           FLGSTR(WS_CLIPSIBLINGS),
                                           FLGSTR(WS_DISABLED),
                                           FLGSTR(WS_DLGFRAME),
                                           FLGSTR(WS_GROUP),
                                           FLGSTR(WS_HSCROLL),
                                           FLGSTR(WS_OVERLAPPED),
#if defined(WS_OVERLAPPEDWINDOW) && (WS_OVERLAPPEDWINDOW != 0)
                                           FLGSTR(WS_OVERLAPPEDWINDOW),
#endif
#ifdef WS_ICONIC
                                           FLGSTR(WS_ICONIC),
#endif
                                           FLGSTR(WS_MAXIMIZE),
                                           FLGSTR(WS_MAXIMIZEBOX),
                                           FLGSTR(WS_MINIMIZE),
                                           FLGSTR(WS_MINIMIZEBOX),
                                           FLGSTR(WS_OVERLAPPEDWINDOW),
                                           FLGSTR(WS_POPUP),
#ifdef WS_POPUPWINDOW
                                           FLGSTR(WS_POPUPWINDOW),
#endif
                                           FLGSTR(WS_SIZEBOX),
                                           FLGSTR(WS_SYSMENU),
                                           FLGSTR(WS_TABSTOP),
                                           FLGSTR(WS_THICKFRAME),
#ifdef WS_TILED
                                           FLGSTR(WS_TILED),
#endif
#ifdef WS_TILEDWINDOW
                                           FLGSTR(WS_TILEDWINDOW),
#endif
                                           FLGSTR(WS_VISIBLE),
                                           FLGSTR(WS_VSCROLL),
                                           FLAG_STRING());

                QString exStyles = flagCheck(lpcs->dwExStyle,
#ifdef WS_EX_ACCEPTFILES
                                           FLGSTR(WS_EX_ACCEPTFILES),
#endif
#ifdef WS_EX_APPWINDOW
                                           FLGSTR(WS_EX_APPWINDOW),
#endif
                                           FLGSTR(WS_EX_CLIENTEDGE),
                                           FLGSTR(WS_EX_DLGMODALFRAME),
#ifdef WS_EX_LEFT
                                           FLGSTR(WS_EX_LEFT),
#endif
                                           FLGSTR(WS_EX_LEFTSCROLLBAR),
#ifdef WS_EX_LTRREADING
                                           FLGSTR(WS_EX_LTRREADING),
#endif
#ifdef WS_EX_MDICHILD
                                           FLGSTR(WS_EX_MDICHILD),
#endif
#ifdef WS_EX_NOACTIVATE
                                           FLGSTR(WS_EX_NOACTIVATE),
#endif
#ifdef WS_EX_NOANIMATION
                                           FLGSTR(WS_EX_NOANIMATION),
#endif
                                           FLGSTR(WS_EX_NOPARENTNOTIFY),
                                           FLGSTR(WS_EX_OVERLAPPEDWINDOW),
#ifdef WS_EX_PALETTEWINDOW
                                           FLGSTR(WS_EX_PALETTEWINDOW),
#endif
#ifdef WS_EX_RIGHT
                                           FLGSTR(WS_EX_RIGHT),
#endif
#ifdef WS_EX_RIGHTSCROLLBAR
                                           FLGSTR(WS_EX_RIGHTSCROLLBAR),
#endif
#ifdef WS_EX_RTLREADING
                                           FLGSTR(WS_EX_RTLREADING),
#endif
                                           FLGSTR(WS_EX_STATICEDGE),
                                           FLGSTR(WS_EX_TOOLWINDOW),
                                           FLGSTR(WS_EX_TOPMOST),
#ifdef WS_EX_TRANSPARENT
                                           FLGSTR(WS_EX_TRANSPARENT),
#endif
                                           FLGSTR(WS_EX_WINDOWEDGE),
#ifdef WS_EX_CAPTIONOKBTN
                                           FLGSTR(WS_EX_CAPTIONOKBTN),
#endif
                                           FLAG_STRING());

                QString className;
                if (lpcs->lpszClass != 0) {
                    if (HIWORD(lpcs->lpszClass) == 0) // Atom
                        className = QString::number(LOWORD(lpcs->lpszClass), 16);
                    else                              // String
                        className = QString((QChar*)lpcs->lpszClass,
                                            wcslen((unsigned short*)lpcs->lpszClass));
                }

                QString windowName;
                if (lpcs->lpszName != 0)
                    windowName = QString((QChar*)lpcs->lpszName,
                                         wcslen((unsigned short*)lpcs->lpszName));

                parameters.sprintf("x,y(%4d,%4d) w,h(%4d,%4d) className(%s) windowName(%s) parent(0x%08x) style(%s) exStyle(%s)",
                                    lpcs->x, lpcs->y, lpcs->cx, lpcs->cy, className.latin1(), windowName.latin1(),
                                    lpcs->hwndParent, styles.latin1(), exStyles.latin1());
            }
            break;
#endif
#ifdef WM_DESTROY
        case WM_DESTROY:
            parameters.sprintf("Destroy hwnd (0x%08x)", msg.hwnd);
            break;
#endif
#ifdef WM_IME_NOTIFY
        case WM_IME_NOTIFY:
            {
                QString imnCommand = valueCheck(wParam,
                                            FLGSTR(IMN_CHANGECANDIDATE),
                                            FLGSTR(IMN_CLOSECANDIDATE),
                                            FLGSTR(IMN_CLOSESTATUSWINDOW),
                                            FLGSTR(IMN_GUIDELINE),
                                            FLGSTR(IMN_OPENCANDIDATE),
                                            FLGSTR(IMN_OPENSTATUSWINDOW),
                                            FLGSTR(IMN_SETCANDIDATEPOS),
                                            FLGSTR(IMN_SETCOMPOSITIONFONT),
                                            FLGSTR(IMN_SETCOMPOSITIONWINDOW),
                                            FLGSTR(IMN_SETCONVERSIONMODE),
                                            FLGSTR(IMN_SETOPENSTATUS),
                                            FLGSTR(IMN_SETSENTENCEMODE),
                                            FLGSTR(IMN_SETSTATUSWINDOWPOS),
                                            FLAG_STRING());
                parameters.sprintf("Command(%s : 0x%08x)", imnCommand.latin1(), lParam);
            }
            break;
#endif
#ifdef WM_IME_SETCONTEXT
        case WM_IME_SETCONTEXT:
            {
                bool fSet = (BOOL)wParam;
                DWORD fShow = (DWORD)lParam;
                QString showFlgs = flagCheck(fShow,
#ifdef ISC_SHOWUICOMPOSITIONWINDOW
                                             FLGSTR(ISC_SHOWUICOMPOSITIONWINDOW),
#endif
#ifdef ISC_SHOWUIGUIDWINDOW
                                             FLGSTR(ISC_SHOWUIGUIDWINDOW),
#endif
#ifdef ISC_SHOWUISOFTKBD
                                             FLGSTR(ISC_SHOWUISOFTKBD),
#endif
                                             FLGSTR(ISC_SHOWUICANDIDATEWINDOW),
                                             FLGSTR(ISC_SHOWUICANDIDATEWINDOW << 1),
                                             FLGSTR(ISC_SHOWUICANDIDATEWINDOW << 2),
                                             FLGSTR(ISC_SHOWUICANDIDATEWINDOW << 3),
                                             FLAG_STRING());
                parameters.sprintf("Input context(%s) Show flags(%s)", (fSet?"Active":"Inactive"), showFlgs.latin1());
            }
            break;
#endif
#ifdef WM_KILLFOCUS
        case WM_KILLFOCUS:
            parameters.sprintf("Hwnd gaining keyboard focus (0x%08x)", wParam);
            break;
#endif
#ifdef WM_CHAR
        case WM_CHAR:
#endif
#ifdef WM_IME_CHAR
        case WM_IME_CHAR:
#endif
#ifdef WM_KEYDOWN
        case WM_KEYDOWN:
#endif
#ifdef WM_KEYUP
        case WM_KEYUP:
            {
                int nVirtKey     = (int)wParam;
                long lKeyData    = (long)lParam;
                int repCount     = (lKeyData & 0xffff);        // Bit 0-15
                int scanCode     = (lKeyData & 0xf0000) >> 16; // Bit 16-23
                bool contextCode = (lKeyData && 0x20000000);   // Bit 29
                bool prevState   = (lKeyData && 0x40000000);   // Bit 30
                bool transState  = (lKeyData && 0x80000000);   // Bit 31
                parameters.sprintf("Virual-key(0x%x) Scancode(%d) Rep(%d) Contextcode(%d), Prev state(%d), Trans state(%d)",
                                   nVirtKey, scanCode, repCount, contextCode, prevState, transState);
            }
            break;
#endif
#ifdef WM_NCACTIVATE
        case WM_NCACTIVATE:
            {
            parameters = (msg.wParam?"Active Titlebar":"Inactive Titlebar");
            }
            break;
#endif
#ifdef WM_MOUSEACTIVATE
        case WM_MOUSEACTIVATE:
            {
                QString mouseMsg = findWMstr(HIWORD(lParam));
                parameters.sprintf("TLW(0x%08x) HittestCode(0x%x) MouseMsg(%s)", wParam, LOWORD(lParam), mouseMsg.latin1());
            }
            break;
#endif
#ifdef WM_MOUSELEAVE
        case WM_MOUSELEAVE:
            break; // wParam & lParam not used
#endif
#ifdef WM_MOUSEHOVER
        case WM_MOUSEHOVER:
#endif
#ifdef WM_MOUSEWHEEL
        case WM_MOUSEWHEEL:
#endif
#ifdef WM_LBUTTONDBLCLK
        case WM_LBUTTONDBLCLK:
#endif
#ifdef WM_LBUTTONDOWN
        case WM_LBUTTONDOWN:
#endif
#ifdef WM_LBUTTONUP
        case WM_LBUTTONUP:
#endif
#ifdef WM_MBUTTONDBLCLK
        case WM_MBUTTONDBLCLK:
#endif
#ifdef WM_MBUTTONDOWN
        case WM_MBUTTONDOWN:
#endif
#ifdef WM_MBUTTONUP
        case WM_MBUTTONUP:
#endif
#ifdef WM_RBUTTONDBLCLK
        case WM_RBUTTONDBLCLK:
#endif
#ifdef WM_RBUTTONDOWN
        case WM_RBUTTONDOWN:
#endif
#ifdef WM_RBUTTONUP
        case WM_RBUTTONUP:
#endif
#ifdef WM_MOUSEMOVE
        case WM_MOUSEMOVE:
            {
                QString vrtKeys = flagCheck(wParam,
                                            FLGSTR(MK_CONTROL),
                                            FLGSTR(MK_LBUTTON),
                                            FLGSTR(MK_MBUTTON),
                                            FLGSTR(MK_RBUTTON),
                                            FLGSTR(MK_SHIFT),
#ifdef MK_XBUTTON1
                                            FLGSTR(MK_XBUTTON1),
#endif
#ifdef MK_XBUTTON2
                                            FLGSTR(MK_XBUTTON2),
#endif
                                            FLAG_STRING());
                parameters.sprintf("x,y(%4d,%4d) Virtual Keys(%s)", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), vrtKeys.latin1());
            }
            break;
#endif
#ifdef WM_MOVE
        case WM_MOVE:
            parameters.sprintf("x,y(%4d,%4d)", LOWORD(lParam), HIWORD(lParam));
            break;
#endif
#if defined(WM_PAINT) && defined(WM_ERASEBKGND)
        case WM_ERASEBKGND:
        case WM_PAINT:
            parameters.sprintf("hdc(0x%08x)", wParam);
            break;
#endif
#ifdef WM_QUERYNEWPALETTE
        case WM_QUERYNEWPALETTE:
            break; // lParam & wParam are unused
#endif
#ifdef WM_SETCURSOR
        case WM_SETCURSOR:
            {
                QString mouseMsg = findWMstr(HIWORD(lParam));
                parameters.sprintf("HitTestCode(0x%x) MouseMsg(%s)", LOWORD(lParam), mouseMsg.latin1());
            }
            break;
#endif
#ifdef WM_SETFOCUS
        case WM_SETFOCUS:
            parameters.sprintf("Lost Focus (0x%08x)", wParam);
            break;
#endif
#ifdef WM_SETTEXT
        case WM_SETTEXT:
            parameters.sprintf("Set Text (%s)", QString((QChar*)lParam, wcslen((unsigned short*)lParam)).latin1()); //Unicode string
            break;
#endif
#ifdef WM_SIZE
        case WM_SIZE:
            {
                QString showMode = valueCheck(wParam,
                                              FLGSTR(SIZE_MAXHIDE),
                                              FLGSTR(SIZE_MAXIMIZED),
                                              FLGSTR(SIZE_MAXSHOW),
                                              FLGSTR(SIZE_MINIMIZED),
                                              FLGSTR(SIZE_RESTORED),
                                              FLAG_STRING());

                parameters.sprintf("w,h(%4d,%4d) showmode(%s)", LOWORD(lParam), HIWORD(lParam), showMode.latin1());
            }
            break;
#endif
#ifdef WM_WINDOWPOSCHANGED
        case WM_WINDOWPOSCHANGED:
            {
                LPWINDOWPOS winPos = (LPWINDOWPOS)lParam;
                if (!winPos)
                    break;
                QString hwndAfter = valueCheck((uint)winPos->hwndInsertAfter,
                                          FLAG_STRING((uint)HWND_BOTTOM,    "HWND_BOTTOM"),
                                          FLAG_STRING((uint)HWND_NOTOPMOST, "HWND_NOTOPMOST"),
                                          FLAG_STRING((uint)HWND_TOP,       "HWND_TOP"),
                                          FLAG_STRING((uint)HWND_TOPMOST,   "HWND_TOPMOST"),
                                          FLAG_STRING());
                if (hwndAfter.size() == 0)
                    hwndAfter = QString::number((uint)winPos->hwndInsertAfter, 16);
                QString flags = flagCheck(winPos->flags,
                                          FLGSTR(SWP_DRAWFRAME),
                                          FLGSTR(SWP_FRAMECHANGED),
                                          FLGSTR(SWP_HIDEWINDOW),
                                          FLGSTR(SWP_NOACTIVATE),
#ifdef SWP_NOCOPYBITS
                                          FLGSTR(SWP_NOCOPYBITS),
#endif
                                          FLGSTR(SWP_NOMOVE),
                                          FLGSTR(SWP_NOOWNERZORDER),
                                          FLGSTR(SWP_NOREDRAW),
                                          FLGSTR(SWP_NOREPOSITION),
#ifdef SWP_NOSENDCHANGING
                                          FLGSTR(SWP_NOSENDCHANGING),
#endif
                                          FLGSTR(SWP_NOSIZE),
                                          FLGSTR(SWP_NOZORDER),
                                          FLGSTR(SWP_SHOWWINDOW),
                                          FLAG_STRING());
                parameters.sprintf("x,y(%4d,%4d) w,h(%4d,%4d) flags(%s) hwndAfter(%s)", winPos->x, winPos->y, winPos->cx, winPos->cy, flags.latin1(), hwndAfter.latin1());
            }
            break;
#endif
        default:
            parameters.sprintf("wParam(0x%08x) lParam(0x%08x)", wParam, lParam);
            break;
    }
    // Yes, we want to give the WM_ names 20 chars of space before showing the
    // decoded message, since some of the common messages are quite long, and
    // we don't want the decoded information to vary in output position
    QString message = QString("%1: ").arg(wmmsg, 20);
    message += rawParameters;
    message += parameters;
    return message;
}

Q_CORE_EXPORT QDebug operator<<(QDebug dbg, const MSG &msg)
{
    dbg << decodeMSG(msg);
    return dbg.nospace();
}
#endif
