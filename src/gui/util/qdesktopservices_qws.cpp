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

#ifndef QT_NO_DESKTOPSERVICES

static bool launchWebBrowser(const QUrl &url)
{
    qWarning("QDesktopServices::launchWebBrowser not implemented");
    return false;
}

static bool openDocument(const QUrl &file)
{
    qWarning("QDesktopServices::openDocument not implemented");
    return false;
}

#endif // QT_NO_DESKTOPSERVICES

