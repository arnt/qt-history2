/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhbox.cpp#11 $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qhbox.h"
#include "qlayout.h"
#include "qapplication.h"

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
QHBox::QHBox( QWidget *parent, const char *name, WFlags f )
    :QWidget( parent, name, f )
{
    lay = new QHBoxLayout( this, parent?0:5, 5, name ); //### border
}


/*!
  Constructs a horizontal hbox if \a horizontal is TRUE, otherwise
  constructs a vertical hbox (also known as a vbox).

  This constructor is provided for the QVBox class. You should never need
  to use it directly.
*/

QHBox::QHBox( bool horizontal, QWidget *parent , const char *name, WFlags f )
    :QWidget( parent, name, f )
{
    lay = new QBoxLayout( this,
		       horizontal ? QBoxLayout::LeftToRight : QBoxLayout::Down,
			  parent?0:5, 5, name ); //### border
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
