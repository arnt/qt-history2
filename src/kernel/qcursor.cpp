/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor.cpp#19 $
**
** Implementation of QCursor class
**
** Created : 940220
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcursor.h"
#include "qbitmap.h"
#include "qdstream.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qcursor.cpp#19 $");


/*!
  \class QCursor qcursor.h

  \brief The QCursor class provides a mouse cursor with an arbitrary shape.

  \ingroup guitool
  \ingroup shared

  This class is used for mainly two things; to create mouse cursors to be
  associated with widgets and to get and set the position of the mouse
  cursor.

  Qt has a number of standard cursor shapes, but you can also make
  custom cursor shapes based on a \link QBitmap bitmap \endlink, a
  mask and a hotspot.

  To associate a cursor with a widget, use QWidget::setCursor().
  To associate a cursor with all widgets (maybe for a short period of time),
  use QApplication::setOverrideCursor().

  The <a name=cursors>predefined cursor</a> objects are:
  <dl compact>
  <dt> arrowCursor <dd> standard arrow cursor
  <dt> upArrowCursor <dd> upwards arrow
  <dt> crossCursor <dd> crosshair
  <dt> waitCursor <dd> hourglass/watch
  <dt> ibeamCursor <dd> ibeam/text entry
  <dt> sizeVerCursor <dd> vertical resize
  <dt> sizeHorCursor <dd> horizontal resize
  <dt> sizeBDiagCursor <dd> diagonal resize (/)
  <dt> sizeFDiagCursor <dd> diagonal resize (\)
  <dt> sizeAllCursor <dd> all directions resize
  <dt> blankCursor <dd> blank/invisible cursor
  </dl>
*/


/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/

/*!
  \relates QCursor
  Writes a cursor to the stream.
*/

QDataStream &operator<<( QDataStream &s, const QCursor &c )
{
    s << (INT16)c.shape();			// write shape id to stream
    if ( c.shape() == BitmapCursor ) {		// bitmap cursor
	s << *c.bitmap() << *c.mask();
	s << c.hotSpot();
    }
    return s;
}

/*!
  \relates QCursor
  Reads a cursor from the stream.
*/

QDataStream &operator>>( QDataStream &s, QCursor &c )
{
    INT16 shape;
    s >> shape;					// read shape id from stream
    if ( shape == BitmapCursor ) {		// read bitmap cursor
	QBitmap bm, bmm;
	QPoint	hot;
	s >> bm >> bmm >> hot;
	c = QCursor( bm, bmm, hot.x(), hot.y() );
    } else {
	c.setShape( (int)shape );		// create cursor with shape
    }
    return s;
}
