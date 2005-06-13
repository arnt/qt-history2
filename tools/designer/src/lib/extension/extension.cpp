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

#include <QtDesigner/extension.h>

/*!
    \class QAbstractExtensionFactory
    \brief The QAbstractExtensionFactory class provides the standard interface for extension
    factories in \QD.
    \inmodule QtDesigner
*/

/*!
    \fn virtual QAbstractExtensionFactory::~QAbstractExtensionFactory()

    Destroys the extension factory.
*/

/*!
    \fn virtual QObject *QAbstractExtensionFactory::extension(QObject *object, const QString &iid) const = 0

    Returns an extension for the given \a object with an extension identifier specified by \a iid.
*/


/*!
    \class QAbstractExtensionManager
    \brief The QAbstractExtensionManager class provides the abstract interface for extension
    managers in \QD.
    \inmodule QtDesigner
*/

/*!
    \fn virtual QAbstractExtensionManager::~QAbstractExtensionManager()

    Destroys the extension manager.
*/

/*!
    \fn virtual void QAbstractExtensionManager::registerExtensions(QAbstractExtensionFactory *factory, const QString &iid) = 0
*/

/*!
    \fn virtual void QAbstractExtensionManager::unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid) = 0
*/

/*!
    \fn virtual QObject *QAbstractExtensionManager::extension(QObject *object, const QString &iid) const = 0
*/
