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

#ifndef EXTENSION_H
#define EXTENSION_H

#include <QString>
#include <QObject>

#define Q_TYPEID(IFace) QLatin1String(IFace##_iid)

class Extensible
{
public:
    virtual ~Extensible() {}

    virtual QObject *extension(const QString &iid) const = 0;
};
Q_DECLARE_INTERFACE(Extensible, "http://trolltech.com/Qt/Extensible")

class ExtensionFactory
{
public:
    virtual ~ExtensionFactory() {}

    virtual QObject *extension(QObject *object, const QString &iid) const = 0;
};
Q_DECLARE_INTERFACE(ExtensionFactory, "http://trolltech.com/Qt/ExtensionFactory")

class ExtensionManager
{
public:
    virtual ~ExtensionManager() {}

    virtual void registerExtensions(ExtensionFactory *factory, const QString &iid) = 0;
    virtual void unregisterExtensions(ExtensionFactory *factory, const QString &iid) = 0;

    virtual QObject *extension(QObject *object, const QString &iid) const = 0;
};
Q_DECLARE_INTERFACE(ExtensionManager, "http://trolltech.com/Qt/ExtensionManager")

#if defined(Q_CC_MSVC) && (_MSC_VER < 1300)

template <class T>
inline T qt_extension_helper(ExtensionManager *, QObject *, T)
{ return 0; }

template <class T>
inline T qt_extension(ExtensionManager* manager, QObject *object)
{ return qt_extension_helper(manager, object, T(0)); }

#define Q_DECLARE_EXTENSION_INTERFACE(IFace, IId) \
Q_DECLARE_INTERFACE(IFace, IId) \
template <> inline IFace *qt_extension_helper<IFace *>(ExtensionManager *manager, QObject *object, IFace *) \
{ QObject *extension = manager->extension(object, IFace##_iid); return (IFace *)(extension ? extension->qt_metacast(IFace##_iid) : 0); }

#else

template <class T>
inline T qt_extension(ExtensionManager* manager, QObject *object)
{ return 0; }

#define Q_DECLARE_EXTENSION_INTERFACE(IFace, IId) \
Q_DECLARE_INTERFACE(IFace, IId) \
template <> inline IFace *qt_extension<IFace *>(ExtensionManager *manager, QObject *object) \
{ QObject *extension = manager->extension(object, IFace##_iid); return (IFace *)(extension ? extension->qt_metacast(IFace##_iid) : 0); }

#endif

#endif // EXTENSION_H
