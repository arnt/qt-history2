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
class QStringList;
class QUrl;

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_DESKTOPSERVICES

namespace QDesktopServices
{
    bool Q_GUI_EXPORT launchWebBrowser(const QUrl &url);
    bool Q_GUI_EXPORT openDocument(const QUrl &file);
};

#endif // QT_NO_DESKTOPSERVICES

QT_END_HEADER

#endif // QDESKTOPSERVICES_H

