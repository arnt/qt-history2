/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qgrid.cpp#4 $
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

  The number of rows or columns is defined in the constructor. All its
  children will be placed and sized according to their sizeHint()s.
  
  \sa QVBox and QHBox
 */



/*!
  Constructs a grid widget with parent \a parent and name \a name. If \a d is
  \c Horizontal, \a n specifies the number of columns. If \a d is \c Vertical,
  \a n specifies the number of rows.
 */
QGrid::QGrid( int n, Direction d, QWidget *parent, const char *name )
    :QWidget( parent, name )
{
    if ( d == Horizontal ) {
	nCols = n;
	nRows = 1;
    } else {
	nCols = 1;
	nRows = n;
    }
    dir = d;
    lay = new QGridLayout( this, nRows, nCols, parent?0:5, 5, name ); //### border
    row = col = 0;
}



/*!
  Constructs a grid widget with parent \a parent and name \a name.
  \a n specifies the number of columns. 
 */
QGrid::QGrid( int n, QWidget *parent, const char *name )
    :QWidget( parent, name )
{
    nCols = n;
    nRows = 1;
    dir = Horizontal;
    lay = new QGridLayout( this, nRows, nCols, parent?0:5, 5, name ); //### border
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

    if ( row >= nRows || col >= nCols )
	lay->expand( row+1, col+1 );
    lay->addWidget( w, row, col );
    //    debug( "adding %d,%d", row, col );
    if ( dir == Horizontal ) {
	if ( col+1 < nCols ) {
	    col++;
	} else {
	    col = 0;
	    row++;
	}
    } else { //Vertical
	if ( row+1 < nRows ) {
	    row++;
	} else {
	    row = 0;
	    col++;
	}
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

