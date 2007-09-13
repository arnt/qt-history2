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

#ifndef QEVENT_P_H
#define QEVENT_P_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// ### Qt 5: remove
class Q_GUI_EXPORT QKeyEventEx : public QKeyEvent
{
public:
    QKeyEventEx(Type type, int key, Qt::KeyboardModifiers modifiers,
                const QString &text, bool autorep, ushort count,
                quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers);
    QKeyEventEx(const QKeyEventEx &other);

    ~QKeyEventEx();

protected:
    quint32 nScanCode;
    quint32 nVirtualKey;
    quint32 nModifiers;
    friend class QKeyEvent;
};

QT_END_NAMESPACE

#endif // QEVENT_P_H
