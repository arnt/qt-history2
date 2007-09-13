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

#ifndef QDESKTOPSERVICES_H
#define QDESKTOPSERVICES_H

#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_DESKTOPSERVICES

class QStringList;
class QUrl;
class QObject;

class Q_GUI_EXPORT QDesktopServices
{
public:
    static bool openUrl(const QUrl &url);
    static void setUrlHandler(const QString &scheme, QObject *receiver, const char *method);
    static void unsetUrlHandler(const QString &scheme);
};

#endif // QT_NO_DESKTOPSERVICES

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDESKTOPSERVICES_H

