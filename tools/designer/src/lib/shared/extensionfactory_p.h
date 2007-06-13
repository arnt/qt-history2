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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef SHARED_EXTENSIONFACTORY_H
#define SHARED_EXTENSIONFACTORY_H

#include <QtDesigner/default_extensionfactory.h>
#include <QtDesigner/QExtensionManager>

namespace qdesigner_internal {

// Extension factory for registering an extension for an object type.
template <class ExtensionInterface, class Object, class Extension>
class ExtensionFactory: public QExtensionFactory
{
public:
    explicit ExtensionFactory(const QString &iid, QExtensionManager *parent = 0);

    // Convenience for registering the extension. Do not use for derived classes.
    static void registerExtension(QExtensionManager *mgr, const QString &iid);

protected:
    virtual QObject *createExtension(QObject *qObject, const QString &iid, QObject *parent) const;

private:
    // Can be overwritten to perform checks on the object.
    // Default does a qobject_cast to the desired class.
    virtual Object *checkObject(QObject *qObject) const;

    const QString m_iid;
};

template <class ExtensionInterface, class Object, class Extension>
ExtensionFactory<ExtensionInterface, Object, Extension>::ExtensionFactory(const QString &iid, QExtensionManager *parent) :
    QExtensionFactory(parent),
    m_iid(iid)
{
}

template <class ExtensionInterface, class Object, class Extension>
Object *ExtensionFactory<ExtensionInterface, Object, Extension>::checkObject(QObject *qObject) const
{
    return qobject_cast<Object*>(qObject);
}

template <class ExtensionInterface, class Object, class Extension>
QObject *ExtensionFactory<ExtensionInterface, Object, Extension>::createExtension(QObject *qObject, const QString &iid, QObject *parent) const
{
    if (iid != m_iid)
        return 0;

    Object *object = checkObject(qObject);
    if (!object)
        return 0;

    return new Extension(object, parent);
}

template <class ExtensionInterface, class Object, class Extension>
void ExtensionFactory<ExtensionInterface, Object, Extension>::registerExtension(QExtensionManager *mgr, const QString &iid)
{
    ExtensionFactory *factory = new ExtensionFactory(iid, mgr);
    mgr->registerExtensions(factory, iid);
}
}  // namespace qdesigner_internal

#endif // SHARED_EXTENSIONFACTORY_H
