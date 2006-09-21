/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsystemtrayicon_p.h"
//#define _WIN32_IE 0x0500
#define _WIN32_IE 0x0600 //required for NOTIFYICONDATAW_V2_SIZE
#include <QDesktopWidget>

//workaround for MINGW :
#ifndef NIN_BALLOONSHOW
#define NIN_BALLOONTIMEOUT  (WM_USER + 4)
#endif
#ifndef NIN_BALLOONUSERCLICK
#define NIN_BALLOONUSERCLICK (WM_USER + 5)
#endif

#include <qt_windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <QBitmap>
#include <QLibrary>
#include <QApplication>
#include <QToolTip>

static uint MYWM_TASKBARCREATED = 0;
#define MYWM_NOTIFYICON	(WM_APP+101)

typedef BOOL (WINAPI *PtrShell_NotifyIcon)(DWORD,PNOTIFYICONDATA);
static PtrShell_NotifyIcon ptrShell_NotifyIcon = 0;

static void resolveLibs()
{
    static bool triedResolve = false;
    if (!triedResolve) {
        QLibrary lib("shell32");
	triedResolve = true;
	ptrShell_NotifyIcon = (PtrShell_NotifyIcon) lib.resolve("Shell_NotifyIconW");
    }
}

class QSystemTrayIconSys : QWidget
{
public:
    QSystemTrayIconSys(QSystemTrayIcon *object);
    ~QSystemTrayIconSys();
    bool winEvent( MSG *m, long *result );
    bool trayMessageA(DWORD msg);
    bool trayMessageW(DWORD msg);
    bool trayMessage(DWORD msg);
    bool iconDrawItem(LPDRAWITEMSTRUCT lpdi);
    void setIconContentsW(NOTIFYICONDATAW &data);
    void setIconContentsA(NOTIFYICONDATAA &data);
    bool showMessageW(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon type, uint uSecs);
    bool showMessageA(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon type, uint uSecs);
    bool supportsMessages();
    QPoint findIconPosition(const int a_iButtonID);
    QRect findTrayGeometry();
    HBITMAP createIconMask(const QBitmap &bitmap);
    void createIcon();
    HICON hIcon;
    QPoint globalPos;
    QSystemTrayIcon *q;
private:
    uint notifyIconSizeW;
    uint notifyIconSizeA;

};

// Checks for the shell32 dll version number, since only version
// 5 or later of supports ballon messages
bool QSystemTrayIconSys::supportsMessages()
{
    HMODULE hmod = LoadLibraryA("shell32.dll");
    if (hmod)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hmod,
                          "DllGetVersion");
        if (pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;
            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);
            hr = (*pDllGetVersion)(&dvi);
            if (SUCCEEDED(hr)) {
                if (dvi.dwMajorVersion >= 5)
                {
#ifndef NOTIFYICONDATAW_V2_SIZE // for mingw which has version>=3 but has this  undefined
		    notifyIconSizeA = sizeof(NOTIFYICONDATAA);
                    notifyIconSizeW = sizeof(NOTIFYICONDATA);
#elif NOTIFYICON_VERSION >= 3
                    notifyIconSizeA = NOTIFYICONDATAA_V2_SIZE;
                    notifyIconSizeW = NOTIFYICONDATAW_V2_SIZE;
#endif
                    return true;
                }
            }
        }
    }
    return false;
}

QSystemTrayIconSys::QSystemTrayIconSys(QSystemTrayIcon *object)
    : hIcon(0), q(object)
{
    notifyIconSizeA = FIELD_OFFSET(NOTIFYICONDATAA, szTip[64]); // NOTIFYICONDATAA_V1_SIZE
    notifyIconSizeW = FIELD_OFFSET(NOTIFYICONDATAW, szTip[64]); // NOTIFYICONDATAW_V1_SIZE;

    // For restoring the tray icon after explorer crashes
    if (!MYWM_TASKBARCREATED) {
        MYWM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated");
    }
}

QSystemTrayIconSys::~QSystemTrayIconSys()
{
    if (hIcon)
	DestroyIcon(hIcon);
}

void QSystemTrayIconSys::setIconContentsW(NOTIFYICONDATAW &tnd)
{
    tnd.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
    tnd.uCallbackMessage = MYWM_NOTIFYICON;
    tnd.hIcon = hIcon;
    QString tip = q->toolTip();
    if (!tip.isNull()) {
        // Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
        tip = tip.left(63) + QChar();
        lstrcpynW(tnd.szTip, (TCHAR*)tip.utf16(), qMin(tip.length()+1, 64));
    }
}

void QSystemTrayIconSys::setIconContentsA(NOTIFYICONDATAA &tnd)
{
    tnd.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
    tnd.uCallbackMessage = MYWM_NOTIFYICON;
    tnd.hIcon = hIcon;
    QString tip = q->toolTip();
    if (!tip.isNull()) {
        // Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
        tip = tip.left(63) + QChar();
        lstrcpynA(tnd.szTip, tip.toLocal8Bit().constData(), qMin(tip.length()+1, 64));
    }
}

int iconFlag( QSystemTrayIcon::MessageIcon icon )
{
    int flag = 0;
#if NOTIFYICON_VERSION >= 3
    switch (icon) {
        case QSystemTrayIcon::NoIcon:
            break;
        case QSystemTrayIcon::Critical:
            flag = NIIF_ERROR;
            break;
        case QSystemTrayIcon::Warning:
            flag = NIIF_WARNING;
            break;
        case QSystemTrayIcon::Information:
        default : // fall through
            flag = NIIF_INFO;
    }
#endif
    return flag;
}

bool QSystemTrayIconSys::showMessageW(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon type, uint uSecs)
{
#if NOTIFYICON_VERSION>=3
    NOTIFYICONDATA tnd;
    memset(&tnd, 0, notifyIconSizeW);
    Q_ASSERT(testAttribute(Qt::WA_WState_Created));

    setIconContentsW(tnd);
    lstrcpynW(tnd.szInfo, (TCHAR*)message.utf16(), qMin(message.length() + 1, 256));
    lstrcpynW(tnd.szInfoTitle, (TCHAR*)title.utf16(), qMin(title.length() + 1, 64));
    tnd.uID = 0;
    tnd.dwInfoFlags = iconFlag(type);
    tnd.cbSize = notifyIconSizeW;
    tnd.hWnd = winId();
    tnd.uTimeout = uSecs;
    tnd.uFlags = NIF_INFO;
    tnd.uVersion = NOTIFYICON_VERSION;
    return ptrShell_NotifyIcon(NIM_MODIFY, &tnd);
#else
    return false;
#endif
}

bool QSystemTrayIconSys::showMessageA(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon type, uint uSecs)
{
#if NOTIFYICON_VERSION>=3
    NOTIFYICONDATAA tnd;
    memset(&tnd, 0, notifyIconSizeA);
    Q_ASSERT(testAttribute(Qt::WA_WState_Created));

    setIconContentsA(tnd);
    lstrcpynA(tnd.szInfo, message.toLocal8Bit().constData(), qMin(message.length() + 1, 256));
    lstrcpynA(tnd.szInfoTitle, title.toLocal8Bit().constData(), qMin(title.length() + 1, 64));
    tnd.uID = 0;
    tnd.dwInfoFlags = iconFlag(type);
    tnd.cbSize = notifyIconSizeA;
    tnd.hWnd = winId();
    tnd.uTimeout = uSecs;
    tnd.uFlags = NIF_INFO;
    tnd.uVersion = NOTIFYICON_VERSION;
    return Shell_NotifyIconA(NIM_MODIFY, &tnd);
#else
    return false;
#endif
}

bool QSystemTrayIconSys::trayMessageA(DWORD msg)
{
    NOTIFYICONDATAA tnd;
    memset(&tnd, 0, notifyIconSizeA);
    tnd.cbSize = notifyIconSizeA;
    tnd.hWnd = winId();
    Q_ASSERT(testAttribute(Qt::WA_WState_Created));

    if (msg != NIM_DELETE) {
        setIconContentsA(tnd);
    }
    return Shell_NotifyIconA(msg, &tnd);
}

bool QSystemTrayIconSys::trayMessageW(DWORD msg)
{
    NOTIFYICONDATAW tnd;
    memset(&tnd, 0, notifyIconSizeW);
    tnd.cbSize = notifyIconSizeW;
    tnd.hWnd = winId();
    Q_ASSERT(testAttribute(Qt::WA_WState_Created));

    if (msg != NIM_DELETE) {
        setIconContentsW(tnd);
    }
    return ptrShell_NotifyIcon(msg, &tnd);
}

bool QSystemTrayIconSys::trayMessage(DWORD msg)
{
    resolveLibs();
    if (!(ptrShell_NotifyIcon))
        return false;

    QT_WA({
        return trayMessageW(msg);
    },
    {
        return trayMessageA(msg);
    });
}

bool QSystemTrayIconSys::iconDrawItem(LPDRAWITEMSTRUCT lpdi)
{
    if (!hIcon)
	return false;

    DrawIconEx(lpdi->hDC, lpdi->rcItem.left, lpdi->rcItem.top, hIcon, 0, 0, 0, 0, DI_NORMAL);
    return true;
}

HBITMAP QSystemTrayIconSys::createIconMask(const QBitmap &bitmap)
{
    QImage bm = bitmap.toImage().convertToFormat(QImage::Format_Mono);
    int w = bm.width();
    int h = bm.height();
    int bpl = ((w+15)/16)*2;                        // bpl, 16 bit alignment
    uchar *bits = new uchar[bpl*h];
    bm.invertPixels();
    for (int y=0; y<h; y++)
        memcpy(bits+y*bpl, bm.scanLine(y), bpl);
    HBITMAP hbm = CreateBitmap(w, h, 1, 1, bits);
    delete [] bits;
    return hbm;
}

void QSystemTrayIconSys::createIcon()
{
    hIcon = 0;
    QIcon icon = q->icon();
    if (icon.isNull())
        return;

    QSize size = icon.actualSize(QSize(32, 32));
    QPixmap pm = icon.pixmap(size);
    if (pm.isNull())
        return;

    QBitmap mask = pm.mask();
    if (mask.isNull()) {
        mask = QBitmap(pm.size());
        mask.fill(Qt::color1);
    }

    HBITMAP im = createIconMask(mask);
    ICONINFO ii;
    ii.fIcon    = true;
    ii.hbmMask  = im;
    ii.hbmColor = pm.toWinHBITMAP(QPixmap::PremultipliedAlpha);
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    hIcon = CreateIconIndirect(&ii);

    DeleteObject(ii.hbmColor);
    DeleteObject(im);
}

bool QSystemTrayIconSys::winEvent( MSG *m, long *result )
{
    switch(m->message) {
    case WM_CREATE:
#ifdef GWLP_USERDATA
        SetWindowLongPtr(winId(), GWLP_USERDATA, (LONG_PTR)((CREATESTRUCTW*)m->lParam)->lpCreateParams);
#else
        SetWindowLong(winId(), GWL_USERDATA, (LONG)((CREATESTRUCTW*)m->lParam)->lpCreateParams);
#endif
        break;

    case WM_DRAWITEM:
	return iconDrawItem((LPDRAWITEMSTRUCT)m->lParam);

    case MYWM_NOTIFYICON:
	{
            RECT r;
            GetWindowRect(winId(), &r);
            QEvent *e = 0;
            Qt::KeyboardModifiers keys = QApplication::keyboardModifiers();
            QPoint gpos = QCursor::pos();

            switch (m->lParam) {
            case WM_LBUTTONDBLCLK:
                emit q->activated(QSystemTrayIcon::DoubleClick);
                break;

            case WM_LBUTTONUP:
                emit q->activated(QSystemTrayIcon::Trigger);
                break;

            case WM_RBUTTONUP:
                if (q->contextMenu()) {
                    q->contextMenu()->popup(gpos);
                    q->contextMenu()->activateWindow();
                    //Must be activated for proper keyboardfocus and menu closing on windows:
                }
                emit q->activated(QSystemTrayIcon::Context);
                break;

            case NIN_BALLOONUSERCLICK:
                emit q->messageClicked();
                break;

            case WM_MBUTTONUP:
                emit q->activated(QSystemTrayIcon::MiddleClick);
                break;

            default:
		        break;
	    }
	    if (e) {
		bool res = QApplication::sendEvent(q, e);
		delete e;
		return res;
	    }
	    break;
	}
    default:
	if (m->message == MYWM_TASKBARCREATED)
            trayMessage(NIM_ADD);
        else
            return QWidget::winEvent(m, result);
	break;
    }
    return 0;
}

void QSystemTrayIconPrivate::install_sys()
{
    Q_Q(QSystemTrayIcon);
    if (!sys) {
        sys = new QSystemTrayIconSys(q);
        sys->createIcon();
        sys->trayMessage(NIM_ADD);
    }
}

//fallback on win 95/98
QRect QSystemTrayIconSys::findTrayGeometry()
{
    //Use lower right corner as fallback
    QPoint brCorner = qApp->desktop()->screenGeometry().bottomRight();
    QRect ret(brCorner.x() - 10, brCorner.y() - 10, 10, 10);

    HWND trayHandle = FindWindowA("Shell_TrayWnd", NULL);
    if (trayHandle) {
        trayHandle = FindWindowExA(trayHandle, NULL, "TrayNotifyWnd", NULL);
        if (trayHandle) {
            RECT r;
            if (GetWindowRect(trayHandle, &r)) {
                ret = QRect(r.left, r.top, r.right- r.left, r.bottom - r.top);
            }
        }
    }
    return ret;
}

/*
* This function tries to determine the icon center position from the tray, in order to obtain
* the source of balloon tips, when emulated by Qt.
*
* If no visible icon is found a point centered on the left side of the tray is returned.
*
* If everything fails, (-1, -1) is returned.
*/
QPoint QSystemTrayIconSys::findIconPosition(const int iconId)
{
    QPoint ret(-1, -1);
    TBBUTTON buttonData;
    DWORD processID = 0;

    HWND trayHandle = FindWindowA("Shell_TrayWnd", NULL);

    //find the toolbar used in the notification area
    if (trayHandle) {
        trayHandle = FindWindowExA(trayHandle, NULL, "TrayNotifyWnd", NULL);
        if (trayHandle) {
            HWND hwnd = FindWindowEx(trayHandle, NULL, L"SysPager", NULL);
            if (hwnd) {
                hwnd = FindWindowEx(hwnd, NULL, L"ToolbarWindow32", NULL);
                if (hwnd)
                    trayHandle = hwnd;
            }
        }
    }

    if (!trayHandle)
        return ret;

    GetWindowThreadProcessId(trayHandle, &processID);
    if (processID <= 0)
        return ret;

    HANDLE trayProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, processID);
    if (!trayProcess)
        return ret;

    int buttonCount = SendMessage(trayHandle, TB_BUTTONCOUNT, 0, 0);
    LPVOID data = VirtualAllocEx(trayProcess, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);

    if ( buttonCount < 1 || !data ) {
	CloseHandle(trayProcess);
	return ret;
    }

    //search for our icon among all toolbar buttons
    for (int toolbarButton = 0; toolbarButton  < buttonCount; ++toolbarButton ) {
        SIZE_T numBytes = 0;
        DWORD appData[2] = { 0, 0 };
        SendMessage(trayHandle, TB_GETBUTTON, toolbarButton , (LPARAM)data);

        if(!ReadProcessMemory(trayProcess, data, &buttonData, sizeof(TBBUTTON), &numBytes))
            continue;

        if(!ReadProcessMemory(trayProcess, (LPVOID) buttonData.dwData, appData, sizeof(appData), &numBytes))
            continue;

        int currentIconId = appData[1];
        HWND currentIconHandle = (HWND) appData[0];
        bool isHidden = buttonData.fsState & TBSTATE_HIDDEN;

        if (currentIconHandle == winId() &&
            currentIconId == iconId && !isHidden) {
            SendMessage(trayHandle, TB_GETITEMRECT, toolbarButton , (LPARAM)data);
            RECT iconRect = {0, 0};
            if(ReadProcessMemory(trayProcess, data, &iconRect, sizeof(RECT), &numBytes)) {
    	        MapWindowPoints(trayHandle, NULL, (LPPOINT)&iconRect, 2);
                QRect geometry(iconRect.left, iconRect.top,
                                iconRect.right - iconRect.left,
                                iconRect.bottom - iconRect.top);
                ret = geometry.center();
                break;
            }
        }
    }
    VirtualFreeEx(trayProcess, data, 0, MEM_RELEASE);
    CloseHandle(trayProcess);
    return ret;
}


void QSystemTrayIconPrivate::showMessage_sys(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon type, int timeOut)
{
    if (!sys)
        return;

    uint uSecs = 0;
    if ( timeOut < 0)
        uSecs = 10000; //10 sec default
    else uSecs = (int)timeOut;

    resolveLibs();

    //message is limited to 255 chars + NULL
    QString messageString;
    if (message.isEmpty())
        messageString = " "; //ensures that the message shows when only title is set
    else
        messageString = message.left(255) + QChar();

    //title is limited to 63 chars + NULL
    QString titleString = title.left(63) + QChar();

    if (sys->supportsMessages()) {
        QT_WA({
            sys->showMessageW(titleString, messageString, type, (unsigned int)uSecs);
        }, {
            sys->showMessageA(titleString, messageString, type, (unsigned int)uSecs);
        });
    } else {
        //use fallbacks
        QPoint iconPos = sys->findIconPosition(0);
        if(iconPos != QPoint(-1, -1)) {
            QBalloonTip::showBalloon(type, title, message, sys->q, iconPos, uSecs, true);
        } else {
            QRect trayRect = sys->findTrayGeometry();
            QBalloonTip::showBalloon(type, title, message, sys->q, QPoint(trayRect.left(),
                                     trayRect.center().y()), uSecs, false);
        }
    }
}

QPoint QSystemTrayIconPrivate::globalPos_sys() const
{
    return QPoint();
}

void QSystemTrayIconPrivate::remove_sys()
{
    if (!sys)
	return;

    sys->trayMessage(NIM_DELETE);
    delete sys;
    sys = 0;
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
    if (!sys)
	return;

    if (sys->hIcon)
	DestroyIcon(sys->hIcon);

    sys->createIcon();
    sys->trayMessage(NIM_MODIFY);
}

void QSystemTrayIconPrivate::updateMenu_sys()
{

}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
    if (!sys)
	return;

    sys->trayMessage(NIM_MODIFY);
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
    return true;
}
