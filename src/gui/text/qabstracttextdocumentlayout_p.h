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

#ifndef QABSTRACTTEXTDOCUMENTLAYOUT_P_H
#define QABSTRACTTEXTDOCUMENTLAYOUT_P_H

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

#include "private/qobject_p.h"
#include "QtCore/qhash.h"

struct QTextObjectHandler
{
    QTextObjectHandler() : iface(0) {}
    QTextObjectInterface *iface;
    QPointer<QObject> component;
};
typedef QHash<int, QTextObjectHandler> HandlerHash;

class QAbstractTextDocumentLayoutPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QAbstractTextDocumentLayout)

    inline QAbstractTextDocumentLayoutPrivate()
        : paintDevice(0) {}

    inline void setDocument(QTextDocument *doc) {
        document = doc;
        docPrivate = 0;
        if (doc)
            docPrivate = doc->docHandle();
    }

    inline int _q_dynamicPageCountSlot() const
    { return q_func()->pageCount(); }
    inline QSizeF _q_dynamicDocumentSizeSlot() const
    { return q_func()->documentSize(); }

    HandlerHash handlers;

    void _q_handlerDestroyed(QObject *obj);
    QPaintDevice *paintDevice;

    QTextDocument *document;
    QTextDocumentPrivate *docPrivate;
};

#endif // QABSTRACTTEXTDOCUMENTLAYOUT_P_H
