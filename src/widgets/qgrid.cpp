/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qgrid.cpp#13 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qgrid.h"
#include "qlayout.h"

/*!
  \class QGrid qgrid.h
  \brief The QGrid widget performs geometry management on its children

  \ingroup geomanagement

  The number of rows or columns is defined in the constructor. All its
  children will be placed and sized according to their sizeHint()s.

  Use setMargin() to add space around the edge, and use addSpacing to
  add space between the widgets.
  
  \sa QVBox and QHBox
 */



/*!
  Constructs a grid widget with parent \a parent and name \a name. If \a d is
  \c Horizontal, \a n specifies the number of columns. If \a d is \c Vertical,
  \a n specifies the number of rows.
 */
QGrid::QGrid( int n, Direction d, QWidget *parent, const char *name, WFlags f,
	      bool allowLines )
    :QFrame( parent, name, f, allowLines )
{
    int nCols, nRows;
    if ( d == Horizontal ) {
	nCols = n;
	nRows = -1;
    } else {
	nCols = -1;
	nRows = n;
    }
    lay = new QGridLayout( this, nRows, nCols, 0, 0, name );
    lay->setAutoAdd( TRUE );
}



/*!
  Constructs a grid widget with parent \a parent and name \a name.
  \a n specifies the number of columns.
 */
QGrid::QGrid( int n, QWidget *parent, const char *name, WFlags f, 
	      bool allowLines )
    :QFrame( parent, name, f, allowLines )
{
    lay = new QGridLayout( this, -1, n, 0, 0, name );
    lay->setAutoAdd( TRUE );
}


/*!
  Sets the spacing between children to \a space.
*/

void QGrid::setSpacing( int space )
{
    if ( layout() )
	layout()->setSpacing( space );
}


/*!
  Reimplemented for internal purposes
 */
void QGrid::frameChanged()
{
    if ( !layout() )
	return;
    layout()->setMargin( frameWidth() );
}
