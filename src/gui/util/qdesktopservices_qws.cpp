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

QT_BEGIN_NAMESPACE

static bool launchWebBrowser(const QUrl &url)
{
    Q_UNUSED(url);
    qWarning("QDesktopServices::launchWebBrowser not implemented");
    return false;
}

static bool openDocument(const QUrl &file)
{
    Q_UNUSED(file);
    qWarning("QDesktopServices::openDocument not implemented");
    return false;
}

QT_END_NAMESPACE

#endif // QT_NO_DESKTOPSERVICES

