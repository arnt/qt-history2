/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhbox.cpp#6 $
**
** Implementation of hbox layout widget
**
** Created : 980220
**
** Copyright (C) 1996-1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qhbox.h"
#include "qlayout.h"

/*!
  \class QHBox qhbox.h
  \brief The QHBox widget performs geometry management on its children

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
    if ( !c->inserted() ) 
	return;
    QWidget *w = c->child();
    w->setAutoMinimumSize( TRUE );
    QSize sh = w->sizeHint();
    if ( !sh.isEmpty() )
	w->setMinimumSize( sh );
    lay->addWidget( w );
    if ( isVisible() )
	lay->activate();
}


/*!
  Provides childEvent() while waiting for Qt 2.0.
 */
bool QHBox::event( QEvent *e ) {
    switch ( e->type() ) {
    case Event_ChildInserted:
    case Event_ChildRemoved:
	childEvent( (QChildEvent*)e );
	return TRUE;
    default:
	return QWidget::event( e );
    }
}



/*!
  Adds infinitely stretchable space.
*/

void QHBox::addStretch()
{
    lay->addStretch( 1 );
    if ( isVisible() )
	lay->activate();
    
}
