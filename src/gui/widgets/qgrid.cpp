/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "qgrid.h"
#ifndef QT_NO_GRID
#include "qlayout.h"
#include "qapplication.h"
#include "qevent.h"
#include "qmenubar.h"
/*!
    \class QGrid qgrid.h
    \brief The QGrid widget provides simple geometry management of its children.

    \ingroup geomanagement
    \ingroup appearance

    The grid places its widgets either in columns or in rows depending
    on its orientation.

    The number of rows \e or columns is defined in the constructor.
    All the grid's children will be placed and sized in accordance
    with their sizeHint() and sizePolicy().

    Use setMargin() to add space around the grid itself, and
    setSpacing() to add space between the widgets.

    \img qgrid-m.png QGrid

    \sa QVBox QHBox QGridLayout
*/

/*!
    Constructs a grid widget with parent \a parent, called \a name.
    If \a orient is \c Horizontal, \a n specifies the number of
    columns. If \a orient is \c Vertical, \a n specifies the number of
    rows. The widget flags \a f are passed to the QFrame constructor.
*/
QGrid::QGrid( int n, Orientation orient, QWidget *parent, const char *name,
	      WFlags f )
    : QFrame( parent, name, f )
{
    int nCols, nRows;
    if ( orient == Horizontal ) {
	nCols = n;
	nRows = -1;
    } else {
	nCols = -1;
	nRows = n;
    }
    lay = new QGridLayout( this, nRows, nCols, 0, 0, name );
}



/*!
    Constructs a grid widget with parent \a parent, called \a name.
    \a n specifies the number of columns. The widget flags \a f are
    passed to the QFrame constructor.
 */
QGrid::QGrid( int n, QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f )
{
    lay = new QGridLayout( this, -1, n, 0, 0, name );
}

/*!
    \internal
*/
void QGrid::childEvent(QChildEvent *e)
{
    QWidget *child = qt_cast<QWidget*>(e->child());
    if (!child || child->isTopLevel())
	return;
    if (e->added()) {
	lay->addWidget(child);
    } else if (e->polished()) {
	QMenuBar *mb;
	if ((mb=qt_cast<QMenuBar*>(child))) {
	    lay->removeWidget(mb);
	    lay->setMenuBar(mb);
	}
    }
}

/*!
    Sets the spacing between the child widgets to \a space.
*/

void QGrid::setSpacing( int space )
{
    if ( layout() )
	layout()->setSpacing( space );
}

#endif
