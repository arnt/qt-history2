/****************************************************************************
**
** Implementation of the QAccessibleObject and QAccessibleApplication classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaccessibleobject.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qapplication.h"
#include "qwidget.h"

class QAccessibleObjectPrivate
{
public:
    QPointer<QObject> object;
};

/*!
    \class QAccessibleObject qaccessibleobject.h
    \brief The QAccessibleObject class implements parts of the
    QAccessibleInterface for QObjects.

    \ingroup misc

    This class is mainly provided for convenience. All subclasses of
    the QAccessibleInterface should use this class as the base class.
*/

extern void qInsertAccessibleObject(QObject *object, QAccessibleInterface *iface);
extern void qRemoveAccessibleObject(QObject *object);

/*!
    Creates a QAccessibleObject for \a object.
*/
QAccessibleObject::QAccessibleObject( QObject *object )
{
    d = new QAccessibleObjectPrivate;
    d->object = object;

    qInsertAccessibleObject(object, this);
}

/*!
    Destroys the QAccessibleObject.

    This only happens when a call to release() decrements the internal
    reference counter to zero.
*/
QAccessibleObject::~QAccessibleObject()
{
    qRemoveAccessibleObject(d->object);

    delete d;
}

/*!
    \reimp
*/
QRESULT QAccessibleObject::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QAccessible )
	*iface = (QAccessibleInterface*)this;
    else if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

/*!
    \reimp
*/
QObject *QAccessibleObject::object() const
{
#ifndef QT_NO_DEBUG
    if ( !isValid() )
	qWarning( "QAccessibleInterface is invalid. Crash pending..." );
#endif
    return d->object;
}

/*!
    \reimp
*/
bool QAccessibleObject::isValid() const
{
    return !d->object.isNull();
}


/*!
    \class QAccessibleApplication qaccessibleobject.h
    \brief The QAccessibleApplication class implements the QAccessibleInterface for QApplication.
    \internal
*/

/*!
    Creates a QAccessibleApplication for the QApplication object referenced by qApp.
*/
QAccessibleApplication::QAccessibleApplication()
: QAccessibleObject(qApp)
{
}

// all toplevel widgets except popups and the desktop
static QWidgetList topLevelWidgets()
{
    QWidgetList list;
    const QWidgetList tlw(qApp->topLevelWidgets());
    for (int i = 0; i < tlw.count(); ++i) {
	QWidget *w = tlw.at(i);
	if (!w->isPopup() && !w->isDesktop())
	    list.append(w);
    }

    return list;
}

/*! \reimp */
int QAccessibleApplication::childCount() const
{
    return topLevelWidgets().count();
}

/*! \reimp */
int QAccessibleApplication::indexOfChild(const QAccessibleInterface *child) const
{
    if (!child->object()->isWidgetType())
	return -1;

    const QWidgetList tlw(topLevelWidgets());
    int index = tlw.indexOf(static_cast<QWidget*>(child->object()));
    if (index != -1)
	++index;
    return index;
}

/*! \reimp */
bool QAccessibleApplication::queryChild( int control, QAccessibleInterface **iface ) const
{
    *iface = 0;
    if (!control)
	return FALSE;

    QObject *o = 0;
    const QWidgetList tlw(topLevelWidgets());
    if (tlw.count() >= control)
	o = tlw.at(control-1);
    if (!o )
	return FALSE;

    return QAccessible::queryAccessibleInterface( o, iface );
}

/*! \reimp */
bool QAccessibleApplication::queryParent( QAccessibleInterface **iface ) const
{
    *iface = 0;
    return FALSE;
}

/*! \reimp */
int QAccessibleApplication::childAt( int x, int y ) const
{
    const QWidgetList tlw(topLevelWidgets());
    for (int i = 0; i < tlw.count(); ++i) {
	QWidget *w = tlw.at(i);
	if (w->frameGeometry().contains(x,y))
	    return i+1;
    }
    return -1;
}

/*! \reimp */
QRect QAccessibleApplication::rect( int ) const
{
    return QRect();
}

/*! \reimp */
int QAccessibleApplication::navigate( NavDirection dir, int startControl ) const
{
#if defined(QT_DEBUG)
    if ( startControl )
	qWarning( "QAccessibleApplication::navigate: This implementation does not support subelements! (ID %d unknown for %s)", startControl, object()->className() );
#else
    Q_UNUSED(startControl);
#endif
    int count = childCount();
    switch ( dir ) {
    case NavFirstChild:
	return count ? 1 : -1;
    case NavLastChild:
	return count ? count : -1;
    case NavFocusChild:
	{
	    QWidget *actw = qApp->activeWindow();
	    if (!actw)
		return -1;
	    QAccessibleInterface *iface = 0;
	    QAccessible::queryAccessibleInterface(actw, &iface);
	    if (!iface)
		return -1;
	    int index = indexOfChild(iface);
	    iface->release();
	    return index;
	}
    default:
	qWarning( "QAccessibleApplication::navigate: unhandled request" );
	break;
    };
    return -1;
}

/*! \reimp */
QString QAccessibleApplication::text( Text t, int ) const
{
    switch (t) {
    case Name:
	if (qApp->mainWidget())
	    return qApp->mainWidget()->caption();
	break;
    case Description:
	return qApp->applicationFilePath();
	break;
    case DefaultAction:
	return QApplication::tr("Activate");
    }
    return QString();
}

/*! \reimp */
void QAccessibleApplication::setText( Text t, int, const QString &text )
{
}

/*! \reimp */
QAccessible::Role QAccessibleApplication::role( int ) const
{
    return Application;
}

/*! \reimp */
QAccessible::State QAccessibleApplication::state( int ) const
{
    return Normal;
}

/*! \reimp */
QMemArray<int> QAccessibleApplication::selection() const
{
    return QMemArray<int>();
}

/*! \reimp */
bool QAccessibleApplication::doDefaultAction( int child )
{
    return setFocus( child );
}

/*! \reimp */
bool QAccessibleApplication::setFocus( int )
{
    QWidget *w = qApp->mainWidget();
    if (!w)
	w = topLevelWidgets().at(0);
    if (!w)
	return FALSE;
    w->setActiveWindow();
    return TRUE;
}

/*! \reimp */
bool QAccessibleApplication::setSelected( int, bool, bool )
{
    return FALSE;
}

/*! \reimp */
void QAccessibleApplication::clearSelection()
{
}

#endif //QT_ACCESSIBILITY_SUPPORT
