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
    virtual void emitPropertyChanged( long dispId ) = 0;
    virtual bool emitRequestPropertyChange( long dispId ) = 0;
    virtual QIntDict<QMetaProperty> *propertyList() = 0;
};

/*!
    \class QAxBindable qaxbindable.h
    \brief The QAxBindable class provides an interface between a Qt widget and an ActiveX control.

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

    To implement additional COM interfaces in your ActiveX control,
    reimplement queryInterface(). To make your ActiveX control a top
    level window, reimplement stayTopLevel() and return TRUE.
*/

/*!
    Constructs an empty QAxBindable object.
*/
QAxBindable::QAxBindable()
    :activex(0)
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

    DISPID dispId = -1;
    QIntDictIterator <QMetaProperty> it( *activex->propertyList() );
    while ( it.current() && dispId < 0 ) {
	QMetaProperty *mp = it.current();
	if ( !qstrcmp( property, mp->name() ) )
	    dispId = it.currentKey();
	++it;
    }

    return activex->emitRequestPropertyChange( dispId );
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

    DISPID dispId = -1;
    QIntDictIterator <QMetaProperty> it( *activex->propertyList() );
    while ( it.current() && dispId < 0 ) {
	QMetaProperty *mp = it.current();
	if ( !qstrcmp( property, mp->name() ) )
	    dispId = it.currentKey();
	++it;
    }

    activex->emitPropertyChanged( dispId );
}

/*!
    Reimplement this function if you want to support additional
    interfaces in your ActiveX control. Set \a iface to point to the
    interface implementation for interfaces of type \a iid.

    Make sure you add a reference to interfaces using the API provided
    by the COM interface model.

    \code
    QRESULT MyActiveX::queryInterface( const QUuid &iid, void **iface )
    {
        *iface = 0;
	if ( iid == IID_IStream )
	    *iface = (IStream*)this;
	else
	    return E_NOINTERFACE;

        AddRef();
	return S_OK;
    }
    \endcode

    Return the default COM results for QueryInterface (S_OK, E_NOINTERFACE).
*/
QRESULT QAxBindable::queryInterface( const QUuid &iid, void **iface )
{
    *iface = 0;
    return E_NOINTERFACE;
}

/*!
    Adds a reference to the ActiveX control. When implementing
    additional interfaces in your ActiveX class, implement AddRef() to
    call this function.

    \code
    long MyActiveX::AddRef()
    {
        return addRef();
    }
    \endcode

    \sa release(), queryInterface()
*/
long QAxBindable::addRef()
{
    return activex->AddRef();
}

/*!
    Removes a reference from the ActiveX control. When implementing
    additional interfaces in your ActiveX class, implement Release()
    to call this function.

    \code
    long MyActiveX::Release()
    {
        return release();
    }
    \endcode

    \sa addRef(), queryInterface()
*/
long QAxBindable::release()
{
    return activex->Release();
}

/*!
    Reimplement this function to return TRUE if the ActiveX control
    should be a top level window, e.g. a dialog. The default
    implementation returns FALSE.
*/
bool QAxBindable::stayTopLevel() const
{
    return FALSE;
}

/*!
    Reimplement this function to return TRUE if the ActiveX control
    should support the standard ActiveX events
    \list
    \i Click
    \i DblClick
    \i KeyDown
    \i KeyPress
    \i KeyUp
    \i MouseDown
    \i MouseUp
    \i MouseMove
    \endlist

    The default implementation returns FALSE.
*/
bool QAxBindable::hasStockEvents() const
{
    return FALSE;
}
