/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor.h#10 $
**
** Definition of QCursor class
**
** Author  : Haavard Nord
** Created : 940219
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QCURSOR_H
#define QCURSOR_H

#include "qpoint.h"
#include "qshared.h"


class QCursorData;				// internal cursor data


class QCursor					// cursor class
{
public:
    QCursor();					// create default arrow cursor
    QCursor( int shape );			// create cursor with shape
    QCursor( const QBitmap &bitmap, const QBitmap &mask,
	     int hotX=-1, int hotY=-1 );
    QCursor( const QCursor & );
   ~QCursor();
    QCursor &operator=( const QCursor & );

    QCursor	  copy() const;

    int		  shape() const;		// get cursor shape
    bool	  setShape( int );		// set cursor shape

    static QPoint pos();			// get cursor position
    static void	  setPos( int x, int y );	// set cursor position
    static void	  setPos( const QPoint & );	// set cursor position

    static void	  initialize();			// initialize global cursors
    static void	  cleanup();			// cleanup global cursors

#if defined(_WS_WIN_)
    HANDLE	  handle() const;
#elif defined(_WS_PM_)
    HANDLE	  handle() const;
#elif defined(_WS_X11_)
    Cursor	  handle() const;
#endif

    friend QDataStream &operator<<( QDataStream &, const QCursor & );
    friend QDataStream &operator>>( QDataStream &, QCursor & );

private:
    void	  update() const;
    static QCursor *locate( int );
    QCursorData  *data;
};


inline void QCursor::setPos( const QPoint &p )
{
    setPos( p.x(), p.y() );
}


// --------------------------------------------------------------------------
// Cursor shape identifiers (correspond to global cursors)
//

enum CursorShape {
    ArrowCursor, UpArrowCursor, CrossCursor, HourGlassCursor, IbeamCursor,
    SizeVerCursor, SizeHorCursor, SizeBDiagCursor, SizeFDiagCursor,
    SizeAllCursor, BitmapCursor=24 };


// --------------------------------------------------------------------------
// Global cursors
//

extern const QCursor arrowCursor;		// standard arrow cursor
extern const QCursor upArrowCursor;		// upwards arrow
extern const QCursor crossCursor;		// crosshair
extern const QCursor hourGlassCursor;		// hourglass/watch
extern const QCursor ibeamCursor;		// ibeam/text entry
extern const QCursor sizeVerCursor;		// vertical resize
extern const QCursor sizeHorCursor;		// horizontal resize
extern const QCursor sizeBDiagCursor;		// diagonal resize (/)
extern const QCursor sizeFDiagCursor;		// diagonal resize (\)
extern const QCursor sizeAllCursor;		// all directions resize


// --------------------------------------------------------------------------
// QCursor stream functions
//

QDataStream &operator<<( QDataStream &, const QCursor & );
QDataStream &operator>>( QDataStream &, QCursor & );


#endif // QCURSOR_H
