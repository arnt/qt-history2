/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhbox.cpp#1 $
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
  
  \sa QVBox and QHBox */


/*!
  Constructs an hbox widget with parent \a parent and name \a name
 */
QHBox::QHBox( QWidget *parent, const char *name )
    :QWidget( parent, name )
{
    lay = new QHBoxLayout( this, 5, -1, name ); //### border
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
    w->setMinimumSize( w->sizeHint() );
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
