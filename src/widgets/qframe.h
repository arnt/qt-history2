/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.h#4 $
**
** Definition of QFrame widget class
**
** Author  : Haavard Nord
** Created : 950201
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFRAME_H
#define QFRAME_H

#include "qwidget.h"


class QFrame : public QWidget			// frame class
{
    Q_OBJECT
public:
    QFrame( QWidget *parent=0, const char *name=0 );

    const   NoFrame = 0;			// no frame
    const   Box     = 0x0001;			// rectangular box
    const   Panel   = 0x0002;			// rectangular panel
    const   HLine   = 0x0003;			// horizontal line
    const   VLine   = 0x0004;			// vertical line
    const   MType   = 0x000f;
    const   Plain   = 0x0010;			// plain line
    const   Raised  = 0x0020;			// raised shadow effect
    const   Sunken  = 0x0030;			// sunken shadow effect
    const   MStyle  = 0x00f0;

    QRect   contentsRect()  const;		// get rect inside frame

    int	    frame()	    const { return fstyle; }
    void    setFrame( int );

    int	    frameWidth()    const { return fwidth; }
    void    setFrameWidth( int );

    int	    midLineWidth()  const { return mwidth; }
    void    setMidLineWidth( int );

    QRect   frameRect()	    const;

protected:
    void    setFrameRect( const QRect & );
    void    paintEvent( QPaintEvent * );
    virtual void drawFrame( QPainter * );
    virtual void drawContents( QPainter * );    

private:
    QRect   frect;				// frame rectangle
    int     fstyle;				// frame style
    short   fwidth;				// frame width
    short   mwidth;				// mid line width
};


inline QRect QFrame::frameRect() const
{
    return frect.isNull() ? rect() : frect;
}


#endif // QFRAME_H
