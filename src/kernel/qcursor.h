/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor.h#24 $
**
** Definition of QCursor class
**
** Created : 940219
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QCURSOR_H
#define QCURSOR_H

#include "qpoint.h"
#include "qshared.h"


struct QCursorData;				// internal cursor data


class QCursor					// cursor class
{
public:
    QCursor();					// create default arrow cursor
    QCursor( int shape );
    QCursor( const QBitmap &bitmap, const QBitmap &mask,
	     int hotX=-1, int hotY=-1 );
    QCursor( const QCursor & );
   ~QCursor();
    QCursor &operator=( const QCursor & );

    int		  shape()   const;
    void	  setShape( int );

    const QBitmap *bitmap() const;
    const QBitmap *mask()   const;
    QPoint	  hotSpot() const;

    HANDLE	  handle()  const;

    static QPoint pos();
    static void	  setPos( int x, int y );
    static void	  setPos( const QPoint & );

    static void	  initialize();
    static void	  cleanup();

private:
    void	  update() const;
    QCursorData	 *data;
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
    SizeAllCursor, BlankCursor, LastCursor=BlankCursor, BitmapCursor=24 };


/*****************************************************************************
  Global cursors
 *****************************************************************************/

extern const QCursor arrowCursor;		// standard arrow cursor
extern const QCursor upArrowCursor;		// upwards arrow
extern const QCursor crossCursor;		// crosshair
extern const QCursor waitCursor;		// hourglass/watch
extern const QCursor ibeamCursor;		// ibeam/text entry
extern const QCursor sizeVerCursor;		// vertical resize
extern const QCursor sizeHorCursor;		// horizontal resize
extern const QCursor sizeBDiagCursor;		// diagonal resize (/)
extern const QCursor sizeFDiagCursor;		// diagonal resize (\)
extern const QCursor sizeAllCursor;		// all directions resize
extern const QCursor blankCursor;		// blank/invisible cursor


/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/

QDataStream &operator<<( QDataStream &, const QCursor & );
QDataStream &operator>>( QDataStream &, QCursor & );


#endif // QCURSOR_H
