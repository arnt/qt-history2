/****************************************************************************
** $Id: $
**
** Implementation of the QAxBindable classes
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qaxbindable.h"

#include <qintdict.h>
#include <qmetaobject.h>

#include <qt_windows.h> //IUnknown

struct IAxServerBase : public IUnknown
{
    virtual QObject *qObject() = 0;
    virtual QWidget *widget() = 0;
    virtual void emitPropertyChanged( const char*, long dispid = -1 ) = 0;
    virtual bool emitRequestPropertyChange( const char*, long dispid = -1 ) = 0;
};

/*!
    \class QAxBindable qaxbindable.h
    \brief The QAxBindable class provides an interface between the QWidget and the ActiveX client.

    \module QAxServer
    \extension ActiveQt

    The functions provided by this class allow the ActiveX control to
    communicate property changes to the client application. Inherit
    your control class from both QWidget (directly or indirectly) and
    this class to get access to this class's functions. The meta
    object compiler requires you to inherit from QWidget \e first.

    \code
    class MyActiveX : public QWidget, public QAxBindable
    {
	Q_OBJECT
	Q_PROPERTY( int value READ value WRITE setValue )
    public:
	MyActiveX( QWidget *parent = 0, const char *name = 0 );
	...

        int value() const;
	void setValue( int );
    };
    \endcode

    When implementing the property write function, use
    requestPropertyChange() to get permission from the ActiveX client
    application to change this property. When the property changes,
    call propertyChanged() to notify the ActiveX client application
    about the change.

    To implement additional COM interfaces in your ActiveX control, reimplement
    createAggregate() to return a new object of a QAxAggregated subclass.
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
    Returns TRUE if the client allows the change; otherwise returns
    FALSE.

    This function is usually called first in the write function for \a
    property, and writing is abandoned if the function returns FALSE.

    \code
    void MyActiveQt::setText( const QString &text )
    {
	if ( !requestPropertyChange( "text" ) )
	    return;

	// update property

	propertyChanged( "text" );
    }
    \endcode

    \sa propertyChanged()
*/
bool QAxBindable::requestPropertyChange( const char *property )
{
    if ( !activex )
	return TRUE;

    return activex->emitRequestPropertyChange( property );
}

/*!
    Call this function to notify the client that is hosting this
    ActiveX control that the property \a property has been changed.

    This function is usually called at the end of the property's write
    function.

    \sa requestPropertyChange()
*/
void QAxBindable::propertyChanged( const char *property )
{
    if ( !activex )
	return;

    activex->emitPropertyChanged( property );
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

/*! \class QAxAggregated qaxbindable.h
    \brief The QAxAggregated class is an abstract base class for implementations of
    additional COM interfaces.
    \module QAxServer
    \extension ActiveQt

    Create a subclass of QAxAggregated and reimplement queryInterface to support
    additional COM interfaces. Use multiple inheritance from those COM interfaces.
    Implement the IUnknown interface of those COM interfaces by delegating the
    calls to QueryInterface, AddRef and Release to the interface provided by
    controllingUnknown().
    
    Use the widget() method if you need to make calls to the QWidget implementing the 
    ActiveX control. You must not store that pointer in your subclass (unless you use
    QGuardedPtr), as the QWidget can be destroyed by the ActiveQt framework at any time.
*/

/*!
    \internal
    
    The destructor is called by the QAxServerBase, which is a friend.
*/
QAxAggregated::~QAxAggregated()
{
}

/*! \fn long QAxAggregated::queryInterface( const QUuid &iid, void **iface )

    Reimplement this pure virtual function to support additional COM interfaces.
    Set the value of \a iface to point to this object to support the interface \a iid.
    Note that you have to cast the \c this pointer to the respective superclass.

    \code
    long AxImpl::queryInterface( const QUuid &iid, void **iface )
    {
        *iface = 0;
	if ( iid == IID_ISomeCOMInterface )
	    *iface = (ISomeCOMInterface*)this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }
    \endcode

    Return the standard COM results S_OK (interface is supported) or E_NOINTERFACE
    (requested interface is not supported).

    \warning
    Even though you have to implement the IUnknown interface if you implement any
    COM interface you must not support the IUnknown interface in your queryInterface
    implementation.
*/

/*!
    \fn IUnknown *QAxAggregated::controllingUnknown() const

    Returns the IUnknown interface of the ActiveX control. Implement the IUnknown
    interface in your QAxAggregated subclass to delegate calls to QueryInterface, 
    AddRef and Release to the interface provided by this function.

    \code
    HRESULT AxImpl::QueryInterface( REFIID iid, void **iface )
    {
        return controllingUnknown()->QueryInterface( iid, iface );
    }

    unsigned long AxImpl::AddRef()
    {
        return controllingUnknown()->AddRef();
    }

    unsigned long AxImpl::Release()
    {
        return controllingUnknown()->Release();
    }
    \endcode

    The QAXAGG_IUNKNOWN macro expands to that, and you can use it in the
    class declaration of your subclass.
*/

/*!
    \fn QWidget *QAxAggregated::widget() const

    Returns a pointer to the QWidget subclass implementing the ActiveX control.
    This function might return zero.

    \warning
    You must not store the returned pointer, as the QWidget can be destroyed by 
    ActiveQt at any time.
*/
