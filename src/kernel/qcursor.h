/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor.h#35 $
**
** Definition of QCursor class
**
** Created : 940219
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

#ifndef QCURSOR_H
#define QCURSOR_H

#ifndef QT_H
#include "qpoint.h"
#include "qshared.h"
#include "qnamespace.h"
#endif // QT_H


struct QCursorData;				// internal cursor data


class Q_EXPORT QCursor				// cursor class
{
public:
    QCursor(): data( 0 ) {}			// create default arrow cursor
    QCursor( int shape );
    QCursor( const QBitmap &bitmap, const QBitmap &mask,
	     int hotX=-1, int hotY=-1 );
    QCursor( const QPixmap &pixmap,
	     int hotX=-1, int hotY=-1 );
    QCursor( const QCursor & );
   ~QCursor();
    QCursor &operator=( const QCursor & );

    int		  shape()   const;
    void	  setShape( int );

    const QBitmap *bitmap() const;
    const QBitmap *mask()   const;
    QPoint	  hotSpot() const;

#if defined(_WS_WIN_)
    HCURSOR	  handle()  const;
#elif defined(_WS_X11_)
    HANDLE	  handle()  const;
#endif

    static QPoint pos();
    static void	  setPos( int x, int y );
    static void	  setPos( const QPoint & );

    static void	  initialize();
    static void	  cleanup();

private:
    void	  setBitmap( const QBitmap &bitmap, const QBitmap &mask,
				 int hotX, int hotY );
    void	  update() const;
    QCursorData	 *data;
    QCursor	 *find_cur(int);
};


inline void QCursor::setPos( const QPoint &p )
{
    setPos( p.x(), p.y() );
}


/*****************************************************************************
  Cursor shape identifiers (correspond to global cursor objects)
 *****************************************************************************/

enum QCursorShape {
    ArrowCursor, UpArrowCursor, CrossCursor, WaitCursor, IbeamCursor,
    SizeVerCursor, SizeHorCursor, SizeBDiagCursor, SizeFDiagCursor,
    SizeAllCursor, BlankCursor, SplitVCursor, SplitHCursor,
    LastCursor=SplitHCursor, BitmapCursor=24 };


/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QCursor & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QCursor & );


#endif // QCURSOR_H
