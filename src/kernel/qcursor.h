/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor.h#2 $
**
** Definition of QCursor class
**
** Author  : Haavard Nord
** Created : 940219
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QCURSOR_H
#define QCURSOR_H

#include "qpoint.h"


class QCursor
{
friend class QWidget;
public:
    QCursor();					// create default arrow cursor
    QCursor( int shape );			// create cursor with shape
   ~QCursor();

    int	   shape() const { return cshape; }	// get cursor shape
    bool   setShape( int );			// set cursor shape

    static QPoint pos();			// get cursor position
    static void	  setPos( int x, int y );	// set cursor position
    static void	  setPos( const QPoint & );	// set cursor position

    static void	  initialize();			// initialize global cursors
    static void	  cleanup();			// cleanup global cursors

private:
    void   update();
    int	   cshape;
    static QCursor *locate( int );
#if defined(_WS_WIN_) || defined(_WS_WIN32_)
    HANDLE hcurs;
public:
    HANDLE handle() const { return hcurs; }
#elif defined(_WS_PM_)
    HANDLE hcurs;
public:
    HANDLE handle() const { return hcurs; }
#elif defined(_WS_X11_)
    Cursor cursor;
    Pixmap pm, mpm;
#endif
};


inline void QCursor::setPos( const QPoint &p )
{
    setPos( p.getX(), p.getY() );
}


// --------------------------------------------------------------------------
// Cursor shape identifiers
//

enum CursorShape {
    ArrowCursor, UpArrowCursor, CrossCursor, HourGlassCursor, IbeamCursor,
    SizeVerCursor, SizeHorCursor, SizeBDiagCursor, SizeFDiagCursor,
    SizeAllCursor };


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
