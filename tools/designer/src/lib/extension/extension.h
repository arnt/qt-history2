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

#include <QtCore/QString>
#include <QtCore/QObject>

#define Q_TYPEID(IFace) QLatin1String(IFace##_iid)

class QAbstractExtensionFactory
{
public:
    virtual ~QAbstractExtensionFactory() {}

    virtual QObject *extension(QObject *object, const QString &iid) const = 0;
};
Q_DECLARE_INTERFACE(QAbstractExtensionFactory, "com.trolltech.Qt.QAbstractExtensionFactory")

class QAbstractExtensionManager
{
public:
    virtual ~QAbstractExtensionManager() {}

    virtual void registerExtensions(QAbstractExtensionFactory *factory, const QString &iid) = 0;
    virtual void unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid) = 0;

    virtual QObject *extension(QObject *object, const QString &iid) const = 0;
};
Q_DECLARE_INTERFACE(QAbstractExtensionManager, "com.trolltech.Qt.QAbstractExtensionManager")

#if defined(Q_CC_MSVC) && (_MSC_VER < 1300)

template <class T>
inline T qt_extension_helper(QAbstractExtensionManager *, QObject *, T)
{ return 0; }

template <class T>
inline T qt_extension(QAbstractExtensionManager* manager, QObject *object)
{ return qt_extension_helper(manager, object, T(0)); }

#define Q_DECLARE_EXTENSION_INTERFACE(IFace, IId) \
Q_DECLARE_INTERFACE(IFace, IId) \
template <> inline IFace *qt_extension_helper<IFace *>(QAbstractExtensionManager *manager, QObject *object, IFace *) \
{ QObject *extension = manager->extension(object, Q_TYPEID(IFace)); return (IFace *)(extension ? extension->qt_metacast(IFace##_iid) : 0); }

#else

template <class T>
inline T qt_extension(QAbstractExtensionManager* manager, QObject *object)
{ return 0; }

#define Q_DECLARE_EXTENSION_INTERFACE(IFace, IId) \
Q_DECLARE_INTERFACE(IFace, IId) \
template <> inline IFace *qt_extension<IFace *>(QAbstractExtensionManager *manager, QObject *object) \
{ QObject *extension = manager->extension(object, Q_TYPEID(IFace)); return extension ? static_cast<IFace *>(extension->qt_metacast(IFace##_iid)) : static_cast<IFace *>(0); }

#endif

#endif // EXTENSION_H
