/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor.cpp#31 $
**
** Implementation of QCursor class
**
** Created : 940220
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

#include "qcursor.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qdatastream.h"


/*!
  \class QCursor qcursor.h

  \brief The QCursor class provides a mouse cursor with an arbitrary shape.

  \ingroup kernel
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
  <ul>
  <li> arrowCursor : standard arrow cursor
  <li> upArrowCursor : upwards arrow
  <li> crossCursor : crosshair
  <li> waitCursor : hourglass/watch
  <li> ibeamCursor : ibeam/text entry
  <li> sizeVerCursor : vertical resize
  <li> sizeHorCursor : horizontal resize
  <li> sizeBDiagCursor : diagonal resize (/)
  <li> sizeFDiagCursor : diagonal resize (\)
  <li> sizeAllCursor : all directions resize
  <li> blankCursor : blank/invisible cursor
  <li> splitVCursor :vertical splitting
  <li> splitHCursor :horziontal splitting
  <li> pointingHandCursor : a pointing hand
  </ul>

  \sa QWidget
  <a href="guibooks.html#fowler">GUI Design Handbook: Cursors.</a>
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


/*!
  Constructs a custom pixmap cursor.

  \arg \e pixmap is the image
	    (usually it should have a \link QPixmap::setMask() mask\endlink)
  \arg \e hotX and
  \arg \e hotY define the hot spot of this cursor.

  If \e hotX is negative, it is set to the pixmap().width()/2.
  If \e hotY is negative, it is set to the pixmap().height()/2.

  Allowed cursor sizes depend on the display hardware (or the underlying
  window system). We recommend using 32x32 cursors, because this size
  is supported on all platforms. Some platforms also support 16x16, 48x48
  and 64x64 cursors.

  Currently, only black-and-white pixmaps can be used.
*/

QCursor::QCursor( const QPixmap &pixmap, int hotX, int hotY )
{
    QImage img = pixmap.convertToImage().
		    convertDepth(8,Qt::ThresholdDither|Qt::AvoidDither);
    QBitmap bm;
    bm.convertFromImage(img);
    QBitmap bmm;
    if ( pixmap.mask() ) {
	QImage mimg = pixmap.mask()->convertToImage();
	bmm.convertFromImage(mimg);
    }
    setBitmap(bm,bmm,hotX,hotY);
}



/*!
  Constructs a custom bitmap cursor.

  \arg \e bitmap and
  \arg \e mask make up the bitmap.
  \arg \e hotX and
  \arg \e hotY define the hot spot of this cursor.

  If \e hotX is negative, it is set to the bitmap().width()/2.
  If \e hotY is negative, it is set to the bitmap().height()/2.

  The cursor \e bitmap (B) and \e mask (M) bits are combined this way:
  <ol>
  <li> B=1 and M=1 gives black.
  <li> B=0 and M=1 gives white.
  <li> B=0 and M=0 gives transparency.
  <li> B=1 and M=0 gives an undefined result.
  </ol>

  Use the global color \c color0 to draw 0-pixels and \c color1 to draw
  1-pixels in the bitmaps.

  Allowed cursor sizes depend on the display hardware (or the underlying
  window system). We recommend using 32x32 cursors, because this size
  is supported on all platforms. Some platforms also support 16x16, 48x48
  and 64x64 cursors.
*/

QCursor::QCursor( const QBitmap &bitmap, const QBitmap &mask,
		  int hotX, int hotY )
{
    setBitmap(bitmap,mask,hotX,hotY);
}
