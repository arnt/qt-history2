/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QDBUSCONTEXT_P_H
#define QDBUSCONTEXT_P_H

class QDBusMessage;
class QDBusConnection;

class QDBusContext;
class QDBusContextPrivate
{
public:
    inline QDBusContextPrivate(const QDBusConnection &conn, const QDBusMessage &msg)
        : connection(conn), message(msg) {}

    QDBusConnection connection;
    const QDBusMessage &message;

    static QDBusContextPrivate *set(QObject *obj, QDBusContextPrivate *newContext);
};

#endif

