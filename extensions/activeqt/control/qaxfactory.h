/****************************************************************************
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QAXFACTORY_H
#define QAXFACTORY_H

#include <qhash.h>
#include <quuid.h>
#include <private/qcom_p.h>
#include <qmetaobject.h>

// {22B230F6-8722-4051-ADCB-E7C9CE872EB3}
#ifndef IID_QAxFactory
#define IID_QAxFactory QUuid(0x22b230f6, 0x8722, 0x4051, 0xad, 0xcb, 0xe7, 0xc9, 0xce, 0x87, 0x2e, 0xb3)
#endif

class QWidget;
struct QMetaObject;
class QSettings;
struct IDispatch;

struct QAxFactoryInterface : public QFeatureListInterface
{
public:
#ifndef Q_QDOC
    virtual QObject *createObject(const QString &key) = 0;
    virtual const QMetaObject *metaObject(const QString &key) const = 0;
    virtual bool createObjectWrapper(QObject *object, IDispatch **wrapper) = 0;
    
    virtual QUuid classID(const QString &key) const = 0;
    virtual QUuid interfaceID(const QString &key) const = 0;
    virtual QUuid eventsID(const QString &key) const = 0;    
    virtual QUuid typeLibID() const = 0;
    virtual QUuid appID() const = 0;
    
    virtual void registerClass(const QString &key, QSettings *) const = 0;
    virtual void unregisterClass(const QString &key, QSettings *) const = 0;
    
    virtual bool validateLicenseKey(const QString &key, const QString &licenseKey) const = 0;
    
    virtual QString exposeToSuperClass(const QString &key) const = 0;
    virtual bool stayTopLevel(const QString &key) const = 0;
    virtual bool hasStockEvents(const QString &key) const = 0;
    virtual bool isService() const = 0;
#endif
};

extern QAxFactoryInterface *qAxFactory();

class QAxFactory : public QAxFactoryInterface
{
public:
    QAxFactory(const QUuid &, const QUuid &);
    virtual ~QAxFactory();
    Q_REFCOUNT;
    
    QRESULT queryInterface(const QUuid &iid, QUnknownInterface **iface);
    
#ifdef Q_QDOC
    virtual QStringList featureList() const = 0;
#endif
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
    
private:
    QUuid typelib;
    QUuid app;
};

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
    QUnknownInterface *ucm_instantiate()		\
    {							\
        IMPL *impl = new IMPL(QUuid(TYPELIB), QUuid(APPID));	\
        QUnknownInterface* iface = 0; 			\
        impl->queryInterface(IID_QUnknown, &iface);	\
        return iface;					\
    }

#define QAXFACTORY_DEFAULT(Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp) \
    class QAxDefaultFactory : public QAxFactory \
    { \
    public: \
        QAxDefaultFactory(const QUuid &app, const QUuid &lib) \
        : QAxFactory(app, lib), className(Class) {} \
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
                return new Class(parent, name); \
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
    QAxClass(const QString &appId, const QString &libId)
    : QAxFactory(appId, libId)
    {}
    
    const QMetaObject *metaObject(const QString &key) const { return &T::staticMetaObject; }
    QStringList featureList() const { return QString(T::staticMetaObject.className()); }
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
        QHash<QString, QAxFactoryInterface*> factories; \
        QHash<QString, bool> creatable; \
    public: \
        QAxFactoryList() \
        : QAxFactory(IDApp, IDTypeLib) \
        { \
            QAxFactoryInterface *factory = 0; \
            QStringList keys; \
            QStringList::Iterator it; \

#define QAXCLASS(Class) \
            factory = new QAxClass<Class>(appID(), typeLibID()); \
            keys = factory->featureList(); \
            for (it = keys.begin(); it != keys.end(); ++it) { \
                factoryKeys += *it; \
                factories.insert(*it, factory); \
                creatable.insert(*it, true); \
            }\

#define QAXTYPE(Class) \
            factory = new QAxClass<Class>(appID(), typeLibID()); \
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
            QAxFactoryInterface *f = factories[key]; \
            return f ? f->metaObject(key) : 0; \
        } \
        QObject *createObject(const QString &key) { \
            if (!creatable.value(key)) \
                return 0; \
            QAxFactoryInterface *f = factories[key]; \
            return f ? f->createObject(key) : 0; \
        } \
        QUuid classID(const QString &key) { \
            QAxFactoryInterface *f = factories.value(key); \
            return f ? f->classID(key) : QUuid(); \
        } \
        QUuid interfaceID(const QString &key) { \
            QAxFactoryInterface *f = factories.value(key); \
            return f ? f->interfaceID(key) : QUuid(); \
        } \
        QUuid eventsID(const QString &key) { \
            QAxFactoryInterface *f = factories.value(key); \
            return f ? f->eventsID(key) : QUuid(); \
        } \
        void registerClass(const QString &key, QSettings *s) const { \
            QAxFactoryInterface *f = factories.value(key); \
            if (f) f->registerClass(key, s); \
        } \
        void unregisterClass(const QString &key, QSettings *s) const { \
            QAxFactoryInterface *f = factories.value(key); \
            if (f) f->unregisterClass(key, s); \
        } \
        QString exposeToSuperClass(const QString &key) const { \
            QAxFactoryInterface *f = factories.value(key); \
            return f ? f->exposeToSuperClass(key) : QString(); \
        } \
        bool stayTopLevel(const QString &key) const { \
            QAxFactoryInterface *f = factories.value(key); \
            return f ? f->stayTopLevel(key) : false; \
        } \
        bool hasStockEvents(const QString &key) const { \
            QAxFactoryInterface *f = factories.value(key); \
            return f ? f->hasStockEvents(key) : false; \
        } \
    }; \
    QUnknownInterface *ucm_instantiate()		\
    {							\
        QAxFactoryList *impl = new QAxFactoryList();	\
        QUnknownInterface* iface = 0; 			\
        impl->queryInterface(IID_QUnknown, &iface);	\
        return iface;					\
    }


#endif // QAXFACTORY_H
