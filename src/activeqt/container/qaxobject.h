/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QAXOBJECT_H
#define QAXOBJECT_H

#ifndef UNICODE
#define UNICODE
#endif

#include <ActiveQt/qaxbase.h>

QT_BEGIN_HEADER

QT_MODULE(ActiveQt)

class QAxObject : public QObject, public QAxBase
{
    friend class QAxEventSink;
public:
    const QMetaObject *metaObject() const;
    void* qt_metacast(const char*);
    int qt_metacall(QMetaObject::Call, int, void **);
    QObject* qObject() const { return (QObject*)this; }
    const char *className() const;
    
    QAxObject(QObject *parent = 0);
    QAxObject(const QString &c, QObject *parent = 0);
    QAxObject(IUnknown *iface, QObject *parent = 0);
    ~QAxObject();
    
    bool doVerb(const QString &verb);

protected:
    void connectNotify(const char *signal);

private:
    const QMetaObject *parentMetaObject() const;
    static QMetaObject staticMetaObject;
};

#if defined Q_CC_MSVC && _MSC_VER < 1300
template <> inline QAxObject *qobject_cast_helper<QAxObject*>(const QObject *o, QAxObject *)
#else
template <> inline QAxObject *qobject_cast<QAxObject*>(const QObject *o)
#endif
{
    void *result = o ? const_cast<QObject *>(o)->qt_metacast("QAxObject") : 0;
    return (QAxObject*)(result);
}

#if defined Q_CC_MSVC && _MSC_VER < 1300
template <> inline QAxObject *qobject_cast_helper<QAxObject*>(QObject *o, QAxObject *)
#else
template <> inline QAxObject *qobject_cast<QAxObject*>(QObject *o)
#endif
{
    void *result = o ? o->qt_metacast("QAxObject") : 0;
    return (QAxObject*)(result);
}

QT_END_HEADER

#endif // QAXOBJECT_H
