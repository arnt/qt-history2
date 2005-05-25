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

#include "qaxbindable.h"

#include <qmetaobject.h>

#include <qt_windows.h> // for IUnknown
#include "../shared/qaxtypes.h"

/*!
    \class QAxBindable
    \brief The QAxBindable class provides an interface between a
    QWidget and an ActiveX client.

    \inmodule QAxServer

    The functions provided by this class allow an ActiveX control to
    communicate property changes to a client application. Inherit
    your control class from both QWidget (directly or indirectly) and
    this class to get access to this class's functions. The
    \l{moc}{meta-object compiler} requires you to inherit from
    QWidget first.

    \code
        class MyActiveX : public QWidget, public QAxBindable
        {
            Q_OBJECT
            Q_PROPERTY(int value READ value WRITE setValue)

        public:
            MyActiveX(QWidget *parent = 0);
            ...

            int value() const;
            void setValue(int);
        };
    \endcode

    When implementing the property write function, use
    requestPropertyChange() to get permission from the ActiveX client
    application to change this property. When the property changes,
    call propertyChanged() to notify the ActiveX client application
    about the change. If a fatal error occurs in the control, use the
    static reportError() function to notify the client.

    Use the interface returned by clientSite() to call the ActiveX
    client. To implement additional COM interfaces in your ActiveX
    control, reimplement createAggregate() to return a new object of a
    QAxAggregated subclass.

    The ActiveQt \l{activeqt/opengl}{OpenGL} example shows how to use
    QAxBindable to implement additional COM interfaces.

    \sa QAxAggregated, QAxFactory, {ActiveQt Framework}
*/

/*!
    Constructs an empty QAxBindable object.
*/
QAxBindable::QAxBindable()
:activex(0)
{
}

/*!
    Destroys the QAxBindable object.
*/
QAxBindable::~QAxBindable()
{
}

/*!
    Call this function to request permission to change the property
    \a property from the client that is hosting this ActiveX control.
    Returns true if the client allows the change; otherwise returns
    false.

    This function is usually called first in the write function for \a
    property, and writing is abandoned if the function returns false.

    \code
        void MyActiveQt::setText(const QString &text)
        {
            if (!requestPropertyChange("text"))
                return;

            // update property

            propertyChanged("text");
        }
    \endcode

    \sa propertyChanged()
*/
bool QAxBindable::requestPropertyChange(const char *property)
{
    if (!activex)
        return true;
    
    return activex->emitRequestPropertyChange(property);
}

/*!
    Call this function to notify the client that is hosting this
    ActiveX control that the property \a property has been changed.

    This function is usually called at the end of the property's write
    function.

    \sa requestPropertyChange()
*/
void QAxBindable::propertyChanged(const char *property)
{
    if (!activex)
        return;
    
    activex->emitPropertyChanged(property);
}

/*!
    Returns a pointer to the client site interface for this ActiveX object,
    or null if no client site has been set.

    Call \c QueryInterface() on the returned interface to get the
    interface you want to call.
*/
IUnknown *QAxBindable::clientSite() const
{
    if (!activex)
        return 0;
    
    return activex->clientSite();
}

/*!
    Reimplement this function when you want to implement additional
    COM interfaces in the ActiveX control, or when you want to provide
    alternative implementations of COM interfaces. Return a new object
    of a QAxAggregated subclass.

    The default implementation returns the null pointer.
*/
QAxAggregated *QAxBindable::createAggregate()
{
    return 0;
}

/*!
    Reports an error to the client application. \a code is a
    control-defined error code. \a desc is a human-readable description
    of the error intended for the application user. \a src is the name
    of the source for the error, typically the ActiveX server name. \a
    context can be the location of a help file with more information
    about the error. If \a context ends with a number in brackets,
    e.g. [12], this number will be interpreted as the context ID in
    the help file.
*/
void QAxBindable::reportError(int code, const QString &src, const QString &desc, const QString &context)
{
    if (!activex)
        return;

    activex->reportError(code, src, desc, context);
}

/*!
    \class QAxAggregated
    \brief The QAxAggregated class is an abstract base class for implementations of
    additional COM interfaces.

    \inmodule QAxServer

    Create a subclass of QAxAggregated and reimplement
    queryInterface() to support additional COM interfaces. Use
    multiple inheritance from those COM interfaces. Implement the
    IUnknown interface of those COM interfaces by delegating the
    calls to \c QueryInterface(), \c AddRef() and \c Release() to the
    interface provided by controllingUnknown().

    Use the widget() method if you need to make calls to the QWidget
    implementing the ActiveX control. You must not store that pointer
    in your subclass (unless you use QPointer), as the QWidget can
    be destroyed by the ActiveQt framework at any time.

    \sa QAxBindable, QAxFactory, {ActiveQt Framework}
*/

/*!
    \fn QAxAggregated::~QAxAggregated()

    The destructor is called internally by Qt.
*/

/*!
    \fn long QAxAggregated::queryInterface(const QUuid &iid, void **iface)

    Reimplement this pure virtual function to support additional COM
    interfaces. Set the value of \a iface to point to this object to
    support the interface \a iid. Note that you must cast the \c
    this pointer to the appropriate superclass.

    \code
        long AxImpl::queryInterface(const QUuid &iid, void **iface)
        {
            *iface = 0;
            if (iid == IID_ISomeCOMInterface)
                *iface = (ISomeCOMInterface*)this;
            else
                return E_NOINTERFACE;

            AddRef();
            return S_OK;
        }
    \endcode

    Return the standard COM results \c S_OK (interface is supported)
    or \c E_NOINTERFACE (requested interface is not supported).

    \warning
    Even though you must implement the \c IUnknown interface if you
    implement any COM interface you must not support the \c IUnknown
    interface in your queryInterface() implementation.
*/

/*!
    \fn IUnknown *QAxAggregated::controllingUnknown() const

    Returns the \c IUnknown interface of the ActiveX control. Implement
    the \c IUnknown interface in your QAxAggregated subclass to
    delegate calls to \c QueryInterface(), \c AddRef(), and \c
    Release() to the interface provided by this function.

    \code
        HRESULT AxImpl::QueryInterface(REFIID iid, void **iface)
        {
            return controllingUnknown()->QueryInterface(iid, iface);
        }

        ulong AxImpl::AddRef()
        {
            return controllingUnknown()->AddRef();
        }

        ulong AxImpl::Release()
        {
            return controllingUnknown()->Release();
        }
    \endcode

    Instead of declaring and implementing these three functions
    manually, you can use the \c QAXAGG_IUNKNOWN macro in the class
    declaration of your subclass.
*/

/*!
    \fn QObject *QAxAggregated::object() const

    Returns a pointer to the QObject subclass implementing the COM object.
    This function might return 0.

    \warning
    You must not store the returned pointer, unless you use a
    QPointer, since the QObject can be destroyed by ActiveQt at any
    time.
*/

/*!
    \fn QWidget *QAxAggregated::widget() const

    Returns a pointer to the QWidget subclass implementing the ActiveX control.
    This function might return 0.

    \warning
    You must not store the returned pointer, unless you use a
    QPointer, since the QWidget can be destroyed by ActiveQt at any
    time.
*/
