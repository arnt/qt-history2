/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsemimodal.cpp#9 $
**
** Implementation of QSemiModal class
**
** Created : 970627
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qsemimodal.h"
#include "qapplication.h"

/*!
  \class QSemiModal qsemimodal.h
  \brief The QSemiModal class is the base class of semi-modal dialog windows.

  \ingroup dialogs

  The semi-modal dialog window can disable events to other windows while
  it is open.  To enable this, the QSemiModal must be constructed with
  TRUE for the \e modal argument, which is FALSE by default, for consistency
  with QDialog.
  Such a QSemiModal is modal like QDialog, but it does not have
  its own event loop and has no concept of a default button.

  Note that the parent widget has a different meaning for semi-modal
  dialogs than for other types of widgets. A semi-modal dialog is
  placed on top of the parent widget. The dialog is centered on the
  screen if the parent widget is zero.  
*/ 


/*!
  Constructs a semi-modal dialog named \a name, which has a parent
  widget \a parent.  If \a modal is FALSE (the default), the only
  behavior different to a QWidget is automatic sizing and positioning.
*/

QSemiModal::QSemiModal( QWidget *parent, const char *name, bool modal, WFlags f )
    : QWidget( parent, name, modal ? f | WType_Modal : f )
{
    did_move = did_resize = FALSE;
}

/*!
  Destroys the QSemiModal and all its children.
*/

QSemiModal::~QSemiModal()
{
}


/*!
  Shows the widget.
  This implementation also does automatic resizing and automatic
  positioning. If you have not already resized or moved the dialog, it
  will find a size that fits the contents and a position near the middle
  of the screen (or centered relative to the parent widget if any).
*/

void QSemiModal::show()
{
    if ( !did_resize )
	adjustSize();
    if ( !did_move ) {
	QWidget *w = parentWidget();
	QPoint p( 0, 0 );
	if ( w )
	    p = w->mapToGlobal( p );
	else
	    w = QApplication::desktop();
	move( p.x() + w->width()/2  - width()/2,
	      p.y() + w->height()/2 - height()/2 );
    }
    QWidget::show();
}

/*****************************************************************************
  Geometry management.
 *****************************************************************************/



/*****************************************************************************
  Detects any widget geometry changes done by the user.
 *****************************************************************************/

/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QSemiModal::move( int x, int y )
{
    did_move = TRUE;
    QWidget::move( x, y );
}

/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QSemiModal::move( const QPoint &p )
{
    did_move = TRUE;
    QWidget::move( p );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QSemiModal::resize( int w, int h )
{
    did_resize = TRUE;
    QWidget::resize( w, h );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QSemiModal::resize( const QSize &s )
{
    did_resize = TRUE;
    QWidget::resize( s );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QSemiModal::setGeometry( int x, int y, int w, int h )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( x, y, w, h );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QSemiModal::setGeometry( const QRect &r )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( r );
}
