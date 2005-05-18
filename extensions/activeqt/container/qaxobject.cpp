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

#include "qaxobject.h"

#include <quuid.h>
#include <qmetaobject.h>

/*!
    \class QAxObject
    \brief The QAxObject class provides a QObject that wraps a COM object.

    \inmodule QAxContainer

    A QAxObject can be instantiated as an empty object, with the name
    of the COM object it should wrap, or with a pointer to the
    IUnknown that represents an existing COM object. If the COM object
    implements the \c IDispatch interface, the properties, methods and
    events of that object become available as Qt properties, slots and
    signals. The base class, QAxBase, provides an API to access the
    COM object directly through the IUnknown pointer.

    QAxObject is a QObject and can be used as such, e.g. it can be
    organized in an object hierarchy, receive events and connect to
    signals and slots.

    QAxObject also inherits most of its ActiveX-related functionality
    from QAxBase, notably dynamicCall() and querySubObject().

    \warning
    You can subclass QAxObject, but you cannot use the Q_OBJECT macro
    in the subclass (the generated moc-file will not compile), so you
    cannot add further signals, slots or properties. This limitation is 
    due to the metaobject information generated in runtime. 
    To work around this problem, aggregate the QAxObject as a member of 
    the QObject subclass.

    \sa QAxBase, QAxWidget, QAxScript, {ActiveQt Framework}
*/

/*!
    Creates an empty COM object and propagates \a parent to the
    QObject constructor. To initialize the object, call \link
    QAxBase::setControl() setControl \endlink.
*/
QAxObject::QAxObject(QObject *parent)
: QObject(parent)
{
}

/*!
    Creates a QAxObject that wraps the COM object \a c. \a parent is
    propagated to the QWidget contructor.

    \sa setControl()
*/
QAxObject::QAxObject(const QString &c, QObject *parent)
: QObject(parent)
{
    setControl(c);
}

/*!
    Creates a QAxObject that wraps the COM object referenced by \a
    iface. \a parent is propagated to the QObject contructor.
*/
QAxObject::QAxObject(IUnknown *iface, QObject *parent)
: QObject(parent), QAxBase(iface)
{
}

/*!
    Releases the COM object and destroys the QAxObject,
    cleaning up all allocated resources.
*/
QAxObject::~QAxObject()
{
}

/*!
    \reimp
*/
const QMetaObject *QAxObject::metaObject() const
{
    return QAxBase::metaObject();
}

/*!
    \reimp
*/
const QMetaObject *QAxObject::parentMetaObject() const
{
    return &QObject::staticMetaObject;
}

/*!
    \internal
*/
void *QAxObject::qt_metacast(const char *cname)
{
    if (!qstrcmp(cname, "QAxObject")) return (void*)this;
    if (!qstrcmp(cname, "QAxBase")) return (QAxBase*)this;
    return QObject::qt_metacast(cname);
}

/*!
    \reimp
*/
const char *QAxObject::className() const
{
    return "QAxObject";
}

/*!
    \reimp
*/
int QAxObject::qt_metacall(QMetaObject::Call call, int id, void **v)
{
    id = QObject::qt_metacall(call, id, v);
    if (id < 0)
        return id;
    return QAxBase::qt_metacall(call, id, v);
}

/*!
    \fn QObject *QAxObject::qObject() const
    \internal
*/

/*!
    \reimp
*/
void QAxObject::connectNotify(const char *)
{
    QAxBase::connectNotify();
}
