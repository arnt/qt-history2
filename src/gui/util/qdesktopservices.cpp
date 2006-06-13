
#include "qdesktopservices.h"

#if defined(Q_WS_QWS)
#include "qdesktopservices_qws.cpp"
#elif defined(Q_WS_X11)
#include "qdesktopservices_x11.cpp"
#elif defined(Q_WS_WIN)
#include "qdesktopservices_win.cpp"
#elif defined(Q_WS_MAC)
#include "qdesktopservices_mac.cpp"
#endif

bool QDesktopServices::openUrl(const QUrl &url)
{
    if (url.scheme() == QLatin1String("file"))
        return openDocument(url);
    else
        return launchWebBrowser(url);
}
