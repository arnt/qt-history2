/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor_mac.cpp
**
** Implementation of QCursor class for mac
**
** Created : 001019
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcursor.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qapplication.h"
#include "qdatastream.h"
#include "qnamespace.h"

// NOT REVISED


/*****************************************************************************
  Global cursors
 *****************************************************************************/

static const int cursors = 15;
static QCursor cursorTable[cursors];

static const int arrowCursorIdx = 0;

QT_STATIC_CONST_IMPL QCursor & Qt::arrowCursor = cursorTable[0];
QT_STATIC_CONST_IMPL QCursor & Qt::upArrowCursor = cursorTable[1];
QT_STATIC_CONST_IMPL QCursor & Qt::crossCursor = cursorTable[2];
QT_STATIC_CONST_IMPL QCursor & Qt::waitCursor = cursorTable[3];
QT_STATIC_CONST_IMPL QCursor & Qt::ibeamCursor = cursorTable[4];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeVerCursor = cursorTable[5];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeHorCursor = cursorTable[6];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeBDiagCursor = cursorTable[7];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeFDiagCursor = cursorTable[8];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeAllCursor = cursorTable[9];
QT_STATIC_CONST_IMPL QCursor & Qt::blankCursor = cursorTable[10];
QT_STATIC_CONST_IMPL QCursor & Qt::splitHCursor = cursorTable[11];
QT_STATIC_CONST_IMPL QCursor & Qt::splitVCursor = cursorTable[12];
QT_STATIC_CONST_IMPL QCursor & Qt::pointingHandCursor = cursorTable[13];
QT_STATIC_CONST_IMPL QCursor & Qt::forbiddenCursor = cursorTable[14];

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

struct QCursorData : public QShared {
    QCursorData( int s = 0 );
   ~QCursorData();
    int	      cshape;
    QBitmap  *bm, *bmm;
};

QCursorData::QCursorData( int )
{
    qDebug( "QCursorData::QCursorData" );
}

QCursorData::~QCursorData()
{
    qDebug( "QCursorData::~QCursorData" );
}

QCursor *QCursor::find_cur( int shape )		// find predefined cursor
{
    qDebug( "QCursor::find_cur" );
    return (uint)shape <= LastCursor ? &cursorTable[shape] : 0;
}


/*!
  Internal function that deinitializes the predefined cursors.
  This function is called from the QApplication destructor.
  \sa initialize()
*/
void QCursor::cleanup()
{
    qDebug( "QCursor::cleanup" );
}


/*!
  Internal function that initializes the predefined cursors.
  This function is called from the QApplication constructor.
  \sa cleanup()
*/

static bool initialized = FALSE;

void QCursor::initialize()
{
    qDebug( "QCursor::initialize" );
    int shape;
    for( shape = 0; shape < cursors; shape++ )
	cursorTable[shape].data = new QCursorData( shape );
    initialized = TRUE;
    qAddPostRoutine( cleanup );
}


/*!
  Constructs a cursor with the default arrow shape.
*/
QCursor::QCursor()
{
    qDebug( "QCursor::QCursor" );
}



/*!
  Constructs a cursor with the specified \a shape.

  \a shape can be one of
  <ul>
  <li> \c ArrowCursor - standard arrow cursor
  <li> \c UpArrowCursor - upwards arrow
  <li> \c CrossCursor - crosshair
  <li> \c WaitCursor - hourglass/watch
  <li> \c IbeamCursor - ibeam/text entry
  <li> \c SizeVerCursor - vertical resize
  <li> \c SizeHorCursor - horizontal resize
  <li> \c SizeBDiagCursor - diagonal resize (/)
  <li> \c SizeFDiagCursor - diagonal resize (\)
  <li> \c SizeAllCursor - all directions resize
  <li> \c BlankCursor - blank/invisible cursor
  <li> \c SplitVCursor - vertical splitting
  <li> \c SplitHCursor - horziontal splitting
  <li> \c PointingHandCursor - a pointing hand
  <li> \c BitmapCursor - userdefined bitmap cursor
  </ul>

  These correspond to the <a href="#cursors">predefined</a>
  global QCursor objects.

  \sa setShape()
*/

QCursor::QCursor( int )
{
    qDebug( "QCursor::QCursor" );
}


void QCursor::setBitmap( const QBitmap &, const QBitmap &,
			 int, int )
{
    qDebug( "QCursor::setBitmap" );
}


/*!
  Constructs a copy of the cursor \a c.
*/

QCursor::QCursor( const QCursor & )
{
    qDebug( "QCursor::QCursor" );
}

/*!
  Destructs the cursor.
*/

QCursor::~QCursor()
{
    qDebug( "QCursor::~QCursor" );
}


/*!
  Assigns \a c to this cursor and returns a reference to this cursor.
*/

QCursor &QCursor::operator=( const QCursor & )
{
    qDebug( "QCursor::operator" );
    return *this;
}


/*!
  Returns the cursor shape identifer. The return value is one of
  following values (cast to an int)

  <ul>
  <li> \c ArrowCursor - standard arrow cursor
  <li> \c UpArrowCursor - upwards arrow
  <li> \c CrossCursor - crosshair
  <li> \c WaitCursor - hourglass/watch
  <li> \c IbeamCursor - ibeam/text entry
  <li> \c SizeVerCursor - vertical resize
  <li> \c SizeHorCursor - horizontal resize
  <li> \c SizeBDiagCursor - diagonal resize (/)
  <li> \c SizeFDiagCursor - diagonal resize (\)
  <li> \c SizeAllCursor - all directions resize
  <li> \c BlankCursor - blank/invisible cursor
  <li> \c SplitVCursor - vertical splitting
  <li> \c SplitHCursor - horziontal splitting
  <li> \c PointingHandCursor - a pointing hand
  <li> \c ForbiddenCursor - a slashed circle
  <li> \c BitmapCursor - userdefined bitmap cursor
  </ul>

  These correspond to the <a href="#cursors">predefined</a>
  global QCursor objects.

  \sa setShape()
*/

int QCursor::shape() const
{
    qDebug( "QCursor::shape" );
    return 1;
}

/*!
  Sets the cursor to the shape identified by \a shape.

  <ul>
  <li> \c ArrowCursor - standard arrow cursor
  <li> \c UpArrowCursor - upwards arrow
  <li> CrossCursor - crosshair
  <li> \c WaitCursor - hourglass/watch
  <li> \c IbeamCursor - ibeam/text entry
  <li> \c SizeVerCursor - vertical resize
  <li> \c SizeHorCursor - horizontal resize
  <li> \c SizeBDiagCursor - diagonal resize (/)
  <li> \c SizeFDiagCursor - diagonal resize (\)
  <li> \c SizeAllCursor - all directions resize
  <li> \c BlankCursor - blank/invisible cursor
  <li> \c SplitVCursor - vertical splitting
  <li> \c SplitHCursor - horziontal splitting
  <li> \c PointingHandCursor - a pointing hand
  <li> \c ForbiddenCursor - a slashed circle
  <li> \c BitmapCursor - userdefined bitmap cursor
  </ul>

  These correspond to the <a href="#cursors">predefined</a>
  global QCursor objects.

  \sa shape()
*/

void QCursor::setShape( int )
{
    qDebug( "QCursor::setShape" );
}


/*!
  Returns the cursor bitmap, or 0 if it is one of the standard cursors.
*/
const QBitmap *QCursor::bitmap() const
{
    qDebug( "QCursor::bitmap" );
    return 0;
}

/*!
  Returns the cursor bitmap mask, or 0 if it is one of the standard cursors.
*/

const QBitmap *QCursor::mask() const
{
    qDebug( "QCursor::mask" );
    return 0;
}

/*!
  Returns the cursor hot spot, or (0,0) if it is one of the standard cursors.
*/

QPoint QCursor::hotSpot() const
{
    qDebug( "QCursor::hotSpot" );
    return QPoint( 0, 0 );
}


/*!
  Returns the window system cursor handle.

  \warning
  Portable in principle, but if you use it you are probably about to do
  something non-portable. Be careful.
*/

Qt::HANDLE QCursor::handle() const
{
    qDebug( "QCursor::handle" );
    return 0;
}


/*!
  Returns the position of the cursor (hot spot) in global screen
  coordinates.

  You can call QWidget::mapFromGlobal() to translate it to widget
  coordinates.

  \sa setPos(), QWidget::mapFromGlobal(), QWidget::mapToGlobal()
*/

QPoint QCursor::pos()
{
    qDebug( "QWidget::mapFromGlobal" );
    return QPoint( 0, 0 );
}

/*!
  Moves the cursor (hot spot) to the global screen position \a x and \a y.

  You can call QWidget::mapToGlobal() to translate widget coordinates
  to global screen coordinates.

  \sa pos(), QWidget::mapFromGlobal(), QWidget::mapToGlobal()
*/

void QCursor::setPos( int, int )
{
    qDebug( "QWidget::mapToGlobal" );
}

/*!
  \overload void QCursor::setPos ( const QPoint & )
*/


/*!
  \internal Creates the cursor.
*/

void QCursor::update() const
{
    qDebug( "QCursor::setPos" );
}
