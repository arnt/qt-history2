/****************************************************************************
**
** Implementation of QObjectCleanupHandler class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qobjectcleanuphandler.h"
#include "qlist.h"

/*!
    \class QObjectCleanupHandler qobjectcleanuphandler.h
    \brief The QObjectCleanupHandler class watches the lifetime of multiple QObjects.

    \ingroup objectmodel

    A QObjectCleanupHandler is useful whenever you need to know when a
    number of \l{QObject}s that are owned by someone else have been
    deleted. This is important, for example, when referencing memory
    in an application that has been allocated in a shared library.

    To keep track of some \l{QObject}s, create a
    QObjectCleanupHandler, and add() the objects you are interested
    in. If you are no longer interested in tracking a particular
    object, use remove() to remove it from the cleanup handler. If an
    object being tracked by the cleanup handler gets deleted by
    someone else it will automatically be removed from the cleanup
    handler. You can delete all the objects in the cleanup handler
    with clear(), or by destroying the cleanup handler. isEmpty()
    returns true if the QObjectCleanupHandler has no objects to keep
    track of.

    Example:

    \code
    class FactoryComponent : public FactoryInterface, public QLibraryInterface
    {
    public:
        ...

        QObject *createObject();

        bool init();
        void cleanup();
        bool canUnload() const;

    private:
        QObjectCleanupHandler objects;
    };

    // allocate a new object, and add it to the cleanup handler
    QObject *FactoryComponent::createObject()
    {
        return objects.add(new QObject());
    }

    // QLibraryInterface implementation
    bool FactoryComponent::init()
    {
        return true;
    }

    void FactoryComponent::cleanup()
    {
    }

    // it is only safe to unload the library when all QObject's have been destroyed
    bool FactoryComponent::canUnload() const
    {
        return objects.isEmpty();
    }
    \endcode
*/

/*!
    Constructs an empty QObjectCleanupHandler.
*/
QObjectCleanupHandler::QObjectCleanupHandler()
{
}

/*!
    Destroys the cleanup handler. All objects in this cleanup handler
    will be deleted.

    \sa clear()
*/
QObjectCleanupHandler::~QObjectCleanupHandler()
{
    clear();
}

/*!
    Adds \a object to this cleanup handler and returns the pointer to
    the object.

    \sa remove()
*/
QObject *QObjectCleanupHandler::add(QObject* object)
{
    if (!object)
        return 0;

    connect(object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
    cleanupObjects.insert(0, object);
    return object;
}

/*!
    Removes the \a object from this cleanup handler. The object will
    not be destroyed.

    \sa add()
*/
void QObjectCleanupHandler::remove(QObject *object)
{
    int index;
    if ((index = cleanupObjects.indexOf(object)) != -1) {
        cleanupObjects.remove(index);
        disconnect(object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
    }
}

/*!
    Returns true if this cleanup handler is empty or if all objects in
    this cleanup handler have been destroyed; otherwise return false.

    \sa add() remove() clear()
*/
bool QObjectCleanupHandler::isEmpty() const
{
    return cleanupObjects.isEmpty();
}

/*!
    Deletes all objects in this cleanup handler. The cleanup handler
    becomes empty.

    \sa isEmpty() ~QObjectCleanupHandler()
*/
void QObjectCleanupHandler::clear()
{
    while (!cleanupObjects.isEmpty())
        delete cleanupObjects.takeFirst();
}

void QObjectCleanupHandler::objectDestroyed(QObject *object)
{
    remove(object);
}
