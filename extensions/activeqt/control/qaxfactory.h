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

#ifndef QAXFACTORY_H
#define QAXFACTORY_H

#include <QtCore/qhash.h>
#include <QtCore/quuid.h>
#include <QtCore/qfactoryinterface.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qstringlist.h>


class QWidget;
struct QMetaObject;
class QSettings;
struct IUnknown;
struct IDispatch;

class QAxFactory
{
public:
    QAxFactory(const QUuid &libId, const QUuid &appId);
    virtual ~QAxFactory();
    
    virtual QStringList featureList() const = 0;

    virtual QObject *createObject(const QString &key) = 0;
    virtual const QMetaObject *metaObject(const QString &key) const = 0;
    virtual bool createObjectWrapper(QObject *object, IDispatch **wrapper);
    
    virtual QUuid classID(const QString &key) const;
    virtual QUuid interfaceID(const QString &key) const;
    virtual QUuid eventsID(const QString &key) const;
    
    virtual QUuid typeLibID() const;
    virtual QUuid appID() const;
    
    virtual void registerClass(const QString &key, QSettings *) const;
    virtual void unregisterClass(const QString &key, QSettings *) const;
    
    virtual bool validateLicenseKey(const QString &key, const QString &licenseKey) const;
    
    virtual QString exposeToSuperClass(const QString &key) const;
    virtual bool stayTopLevel(const QString &key) const;
    virtual bool hasStockEvents(const QString &key) const;
    virtual bool isService() const;
    
    enum ServerType {
        SingleInstance,
        MultipleInstances
    };
    
    static bool isServer();
    static QString serverDirPath();
    static QString serverFilePath();
    static bool startServer(ServerType type = MultipleInstances);
    static bool stopServer();

    static bool registerActiveObject(QObject *object);
    
private:
    QUuid typelib;
    QUuid app;
};

extern QAxFactory *qAxFactory();

inline bool QAxFactory::startServer(ServerType type)
{
    // implementation in qaxservermain.cpp
    extern bool qax_startServer(ServerType);
    return qax_startServer(type);
}

inline bool QAxFactory::stopServer()
{
    // implementation in qaxservermain.cpp
    extern bool qax_stopServer();
    return qax_stopServer();
}

#define QAXFACTORY_EXPORT(IMPL, TYPELIB, APPID)	\
    QAxFactory *qax_instantiate()		\
    {							\
        IMPL *impl = new IMPL(QUuid(TYPELIB), QUuid(APPID));	\
        return impl;					\
    }

#define QAXFACTORY_DEFAULT(Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp) \
    class QAxDefaultFactory : public QAxFactory \
    { \
    public: \
        QAxDefaultFactory(const QUuid &app, const QUuid &lib) \
        : QAxFactory(app, lib), className(#Class) {} \
        QStringList featureList() const \
        { \
            QStringList list; \
            list << className; \
            return list; \
        } \
        const QMetaObject *metaObject(const QString &key) const \
        { \
            if (key == className) \
            return &Class::staticMetaObject; \
            return 0; \
        } \
        QObject *createObject(const QString &key) \
        { \
            if (key == className) \
                return new Class(0); \
            return 0; \
        } \
        QUuid classID(const QString &key) const \
        { \
            if (key == className) \
                return QUuid(IIDClass); \
            return QUuid(); \
        } \
        QUuid interfaceID(const QString &key) const \
        { \
            if (key == className) \
                return QUuid(IIDInterface); \
            return QUuid(); \
        } \
        QUuid eventsID(const QString &key) const \
        { \
            if (key == className) \
                return QUuid(IIDEvents); \
            return QUuid(); \
        } \
    private: \
        QString className; \
    }; \
    QAXFACTORY_EXPORT(QAxDefaultFactory, IIDTypeLib, IIDApp) \

template<class T>
class QAxClass : public QAxFactory
{
public:
    QAxClass(const QString &libId, const QString &appId)
    : QAxFactory(libId, appId)
    {}
    
    const QMetaObject *metaObject(const QString &key) const { return &T::staticMetaObject; }
    QStringList featureList() const { return QStringList(QString(T::staticMetaObject.className())); }
    QObject *createObject(const QString &key)
    {
        const QMetaObject &mo = T::staticMetaObject;
        if (key != QLatin1String(mo.className()))
            return 0;
        if (!qstrcmp(mo.classInfo(mo.indexOfClassInfo("Creatable")).value(), "no"))
            return 0;
        return new T(0);
    }
};

#define QAXFACTORY_BEGIN(IDTypeLib, IDApp) \
    class QAxFactoryList : public QAxFactory \
    { \
        QStringList factoryKeys; \
        QHash<QString, QAxFactory*> factories; \
        QHash<QString, bool> creatable; \
    public: \
        QAxFactoryList() \
        : QAxFactory(IDTypeLib, IDApp) \
        { \
            QAxFactory *factory = 0; \
            QStringList keys; \
            QStringList::Iterator it; \

#define QAXCLASS(Class) \
            factory = new QAxClass<Class>(typeLibID(), appID()); \
            qRegisterMetaType<Class*>(#Class"*"); \
            keys = factory->featureList(); \
            for (it = keys.begin(); it != keys.end(); ++it) { \
                factoryKeys += *it; \
                factories.insert(*it, factory); \
                creatable.insert(*it, true); \
            }\

#define QAXTYPE(Class) \
            factory = new QAxClass<Class>(typeLibID(), appID()); \
            qRegisterMetaType<Class*>(#Class"*"); \
            keys = factory->featureList(); \
            for (it = keys.begin(); it != keys.end(); ++it) { \
                factoryKeys += *it; \
                factories.insert(*it, factory); \
                creatable.insert(*it, false); \
            }\

#define QAXFACTORY_END() \
        } \
        ~QAxFactoryList() { qDeleteAll(factories); } \
        QStringList featureList() const {  return factoryKeys; } \
        const QMetaObject *metaObject(const QString&key) const { \
            QAxFactory *f = factories[key]; \
            return f ? f->metaObject(key) : 0; \
        } \
        QObject *createObject(const QString &key) { \
            if (!creatable.value(key)) \
                return 0; \
            QAxFactory *f = factories[key]; \
            return f ? f->createObject(key) : 0; \
        } \
        QUuid classID(const QString &key) { \
            QAxFactory *f = factories.value(key); \
            return f ? f->classID(key) : QUuid(); \
        } \
        QUuid interfaceID(const QString &key) { \
            QAxFactory *f = factories.value(key); \
            return f ? f->interfaceID(key) : QUuid(); \
        } \
        QUuid eventsID(const QString &key) { \
            QAxFactory *f = factories.value(key); \
            return f ? f->eventsID(key) : QUuid(); \
        } \
        void registerClass(const QString &key, QSettings *s) const { \
            QAxFactory *f = factories.value(key); \
            if (f) f->registerClass(key, s); \
        } \
        void unregisterClass(const QString &key, QSettings *s) const { \
            QAxFactory *f = factories.value(key); \
            if (f) f->unregisterClass(key, s); \
        } \
        QString exposeToSuperClass(const QString &key) const { \
            QAxFactory *f = factories.value(key); \
            return f ? f->exposeToSuperClass(key) : QString(); \
        } \
        bool stayTopLevel(const QString &key) const { \
            QAxFactory *f = factories.value(key); \
            return f ? f->stayTopLevel(key) : false; \
        } \
        bool hasStockEvents(const QString &key) const { \
            QAxFactory *f = factories.value(key); \
            return f ? f->hasStockEvents(key) : false; \
        } \
    }; \
    QAxFactory *qax_instantiate()		\
    {							\
        QAxFactoryList *impl = new QAxFactoryList();	\
        return impl;					\
    }


Q_DECLARE_METATYPE(IUnknown*)
Q_DECLARE_METATYPE(IDispatch*)

#endif // QAXFACTORY_H
