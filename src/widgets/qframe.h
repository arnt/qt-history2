/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.h#1 $
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

    const NoFrame = 0;				// no frame
    const Box     = 0x0001;			// rectangular box
    const Panel   = 0x0002;			// rectangular panel
    const HLine   = 0x0003;			// horizontal line
    const VLine   = 0x0004;			// vertical line
    const MType	  = 0x000f;
    const Plain	  = 0x0010;			// plain line
    const Raised  = 0x0020;			// raised shadow effect
    const Sunken  = 0x0030;			// sunken shadow effect
    const MStyle  = 0x00f0;

    int	    frameStyle()    const { return fstyle; }
    void    setFrameStyle( int );

    int	    lineWidth()	    const { return lwidth; }
    void    setLineWidth( int );

    int	    midLineWidth()  const { return mwidth; }
    void    setMidLineWidth( int );

protected:
    void    paintEvent( QPaintEvent * );
    virtual void drawFrame( QPainter * );
    virtual void drawContents( QPainter * );    

private:
    int     fstyle;				// frame style
    short   lwidth;				// line width
    short   mwidth;				// mid line width
};


#endif // QFRAME_H
