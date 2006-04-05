/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
****************************************************************************/

#ifndef QDESKTOP_H
#define QDESKTOP_H

#include <qstringlist.h>
#include <qurl.h>

#ifndef QT_NO_DESKTOP

namespace QDesktop {
    Q_GUI_EXPORT bool launchMailComposer(const QStringList &to, const QString &subject, const QString &body = QString());
    Q_GUI_EXPORT bool launchMailComposer(const QString &to, const QString &subject, const QString &body = QString());

    Q_GUI_EXPORT bool launchWebBrowser(const QUrl &url);
    Q_GUI_EXPORT bool open(const QUrl &file);
};

#endif // QT_NO_DESKTOP
#endif // QDESKTOP_H

