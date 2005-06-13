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

// sdk
#include "abstractmetadatabase.h"
#include "abstractformeditor.h"

// extension
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/propertysheet.h>

// Qt
#include <QtCore/qdebug.h>

/*!
    \class QDesignerMetaDataBaseInterface
    \brief The QDesignerMetaDataBaseInterface class provides an interface to \QD's object meta
    database.
    \inmodule QtDesigner
*/

/*!
    Constructs an interface to the meta database with the given \a parent.
*/
QDesignerMetaDataBaseInterface::QDesignerMetaDataBaseInterface(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the interface to the meta database.
*/
QDesignerMetaDataBaseInterface::~QDesignerMetaDataBaseInterface()
{
}

/*!
    \fn virtual QDesignerMetaDataBaseInterface::QDesignerMetaDataBaseItemInterface *item(QObject *object) const = 0

    Returns the item in the meta database associated with the given \a object.
*/

/*!
    \fn virtual void QDesignerMetaDataBaseInterface::add(QObject *object) = 0

    Adds the specified \a object to the meta database.
*/

/*!
    \fn virtual void QDesignerMetaDataBaseInterface::remove(QObject *object) = 0

    Removes the specified \a object from the meta database.
*/

/*!
    \fn virtual QList<QObject*> QDesignerMetaDataBaseInterface::objects() const = 0

    Returns the list of objects that have corresponding items in the meta database.
*/

/*!
    \fn virtual QDesignerFormEditorInterface *QDesignerMetaDataBaseInterface::core() const = 0

    Returns the core interface that is associated with the meta database.
*/


// Doc: Interface only

/*!
    \class QDesignerMetaDataBaseItemInterface
    \brief The QDesignerMetaDataBaseItemInterface class provides an interface to individual items
    in \QD's meta database.
    \inmodule QtDesigner

    This class allows individual items in \QD's meta-data database to be accessed and modified.
    Use the QDesignerMetaDataBaseInterface class to change the properties of the database itself.
*/

/*!
    \fn virtual QDesignerMetaDataBaseItemInterface::~QDesignerMetaDataBaseItemInterface()

    Destroys the item interface to the meta-data database.
*/

/*!
    \fn virtual QString QDesignerMetaDataBaseItemInterface::name() const = 0

    Returns the name of the item in the database.

    \sa setName()
*/

/*!
    \fn virtual void QDesignerMetaDataBaseItemInterface::setName(const QString &name) = 0

    Sets the name of the item to the given \a name.

    \sa name()
*/

/*!
    \fn virtual QList<QWidget*> QDesignerMetaDataBaseItemInterface::tabOrder() const = 0

    Returns a list of widgets in the order defined by the form's tab order.

    \sa setTabOrder()
*/

/*!
    \fn virtual void QDesignerMetaDataBaseItemInterface::setTabOrder(const QList<QWidget*> &tabOrder) = 0

    Sets the tab order in the form using the list of widgets defined by \a tabOrder.

    \sa tabOrder()
*/

/*!
    \fn virtual bool QDesignerMetaDataBaseItemInterface::enabled() const = 0

    Returns whether the item is enabled.

    \sa setEnabled()
*/

/*!
    \fn virtual void QDesignerMetaDataBaseItemInterface::setEnabled(bool enabled) = 0

    If \a enabled is true, the item is enabled; otherwise it is disabled.

    \sa enabled()
*/
