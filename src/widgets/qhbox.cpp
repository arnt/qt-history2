/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhbox.cpp#13 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qhbox.h"
#include "qlayout.h"
#include "qapplication.h"
#include "qobjectlist.h"

/*!
  \class QHBox qhbox.h
  \brief The QHBox widget performs geometry management on its children

  \ingroup geomanagement

  All its children will be placed beside each other and sized
  according to their sizeHint()s.

  \sa QVBox and QGrid

*/


/*!
  Constructs an hbox widget with parent \a parent and name \a name
 */
QHBox::QHBox( QWidget *parent, const char *name, WFlags f,  bool allowLines  )
    :QFrame( parent, name, f, allowLines )
{
    lay = new QHBoxLayout( this, frameWidth(), frameWidth(), name );
}


/*!
  Constructs a horizontal hbox if \a horizontal is TRUE, otherwise
  constructs a vertical hbox (also known as a vbox).

  This constructor is provided for the QVBox class. You should never need
  to use it directly.
*/

QHBox::QHBox( bool horizontal, QWidget *parent , const char *name, WFlags f, bool allowLines )
    :QFrame( parent, name, f, allowLines )
{
    lay = new QBoxLayout( this,
		       horizontal ? QBoxLayout::LeftToRight : QBoxLayout::Down,
			  frameWidth(), frameWidth(), name );
}

/*!
  This function is called when the widget gets a new child or loses an old one.
 */
void QHBox::childEvent( QChildEvent *c )
{
    if ( !c->inserted() || !c->child()->isWidgetType() )
	return;
    QWidget *w = (QWidget*)c->child();
    lay->addWidget( w );
}

/*!
  Reimplemented for internal purposes
 */
void QHBox::frameChanged()
{
    bool horizontal = lay->direction() == QBoxLayout::LeftToRight;
    delete lay;
    lay = new QBoxLayout( this,
			  horizontal ? QBoxLayout::LeftToRight : QBoxLayout::Down,
			  frameWidth(), frameWidth(), name() );

    QObjectListIt it(*children());
    QObject* o;
    while ( ( o = it.current() ) ){
	++it;
	if ( o->isWidgetType() ) {
	    QWidget *w = (QWidget*)o;
	    lay->addWidget( w );
	}
    }
}
