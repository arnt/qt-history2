/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef UNICODE
#define UNICODE
#endif

#ifndef QAXBASE_H
#define QAXBASE_H

#include <qdatastream.h>
#include <qmap.h>
#include <qobject.h>
#include <qvariant.h>

struct IUnknown;
struct QUuid;
class QAxEventSink;
class QAxObject;
class QAxBasePrivate;
struct QAxMetaObject;

class QAxBase
{
#ifdef Q_QDOC
#error "The Symbol Q_QDOC is reserved for documentation purposes."
    Q_PROPERTY(QString control READ control WRITE setControl)
#endif
public:
#ifndef Q_QDOC
    typedef QMap<QString, QVariant> PropertyBag;
#endif
    
    QAxBase(IUnknown *iface = 0);
    virtual ~QAxBase();
    
    QString control() const;
    
    long queryInterface(const QUuid &, void**) const;
    
    QVariant dynamicCall(const char *name, const QVariant &v1 = QVariant(), 
                                           const QVariant &v2 = QVariant(),
                                           const QVariant &v3 = QVariant(),
                                           const QVariant &v4 = QVariant(),
                                           const QVariant &v5 = QVariant(),
                                           const QVariant &v6 = QVariant(),
                                           const QVariant &v7 = QVariant(),
                                           const QVariant &v8 = QVariant());
    QVariant dynamicCall(const char *name, QList<QVariant> &vars);
    QAxObject *querySubObject(const char *name, const QVariant &v1 = QVariant(),
                                           const QVariant &v2 = QVariant(),
                                           const QVariant &v3 = QVariant(),
                                           const QVariant &v4 = QVariant(),
                                           const QVariant &v5 = QVariant(),
                                           const QVariant &v6 = QVariant(),
                                           const QVariant &v7 = QVariant(),
                                           const QVariant &v8 = QVariant());
    QAxObject* querySubObject(const char *name, QList<QVariant> &vars);
    
    virtual const QMetaObject *metaObject() const;
    virtual int qt_metacall(QMetaObject::Call, int, void **);

    virtual QObject *qObject() const = 0;
    virtual const char *className() const = 0;
    
    PropertyBag propertyBag() const;
    void setPropertyBag(const PropertyBag&);
    
    QString generateDocumentation();
    
    virtual bool propertyWritable(const char*) const;
    virtual void setPropertyWritable(const char*, bool);
    
    bool isNull() const;
    
    QVariant asVariant() const;
    
#ifdef Q_QDOC
#error "The Symbol Q_QDOC is reserved for documentation purposes."
    enum PropertyBag {};
signals:
    void signal(const QString&,int,void*);
    void propertyChanged(const QString&);
    void exception(int,const QString&,const QString&,const QString&);
#endif
    
public:
    virtual void clear();
    bool setControl(const QString&);

    void disableMetaObject();
    void disableClassInfo();
    void disableEventSink();
    
protected:
    virtual bool initialize(IUnknown** ptr);
    bool initializeRemote(IUnknown** ptr);
    bool initializeLicensed(IUnknown** ptr);
    bool initializeActive(IUnknown** ptr);
    bool initializeFromFile(IUnknown** ptr);

    void internalRelease();
    void connectNotify();
    
private:
    friend class QAxEventSink;
    bool initializeLicensedHelper(void *factory, const QString &key, IUnknown **ptr);
    QAxBasePrivate *d;
    QAxMetaObject *internalMetaObject() const;
    
    virtual const QMetaObject *parentMetaObject() const = 0;
    int internalProperty(QMetaObject::Call, int index, void **v);
    int internalInvoke(QMetaObject::Call, int index, void **v);
    bool dynamicCallHelper(const char *name, void *out, QList<QVariant> &var, QByteArray &type);

    static QMetaObject staticMetaObject;
};

#if defined Q_CC_MSVC && _MSC_VER < 1300
template <> inline QAxBase *qt_cast_helper<QAxBase*>(const QObject *o, QAxBase *)
#else
template <> inline QAxBase *qt_cast<QAxBase*>(const QObject *o)
#endif
{
    void *result = o ? const_cast<QObject *>(o)->qt_metacast("QAxBase") : 0;
    return (QAxBase*)(result);
}

#if defined Q_CC_MSVC && _MSC_VER < 1300
template <> inline QAxBase *qt_cast_helper<QAxBase*>(QObject *o, QAxBase *)
#else
template <> inline QAxBase *qt_cast<QAxBase*>(QObject *o)
#endif
{
    void *result = o ? o->qt_metacast("QAxBase") : 0;
    return (QAxBase*)(result);
}

inline QString QAxBase::generateDocumentation()
{
    extern QString qax_generateDocumentation(QAxBase *);
    return qax_generateDocumentation(this);
}

#ifndef QT_NO_DATASTREAM
inline QDataStream &operator >>(QDataStream &s, QAxBase &c)
{
    QAxBase::PropertyBag bag;
    c.qObject()->blockSignals(true);
    QString control;
    s >> control;
    c.setControl(control);
    s >> bag;
    c.setPropertyBag(bag);
    c.qObject()->blockSignals(false);
    
    return s;
}

inline QDataStream &operator <<(QDataStream &s, const QAxBase &c)
{
    QAxBase::PropertyBag bag = c.propertyBag();
    s << c.control();
    s << bag;
    
    return s;
}
#endif // QT_NO_DATASTREAM

#endif // QAXBASE_H
