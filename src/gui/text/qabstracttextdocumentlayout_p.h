#ifndef QABSTRACTTEXTDOCUMENTLAYOUT_P_H
#define QABSTRACTTEXTDOCUMENTLAYOUT_P_H

#include <private/qobject_p.h>
#include <qhash.h>

struct QTextObjectHandler
{
    QTextObjectInterface *iface;
    QPointer<QObject> component;
};
typedef QHash<int, QTextObjectHandler> HandlerHash;

class QAbstractTextDocumentLayoutPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QAbstractTextDocumentLayout)

    HandlerHash handlers;

    void handlerDestroyed(QObject *obj);
};


#endif
