/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qgrid.cpp#1 $
**
** Implementation of grid layout widget
**
** Created : 980220
**
** Copyright (C) 1996-1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qgrid.h"
#include "qlayout.h"

/*!
  \class QGrid qgrid.h
  \brief The QGrid widget performs geometry management on its children

  The number of rows and columns is defined in the constructor. All its
  children will be placed and sized according to their sizeHint()s.
  
  \sa QVBox and QHBox
 */



/*!
  Constructs a \a rows x \a cols grid widget with parent \a parent and name \a name
 */
QGrid::QGrid( int rows, int cols, QWidget *parent, const char *name )
    :QWidget( parent, name )
{
    lay = new QGridLayout( this, rows, cols, 5, -1, name ); //### border
    nCols = cols;
    nRows = rows;
    row = col = 0;
}


/*!
  This function is called when the widget gets a new child or loses an old one.
 */
void QGrid::childEvent( QChildEvent *c )
{
    if ( !c->inserted() ) 
	return;
    QWidget *w = c->child();
    w->setAutoMinimumSize( TRUE );
    w->setMinimumSize( w->sizeHint() );
    lay->addWidget( w, row, col );
    //    debug( "adding %d,%d", row, col );
    if ( col+1 < nCols ) {
	col++;
    } else if ( row+1 < nRows ) {
	col = 0;
	row++;
    }
    if ( isVisible() )
	lay->activate();
}


/*!
  Provides childEvent() while waiting for Qt 2.0.
 */
bool QGrid::event( QEvent *e ) {
    switch ( e->type() ) {
    case Event_ChildInserted:
    case Event_ChildRemoved:
	childEvent( (QChildEvent*)e );
	return TRUE;
    default:
	return QWidget::event( e );
    }
}

